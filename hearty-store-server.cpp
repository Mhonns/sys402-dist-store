#include <iostream>
#include <memory>
#include <string>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <grpcpp/grpcpp.h>
#include "heartystore.grpc.pb.h"

namespace fs = std::filesystem;

class HeartyStoreServiceImpl final : public heartystore::HeartyStore::Service {
private:
    std::mutex global_lock;
    std::string base_path = "/tmp/hearty-store/";
    std::unordered_map<std::string, fs::path> store_paths;

    bool try_lock_server() {
        return global_lock.try_lock();
    }

    void unlock_server() {
        global_lock.unlock();
    }

    bool store_exists(const std::string& store_name) {
        return store_paths.find(store_name) != store_paths.end();
    }

    fs::path get_store_path(const std::string& store_name) {
        return base_path / store_name;
    }

public:
    HeartyStoreServiceImpl() {
        // Create base directory if it doesn't exist
        fs::create_directories(base_path);
        
        // Load existing stores
        for (const auto& entry : fs::directory_iterator(base_path)) {
            if (entry.is_directory()) {
                store_paths[entry.path().filename().string()] = entry.path();
            }
        }
    }

    grpc::Status Initialize(grpc::ServerContext* context,
                          const heartystore::InitRequest* request,
                          heartystore::InitResponse* response) override {
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is busy");
            return grpc::Status::OK;
        }

        std::string store_name = request->store_name();
        fs::path store_path = get_store_path(store_name);

        if (store_exists(store_name)) {
            unlock_server();
            response->set_success(false);
            response->set_message("Store already exists");
            return grpc::Status::OK;
        }

        try {
            fs::create_directories(store_path);
            store_paths[store_name] = store_path;
            response->set_success(true);
            response->set_message("Store created successfully");
        } catch (const std::exception& e) {
            response->set_success(false);
            response->set_message(std::string("Failed to create store: ") + e.what());
        }

        unlock_server();
        return grpc::Status::OK;
    }

    grpc::Status Put(grpc::ServerContext* context,
                    const heartystore::PutRequest* request,
                    heartystore::PutResponse* response) override {
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is busy");
            return grpc::Status::OK;
        }

        std::string store_name = request->store_name();
        if (!store_exists(store_name)) {
            unlock_server();
            response->set_success(false);
            response->set_message("Store does not exist");
            return grpc::Status::OK;
        }

        fs::path file_path = store_paths[store_name] / request->file_identifier();
        
        try {
            std::ofstream file(file_path, std::ios::binary);
            file.write(request->file_content().c_str(), request->file_content().size());
            response->set_success(true);
            response->set_message("File stored successfully");
        } catch (const std::exception& e) {
            response->set_success(false);
            response->set_message(std::string("Failed to store file: ") + e.what());
        }

        unlock_server();
        return grpc::Status::OK;
    }

    grpc::Status Get(grpc::ServerContext* context,
                    const heartystore::GetRequest* request,
                    heartystore::GetResponse* response) override {
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is busy");
            return grpc::Status::OK;
        }

        std::string store_name = request->store_name();
        if (!store_exists(store_name)) {
            unlock_server();
            response->set_success(false);
            response->set_message("Store does not exist");
            return grpc::Status::OK;
        }

        fs::path file_path = store_paths[store_name] / request->file_identifier();

        try {
            if (!fs::exists(file_path)) {
                response->set_success(false);
                response->set_message("File not found");
            } else {
                std::ifstream file(file_path, std::ios::binary);
                std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
                response->set_success(true);
                response->set_message("File retrieved successfully");
                response->set_file_content(content);
            }
        } catch (const std::exception& e) {
            response->set_success(false);
            response->set_message(std::string("Failed to retrieve file: ") + e.what());
        }

        unlock_server();
        return grpc::Status::OK;
    }

    grpc::Status List(grpc::ServerContext* context,
                     const heartystore::ListRequest* request,
                     heartystore::ListResponse* response) override {
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is busy");
            return grpc::Status::OK;
        }

        try {
            for (const auto& pair : store_paths) {
                response->add_store_names(pair.first);
            }
            response->set_success(true);
            response->set_message("Stores listed successfully");
        } catch (const std::exception& e) {
            response->set_success(false);
            response->set_message(std::string("Failed to list stores: ") + e.what());
        }

        unlock_server();
        return grpc::Status::OK;
    }

    grpc::Status Destroy(grpc::ServerContext* context,
                        const heartystore::DestroyRequest* request,
                        heartystore::DestroyResponse* response) override {
        if (!try_lock_server()) {
            response->set_success(false);
            response->set_message("Server is busy");
            return grpc::Status::OK;
        }

        std::string store_name = request->store_name();
        if (!store_exists(store_name)) {
            unlock_server();
            response->set_success(false);
            response->set_message("Store does not exist");
            return grpc::Status::OK;
        }

        try {
            fs::remove_all(store_paths[store_name]);
            store_paths.erase(store_name);
            response->set_success(true);
            response->set_message("Store destroyed successfully");
        } catch (const std::exception& e) {
            response->set_success(false);
            response->set_message(std::string("Failed to destroy store: ") + e.what());
        }

        unlock_server();
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    HeartyStoreServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}