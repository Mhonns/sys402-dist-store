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
    // Varaible stores files id assiociated with client ip
    // file_id -> client_ip
    std::unordered_map<std::string, std::string> file_id_to_client_ip;

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
        std::cout << "Put called with content: " << request->file_content() << std::endl;

        // Busy due to the mutex lock
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_file_id("");
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }

        try {
            int store_id = std::stoi(request->store_name());
            std::string object_id = put(store_id, request->file_path(), request->file_content());
            
            if (object_id.empty()) {
                response->set_success(false);
                response->set_file_id("");
                response->set_message("Failed to store file in store " + request->store_name());
            } else {
                response->set_success(true);
                response->set_file_id(object_id);
                response->set_message("Success file stored in store " + request->store_name());
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

    ::grpc::Status Cache(::grpc::ServerContext* context, 
                          const ::cacheRequest* request, 
                          ::cacheResponse* response) override {
        std::cout << "Cache called for file_id: " << request->file_id() << std::endl;
        std::string client_ip = context->peer();
        std::string file_id = request->file_id();
        std::cout << "Client ip: " << client_ip << std::endl;
        
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }

        // If file_id is empty, assign it to the client
        if (file_id.empty()) {
            file_id = client_ip;
            file_id_to_client_ip[file_id] = client_ip;
            response->set_success(true);
            response->set_message("File ID assigned: " + file_id);
        } else {
            // Check if file is owned by a different client
            auto it = file_id_to_client_ip.find(file_id);
            if (it != file_id_to_client_ip.end() && it->second != client_ip) {
                // Send eviction request to the current owner
                evictRequest evict_req;
                evictResponse evict_resp;
                grpc::ClientContext evict_context;
                
                std::shared_ptr<grpc::Channel> channel = 
                    grpc::CreateChannel(it->second, grpc::InsecureChannelCredentials());
                std::unique_ptr<ProcessingService::Stub> stub = 
                    ProcessingService::NewStub(channel);
                
                evict_req.set_file_id(file_id);
                grpc::Status status = stub->Evict(&evict_context, evict_req, &evict_resp);
                
                if (status.ok() && evict_resp.success()) {
                    // Update ownership
                    file_id_to_client_ip[file_id] = client_ip;
                    response->set_success(true);
                    response->set_message("Cache ownership transferred");
                } else {
                    response->set_success(false);
                    response->set_message("Failed to evict from current owner");
                }
            } else {
                // File is either unowned or owned by the requesting client
                file_id_to_client_ip[file_id] = client_ip;
                response->set_success(true);
                response->set_message("Cache ownership confirmed");
            }
        }

        unlock_server();
        return grpc::Status::OK;
    }

    ::grpc::Status Evict(::grpc::ServerContext* context,
                        const ::evictRequest* request,
                        ::evictResponse* response) override {
        std::string file_id = request->file_id();
        
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is handling another request.");
            return grpc::Status::OK;
        }

        // Remove the file_id from tracking
        file_id_to_client_ip.erase(file_id);
        
        response->set_success(true);
        response->set_message("File evicted successfully");

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