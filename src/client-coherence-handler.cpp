#include <grpcpp/grpcpp.h>
#include <proto/hearty-store.grpc.pb.h>
#include <proto/hearty-store.pb.h>
#include "hearty-store-cache.hpp"
#include <iostream>
#include <thread>

class EvictionServiceImpl final : public ProcessingService::Service {
private:
    ClientCache* cache;
    std::unique_ptr<ProcessingService::Stub> stub;

public:
    EvictionServiceImpl(ClientCache* cache_instance, std::unique_ptr<ProcessingService::Stub>& stub_instance) 
        : cache(cache_instance), stub(std::move(stub_instance)) {}

    ::grpc::Status Evict(::grpc::ServerContext* context,
                        const ::evictRequest* request,
                        ::evictResponse* response) override {
        std::string file_id = request->file_id();
        std::cout << "Received eviction request for file: " << file_id << std::endl;

        if (cache->isInCache(file_id)) {
            // Write back dirty data if needed
            if (cache->cache_map[file_id].is_dirty) {
                std::string file_content = cache->getContentFromCache(file_id);
                putRequest put_request;
                putResponse put_response;
                grpc::ClientContext put_context;
                
                // Send the file to the main server
                put_request.set_store_name(cache->cache_map[file_id].store_id);
                put_request.set_file_path(cache->cache_map[file_id].file_path);
                put_request.set_file_content(file_content);
                
                grpc::Status status = stub->Put(&put_context, put_request, &put_response);
                if (!status.ok()) {
                    std::cerr << "Failed to write back dirty data: " << status.error_message() << std::endl;
                    response->set_success(false);
                    response->set_message("Failed to write back dirty data");
                    return grpc::Status::OK;
                }
            }

            // Remove from cache
            cache->removeFileIdFromCache(file_id);
            response->set_success(true);
            response->set_message("Successfully evicted file: " + file_id);
        } else {
            response->set_success(true);
            response->set_message("File not in cache: " + file_id);
        }

        return grpc::Status::OK;
    }
};

int main(int argc, char** argv) {
    // Add argument check
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    std::string server_address = "0.0.0.0:" + port;
    std::string main_server_address = "localhost:2546";  // Address of the main hearty store server

    // Create channel to main server
    auto channel = grpc::CreateChannel(main_server_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<ProcessingService::Stub> stub = ProcessingService::NewStub(channel);
    
    // Initialize cache
    ClientCache cache;
    cache.loadAllCachesFromFile();

    // Start the eviction service
    EvictionServiceImpl service(&cache, stub);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Eviction service listening on " << server_address << std::endl;
    server->Wait();

    return 0;
} 