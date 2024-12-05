#pragma once
#include <grpcpp/grpcpp.h>
#include <proto/hearty-store.grpc.pb.h>
#include <proto/hearty-store.pb.h>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <ctime>

// Cache entry structure
struct CacheEntry {
    std::string file_id;
    std::string file_path;
    bool is_dirty;
    std::time_t timestamp;
};

class ClientCache {
private:
    static const size_t MAX_CACHE_SIZE = 8;
    std::unordered_map<std::string, CacheEntry> cache_map;
    std::queue<std::string> fifo_queue;
    std::string cache_dir;

    void evictIfNeeded(ProcessingService::Stub* stub) {
        if (fifo_queue.size() >= MAX_CACHE_SIZE) {
            std::string to_evict = fifo_queue.front();
            fifo_queue.pop();
            
            if (cache_map[to_evict].is_dirty) {
                putRequest put_request;
                putResponse put_response;
                grpc::ClientContext context;
                put_request.set_file_path(cache_map[to_evict].file_path);
                stub->Put(&context, put_request, &put_response);
            }
            
            std::filesystem::remove(cache_dir + "/" + to_evict);
            cache_map.erase(to_evict);
        }
    }

public:
    ClientCache() {
        cache_dir = "/tmp/hearty-store-cache";
        std::filesystem::create_directories(cache_dir);
    }

    void cacheFile(const std::string& file_id, const std::string& file_path, 
                   const std::string& content, ProcessingService::Stub* stub) {
        evictIfNeeded(stub);
        
        std::string cache_path = cache_dir + "/" + file_id;
        std::ofstream cache_file(cache_path);
        cache_file << content;
        cache_file.close();

        CacheEntry entry{file_id, file_path, false, std::time(nullptr)};
        cache_map[file_id] = entry;
        fifo_queue.push(file_id);
    }

    bool isInCache(const std::string& file_id) {
        return cache_map.find(file_id) != cache_map.end();
    }

    std::string getFromCache(const std::string& file_id) {
        if (!isInCache(file_id)) return "";
        
        std::string cache_path = cache_dir + "/" + file_id;
        std::ifstream cache_file(cache_path);
        std::string content((std::istreambuf_iterator<char>(cache_file)),
                            std::istreambuf_iterator<char>());
        return content;
    }

    void markDirty(const std::string& file_id) {
        if (isInCache(file_id)) {
            cache_map[file_id].is_dirty = true;
        }
    }

    std::string makeCacheableGetRequest(const std::string& file_id, 
                                        const std::unique_ptr<ProcessingService::Stub>& stub) {
        std::string accumulated_content = "";
        if (isInCache(file_id)) {
            std::string content = getFromCache(file_id);
            std::cout << "Client Get from cache: " << std::endl;
            return content;
        } else {
            getRequest get_request;
            getResponse get_response;
            grpc::ClientContext get_context;
            get_request.set_store_name("40");
            get_request.set_file_identifier(file_id);
            grpc::Status get_status;
            std::unique_ptr<grpc::ClientReader<getResponse>> reader = stub->Get(&get_context, get_request);
            std::cout << "Client Get Sent:" << std::endl;
            while (reader->Read(&get_response)) {
                accumulated_content += get_response.file_content();
            }
            get_status = reader->Finish();
            if (!get_status.ok()) {
                std::cerr << "Get failed: " << get_status.error_message() << std::endl;
            }
            cacheFile(file_id, "TestTransfer.txt", accumulated_content, stub.get());
        }
        return accumulated_content;
    }
}; 