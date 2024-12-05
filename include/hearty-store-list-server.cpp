/**
 * @file hearty-store-list.cpp
 * @author Nathadon Samairat
 * @brief   The program scans for directories in the base path corresponding to stores.
 *          Metadata for each store is loaded and displayed, including status, block usage, and HA group information.
 * @version 0.1
 * @date 2024-11-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include <fstream>
#include <iomanip>
#include "hearty-store-server.hpp"

/**
 * @brief Lists all available stores and their metadata.
 * @return string of the result 
 */
std::string list_stores() {
    if (!std::filesystem::exists(BASE_PATH)) {
        return "No store found";
    }

    bool found_stores = false;
    StoreMetadata metadata{};
    std::stringstream output;

    for (const auto& entry : std::filesystem::directory_iterator(BASE_PATH)) {
        if (!entry.is_directory()) continue;

        std::string dirname = entry.path().filename().string();
        if (dirname.substr(0, 6) != "store_") continue;

        int store_id = std::stoi(dirname.substr(6));
        
        std::string metadata_path = BASE_PATH + "/store_" + std::to_string(store_id) + META_FILENAME;
        std::ifstream meta_file(metadata_path, std::ios::binary);
        
        if (meta_file && meta_file.read(reinterpret_cast<char*>(&metadata), sizeof(StoreMetadata))) {
            found_stores = true;
            
            std::string status;
            if (status.empty()) {
                status = "active";
            }

            output << metadata.store_id << " - " 
                  << status 
                  << " (used: " << metadata.used_blocks << "/"
                  << metadata.total_blocks << " blocks)"
                  << std::endl;
        }
    }

    return found_stores ? output.str() : "No store found";
}