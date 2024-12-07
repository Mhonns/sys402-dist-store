/**
 * @file hearty-store-get.cpp
 * @author Nathadon Samairat
 * @brief This program provides functionality to retrieve objects 
 *        from a distributed storage system. It handles metadata loading, 
 *        block reconstruction using parity, and direct block reading.
 * @version 0.1
 * @date 2024-11-27
 * 
 * @copyright Copyright (c) 2024
 */
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include "hearty-store-server.hpp"

/**
 * @brief Retrieve an object by its ID from the store or reconstruct it if necessary.
 * 
 * @param store_id      - ID of the store.
 * @param object_id     - ID of the object to retrieve.
 * @return true         - Successfully retrieved the object.
 * @return false        - Failed to retrieve the object.
 */
std::string get(int store_id, const std::string object_id) {
    // Check if we need to recover first
    recoverFromLog(store_id);
    
    // Load metadata
    StoreMetadata store_metadata{};
    std::vector<BlockMetadata> block_metadata(NUM_BLOCKS);
    
    std::ifstream meta_file(utils::getMetadataPath(store_id), std::ios::binary);
    if (!meta_file) {
        std::cerr << "Failed to open metadata file" << std::endl;
        return "";
    }

    // Read metadata
    meta_file.read(reinterpret_cast<char*>(&store_metadata), sizeof(StoreMetadata));
    for (auto& block : block_metadata) {
        meta_file.read(reinterpret_cast<char*>(&block), sizeof(BlockMetadata));
        if (meta_file.fail()) {
            std::cerr << "Failed to read block metadata" << std::endl;
            return {};
        }
    }

    // Find block with matching object id
    int block_num = -1;
    for (size_t i = 0; i < NUM_BLOCKS; i++) {
        if (block_metadata[i].is_used && 
            strncmp(block_metadata[i].object_id, object_id.c_str(), 
                    sizeof(block_metadata[i].object_id) - 1) == 0) {
            block_num = i;
            break;
        }
    }

    if (block_num == -1) {
        std::cerr << "Object not found: " << object_id << std::endl;
        return "";
    }

    // Read data
    std::ifstream data_file(utils::getDataPath(store_id), std::ios::binary);
    data_file.seekg(block_num * BLOCK_SIZE);
    std::vector<char> buffer(block_metadata[block_num].data_size);
    data_file.read(buffer.data(), block_metadata[block_num].data_size);
    
    if (!data_file) {
        std::cerr << "Failed to read block data" << std::endl;
        return "";
    }

    if (buffer.empty()) {
        std::cerr << "Buffer is empty" << std::endl;
        return "";
    }
    
    // Create string from buffer without assuming null-termination
    return std::string(buffer.data(), block_metadata[block_num].data_size);
}