/**
 * @file hearty-store-init.cpp
 * @author Nathadon Samairat
 * @brief Initializes a new store by creating necessary files and metadata.
 * @version 0.1
 * @date 2024-12-03
 * 
 * @copyright Copyright (c) 2024
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <cstring>
#include "hearty-store-server.hpp"

/**
 * @brief Initializes a new store with the given ID.
 * 
 * @param store_id ID of the store to initialize.
 * 
 * @return true if the store is successfully initialized; false otherwise
 */
bool initialize(int store_id) {
    // Check if store already exists
    std::string store_path = BASE_PATH + STORE_DIR + std::to_string(store_id);
    if (utils::storeExists(store_id)) {
        std::cerr << "Store " << store_id << " already exists" << std::endl;
        return false;
    }

    // Create store directory
    try {
        std::filesystem::create_directories(store_path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create store directory: " << e.what() << std::endl;
        return false;
    }

    // Initialize metadata
    StoreMetadata store_metadata{
        .store_id = store_id,
        .total_blocks = NUM_BLOCKS,
        .block_size = BLOCK_SIZE,
        .used_blocks = 0,
    };
    std::vector<BlockMetadata> block_metadata(NUM_BLOCKS);  // Zero-initialized by default

    // Create and write metadata file
    std::string meta_path = store_path + META_FILENAME;
    {
        std::ofstream meta_file(meta_path, std::ios::binary);
        if (!meta_file) {
            std::cerr << "Failed to create metadata file" << std::endl;
            std::filesystem::remove_all(store_path);
            return false;
        }

        meta_file.write(reinterpret_cast<const char*>(&store_metadata), sizeof(StoreMetadata));
        for (const auto& block : block_metadata) {
            meta_file.write(reinterpret_cast<const char*>(&block), sizeof(BlockMetadata));
            if (meta_file.fail()) {
                std::cerr << "Failed to write block metadata" << std::endl;
                return false;
            }
        }

        if (meta_file.fail()) {
            std::cerr << "Failed to write metadata" << std::endl;
            std::filesystem::remove_all(store_path);
            return false;
        }
    }

    // Create and initialize data file
    std::string data_path = store_path + DATA_FILENAME;
    {
        std::ofstream data_file(data_path, std::ios::binary);
        if (!data_file) {
            std::cerr << "Failed to create data file" << std::endl;
            std::filesystem::remove_all(store_path);
            return false;
        }

        // Initialize the data file with zeros for all blocks
        std::vector<char> zeros(NUM_BLOCKS * BLOCK_SIZE, '\0');
        data_file.write(zeros.data(), zeros.size());
        if (data_file.fail()) {
            std::cerr << "Failed to initialize data file" << std::endl;
            std::filesystem::remove_all(store_path);
            return false;
        }

        if (data_file.fail()) {
            std::cerr << "Failed to write data" << std::endl;
            std::filesystem::remove_all(store_path);
            return false;
        }
    }

    return true;
}