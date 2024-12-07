/**
 * @file hearty-store-put.cpp
 * @author Nathadon Samairat
 * @brief Implements the functionality to store files into a block-based storage system.
 *        Provides replication, metadata synchronization, and high-availability group management.
 * @version 0.1
 * @date 2024-12-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include "hearty-store-server.hpp"

/**
 * @brief Generates a unique ID by combining a timestamp and a random number.
 * 
 * @return std::string A unique identifier string in the format "timestamp_randomNumber".
 *         Example: "1637359000000_1234".
 */
std::string generateUniqueId() {
    // Generate a random ID using timestamp and random number
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    std::random_device rd;      // Random device
    std::mt19937 gen(rd());     // RNG initialized with the seed from 'rd'
    // Generate random integers between 1000 and 9999 (inclusive)
    std::uniform_int_distribution<> dis(1000, 9999); 
    
    return std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

/**
 * @brief   Stores a file in the storage system and performs associated updates.
 * 
 * @param store_id      target store id
 * @param file_content  The content of the file to be stored.
 * @return std::string  The unique object ID assigned to the stored file, or an empty string on failure.
 */
std::string put(int store_id, const std::string& file_path, const std::string& file_content) {
    // Load metadata
    StoreMetadata store_metadata{};
    std::vector<BlockMetadata> block_metadata(NUM_BLOCKS);
    {
        std::ifstream meta_file(utils::getMetadataPath(store_id), std::ios::binary);
        std::ifstream data_file(utils::getDataPath(store_id), std::ios::binary);
        if (!meta_file) {
            std::cerr << "Failed to open metadata file" << std::endl;
            return "";
        }

        meta_file.read(reinterpret_cast<char*>(&store_metadata), sizeof(StoreMetadata));
        for (auto& block : block_metadata) {
            data_file.read(reinterpret_cast<char*>(&block), sizeof(BlockMetadata));
            if (data_file.fail()) {
                std::cerr << "Failed to read block metadata" << std::endl;
                return "";
            }
        }
    }

    std::string object_id = generateUniqueId();
    // Find free block or replace existing file if file path matches
    int block_num = -1;
    for (size_t i = 0; i < NUM_BLOCKS; i++) {
        // Find free block
        if (!block_metadata[i].is_used) {
            block_num = i;
            break;
        }
        // Replace existing file if file path matches but same id
        else if (block_metadata[i].file_path == file_path) {
            block_num = i;
            object_id = block_metadata[i].object_id;
            break; 
        }
    }

    // If no free blocks available, return empty string
    if (block_num == -1) {
        std::cerr << "No free blocks available" << std::endl;
        return "";
    }

    // Check file size
    uintmax_t file_size = file_content.size();
    if (file_size > BLOCK_SIZE) {
        std::cerr << "File too large (max 1MB)" << std::endl;
        return "";
    }

    // Write file to block
    {
        std::fstream data_file(utils::getDataPath(store_id), 
                             std::ios::binary | std::ios::in | std::ios::out);
        
        if (!data_file) {
            std::cerr << "Failed to open data.bin file" << std::endl;
            return "";
        }

        // Write data
        data_file << file_content;
        size_t data_size = file_content.size();

        // Update metadata
        block_metadata[block_num].is_used = true;
        strncpy(block_metadata[block_num].object_id, object_id.c_str(), 
                sizeof(block_metadata[block_num].object_id) - 1);
        block_metadata[block_num].data_size = file_size;
        block_metadata[block_num].timestamp = std::time(nullptr);
        strncpy(block_metadata[block_num].file_path, file_path.c_str(), 
                sizeof(block_metadata[block_num].file_path) - 1);
        store_metadata.used_blocks++;

        // Save metadata
        std::ofstream meta_file(utils::getMetadataPath(store_id), 
                                std::ios::binary);
        if (!meta_file) {
            std::cerr << "Failed to save metadata" << std::endl;
            return "";
        }

        meta_file.write(reinterpret_cast<char*>(&store_metadata), sizeof(StoreMetadata));
        if (meta_file.fail()) {
            std::cerr << "Failed to write store metadata" << std::endl;
            return "";
        }

        for (const auto& block : block_metadata) {            
            meta_file.write(reinterpret_cast<const char*>(&block), sizeof(BlockMetadata));
            if (meta_file.fail()) {
                std::cerr << "Failed to write block metadata" << std::endl;
                return "";
            }
        }

        meta_file.flush(); // Ensure all data is written to the file
        if (meta_file.fail()) {
            std::cerr << "Failed to close metadata file properly" << std::endl;
            return "";
        }

        return object_id;
    }
}
