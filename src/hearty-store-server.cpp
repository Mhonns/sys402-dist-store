#include <iostream>
#include <mutex>
#include <unistd.h> // for delay checking mutex lock
#include <filesystem>
#include <grpcpp/grpcpp.h>
#include <proto/hearty-store.grpc.pb.h>
#include <proto/hearty-store.pb.h>
#include <grpcpp/server_builder.h>
#include "../include/hearty-store-server.hpp"

namespace fs = std::filesystem;

class ProcessingImpl : public ProcessingService::Service {
private:
    std::mutex global_lock;

    bool try_lock_server() {
        return global_lock.try_lock();
    }

    void unlock_server() {
        global_lock.unlock();
    }

public:
    ::grpc::Status Init(::grpc::ServerContext* context, 
                               const ::initRequest* request, 
                               ::initResponse* response) override {
        std::cout << "InitRequest called with store_name: " << request->store_name() << std::endl;
        
        // Busy due to the mutex lock
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }
        
        // Throw to the init function
        int store_id = std::stoi(request->store_name());
        if (!initialize(store_id)) {
            unlock_server();
            response->set_success(false);
            response->set_message("Can not create a store instance.");
            return grpc::Status::OK;
        }

        // Success status
        response->set_success(true);
        response->set_message("Success the store " + request->store_name() + " was created");

        unlock_server();
        return grpc::Status::OK;
    }

    ::grpc::Status Put(::grpc::ServerContext* context, 
                       const ::putRequest* request, 
                       ::putResponse* response) override {
        std::cout << "Put Request called with store_name: " << request->store_name() << std::endl;
        std::cout << "Put called with file_path: " << request->file_path() << std::endl;

        // Busy due to the mutex lock
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }

        try {
            int store_id = std::stoi(request->store_name());
            std::string file_path = request->file_path();

            if (!std::filesystem::exists(file_path)) {
                response->set_success(false);
                response->set_message("Input file does not exist: " + file_path);
                unlock_server();
                return grpc::Status::OK;
            }

            std::string object_id = put(store_id, file_path);
            
            if (object_id.empty()) {
                response->set_success(false);
                response->set_message("Failed to store file in store " + request->store_name());
            } else {
                response->set_success(true);
                response->set_message(object_id);
            }
        }
        catch (const std::exception& e) {
            response->set_success(false);
            response->set_message(std::string("Error processing request: ") + e.what());
        }

        unlock_server();
        return grpc::Status::OK;
    }

    ::grpc::Status Get(::grpc::ServerContext* context, 
                       const ::getRequest* request, 
                       ::grpc::ServerWriter<::getResponse>* writer) override {
        std::cout << "Get called for store_name: " << request->store_name()
                  << " and file_identifier: " << request->file_identifier() << std::endl;

        
        if (!try_lock_server()) {
            ::getResponse response;
            response.set_success(false);
            response.set_message("Server is handling another request.");
            writer->Write(response);
            return grpc::Status::OK;
        }

        try {
            int store_id = std::stoi(request->store_name());
            std::string file_identifier = request->file_identifier();
            std::string content = get(store_id, file_identifier);
    
            if (content.empty()) {
                ::getResponse response;
                response.set_success(false);
                response.set_message("Failed to retrieve file with identifier " + file_identifier);
                writer->Write(response);
            } else {
                // Stream the content in chunks
                ::getResponse response;
                for (size_t i = 0; i < content.size(); i += BLOCK_SIZE) {
                    response.set_success(true);
                    response.set_file_content(content.substr(i, BLOCK_SIZE));
                    writer->Write(response);
                }
            }
        }
        catch (const std::exception& e) {
            ::getResponse response;
            response.set_success(false);
            response.set_message(std::string("Error processing request: ") + e.what());
            writer->Write(response);
        }

        unlock_server();
        return grpc::Status::OK;
    }

    ::grpc::Status List(::grpc::ServerContext* context, 
                        const ::listRequest* request, 
                        ::listResponse* response) override {
        std::cout << "List called." << std::endl;
         // Busy due to the mutex lock
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }
        
        // Throw to the list function
        std::string status = list_stores();
        response->set_success(true);
        response->set_message(status);

        unlock_server();
        return grpc::Status::OK;
    }

    ::grpc::Status Destroy(::grpc::ServerContext* context, 
                           const ::destroyRequest* request, 
                           ::destroyResponse* response) override {
        std::cout << "Destroy called for store_name: " << request->store_name() << std::endl;
        
        // Busy due to the mutex lock
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }

        // Destroy the store
        int store_id = std::stoi(request->store_name());
        if (!destroy_store(store_id)) {
            response->set_success(false);
            response->set_message("Failed to destroy store " + request->store_name());
        } else {
            response->set_success(true);
            response->set_message("Destroyed store " + request->store_name());
        }
        
        unlock_server();
        return grpc::Status::OK;
    }
};

int main() {
    std::string service_ports = "0.0.0.0:2546";
    ProcessingImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(service_ports, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Hearty store service is running on " << service_ports << std::endl;

    server->Wait();

    return 0;
}