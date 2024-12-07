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
    std::string store_id;
    std::string file_id;
    std::string file_path;
    bool is_dirty;
    std::time_t timestamp;
};

class ClientCache {
public:
    static const size_t MAX_CACHE_SIZE = 8;
    std::unordered_map<std::string, CacheEntry> cache_map;
    std::queue<std::string> fifo_queue;
    std::string cache_dir;

    void evictIfNeeded(ProcessingService::Stub* stub) {
        if (fifo_queue.size() >= MAX_CACHE_SIZE) {
            std::string to_evict = fifo_queue.front();
            fifo_queue.pop();
            
            // Write the dirty file to the server
            if (cache_map[to_evict].is_dirty) {
                std::string file_content = getContentFromCache(to_evict);
                putRequest put_request;
                putResponse put_response;
                grpc::ClientContext context;
                put_request.set_store_name(cache_map[to_evict].file_id);
                put_request.set_file_path(cache_map[to_evict].file_path);
                put_request.set_file_content(file_content);
                stub->Put(&context, put_request, &put_response);
            }
            
            // Remove the file from the cache
            std::filesystem::remove(cache_dir + "/" + to_evict);
            cache_map.erase(to_evict);
        }
    }

    ClientCache() {
        cache_dir = "/tmp/hearty-store-cache";
        std::filesystem::create_directories(cache_dir);
    }

    void cacheFile(const std::string& store_id, const std::string& file_id, const std::string& file_path, 
                   const std::string& content, ProcessingService::Stub* stub) {
        evictIfNeeded(stub);
        
        std::string cache_path = cache_dir + "/" + file_id;
        std::ofstream cache_file(cache_path);
        cache_file << content;
        cache_file.close();

        CacheEntry entry{store_id, file_id, file_path, false, std::time(nullptr)};
        cache_map[file_id] = entry;
        fifo_queue.push(file_id);
    }
    
    bool isInCache(const std::string& file_id) {
        return cache_map.find(file_id) != cache_map.end();
    }

    void markDirty(const std::string& file_id) {
        if (isInCache(file_id)) {
            cache_map[file_id].is_dirty = true;
        }
    }

    void removeFileIdFromCache(const std::string& file_id) {
        if (isInCache(file_id)) {
            cache_map.erase(file_id);
            std::filesystem::remove(cache_dir + "/" + file_id);
        }
    }

    std::string getFileFromServer(const std::string& store_id, const std::string& file_id, 
                              ProcessingService::Stub* stub) {
        std::string accumulated_content = "";
        getRequest get_request;
        getResponse get_response;
        grpc::ClientContext get_context;
        get_request.set_store_name(store_id);
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
        cacheFile(store_id, file_id, "", accumulated_content, stub);
        saveAllCachesToFile();
        return accumulated_content;
    }

    std::string putContentToServer(const std::string& store_id, const std::string& file_path, 
                                    std::string file_id, ProcessingService::Stub* stub) {
        // Read file content
        std::ifstream file(file_path);
        std::string file_content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

        // Write to the server
        putRequest put_request;
        putResponse put_response;
        grpc::ClientContext put_context;
        put_request.set_store_name(store_id);
        put_request.set_file_path(file_path);
        put_request.set_file_content(file_content);
        std::cout << "Client Put Sent:" << std::endl;
        grpc::Status put_status = stub->Put(&put_context, put_request, &put_response);
        if (!put_status.ok()) {
            std::cerr << "Put failed: " << put_status.error_message() << std::endl;
        } else {
            file_id = put_response.file_id();
        }

        // Write to the cache
        cacheFile(store_id, file_id, file_path, file_content, stub);
        saveAllCachesToFile();
        return file_id;
    }

    std::string getContentFromCache(const std::string& file_id) {
        if (!isInCache(file_id)) return "";
        std::string cache_path = cache_dir + "/" + file_id;
        std::ifstream cache_file(cache_path);
        std::string content((std::istreambuf_iterator<char>(cache_file)),
                            std::istreambuf_iterator<char>());
        return content;
    }

    std::string getIdFromPath(const std::string& file_path) {
        for (const auto& entry : cache_map) {
            if (entry.second.file_path == file_path) {
                return entry.first;
            }
        }
        return "";
    }

    // Save all caches to file
    void saveAllCachesToFile() {
        std::ofstream file(cache_dir + "/all_caches.caches");
        // Save cache size
        file << cache_map.size() << std::endl;
        // Save fifo queue
        file << fifo_queue.size() << std::endl;
        // Save cache map
        for (const auto& entry : cache_map) {
            file << entry.first << " " << entry.second.file_path << " " << entry.second.is_dirty 
                 << " " << entry.second.timestamp << std::endl;
        }
        file.close();
    }

    // Load all caches from file
    bool loadAllCachesFromFile() {
        if (!std::filesystem::exists(cache_dir + "/all_caches.caches")) return false;
        std::ifstream file(cache_dir + "/all_caches.caches");
        // Load cache size
        size_t cache_size;
        file >> cache_size;
        // Load fifo queue
        size_t fifo_size;
        file >> fifo_size;
        // Load cache map
        for (size_t i = 0; i < cache_size; ++i) {
            std::string file_id;
            std::string file_path;
            bool is_dirty;
            std::string store_id;
            std::time_t timestamp;
            file >> store_id >> file_id >> file_path >> is_dirty >> timestamp;
            cache_map[file_id] = CacheEntry{store_id, file_id, file_path, is_dirty, timestamp};
            fifo_queue.push(file_id);
        }
        return true;
    }

    std::string cacheableGetRequest(const std::string& store_name, const std::string& file_id, 
                                        const std::unique_ptr<ProcessingService::Stub>& stub) {
        loadAllCachesFromFile();
        std::string accumulated_content = "";
        std::cout << "File id: " << file_id << std::endl;

        if (isInCache(file_id)) {
            // Tell server that client get file from cache
            cacheResponse cache_response;
            grpc::ClientContext cache_context;
            cacheRequest cache_request;
            cache_request.set_file_id(file_id);
            stub->Cache(&cache_context, cache_request, &cache_response);
            std::cout << "Cache response: " << cache_response.message() << std::endl;
            if (!cache_response.success()) {
                std::cerr << "Cache not latest fall back to send request to server: " << std::endl;
                accumulated_content = getFileFromServer(store_name, file_id, stub.get());
            } else {
                std::cout << "Client Get from cache: " << std::endl;
                std::string content = getContentFromCache(file_id);
                return content;
            }
        } else {
            // Get file from server
            accumulated_content = getFileFromServer(store_name, file_id, stub.get());
        }
        return accumulated_content;
    }

    std::string cacheablePutRequest(const std::string& store_name, const std::string& file_path, 
                                    ProcessingService::Stub* stub) {
        // Load all caches from file
        loadAllCachesFromFile();

        // Get file id from file path
        std::string file_id = getIdFromPath(file_path);

        // If file is not in cache, then write through
        if (isInCache(file_id)) {
            // Tell server that client put file to cache
            cacheResponse cache_response;
            grpc::ClientContext cache_context;
            cacheRequest cache_request;
            cache_request.set_file_id(file_id);
            stub->Cache(&cache_context, cache_request, &cache_response);
            std::cout << "Cache response: " << cache_response.message() << std::endl;
            if (!cache_response.success()) {
                std::cerr << "Cache not the latest try sending the request again: " << std::endl;
                file_id = putContentToServer(store_name, file_path, file_id, stub);
            } else {
                std::cout << "File already in cache" << std::endl;
                markDirty(file_id);
            }
        } else {
            // Put file to server and cache
            file_id = putContentToServer(store_name, file_path, file_id, stub);
        }
        return file_id;
    }
}; 