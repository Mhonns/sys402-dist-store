/**
 * @file hearty-store-destroy.cpp
 * @author Nathadon Samairat
 * @brief Implements the functionality to destroy a store.
 * @version 0.1
 * @date 2024-12-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <iostream>
#include <string>
#include <filesystem>
#include "hearty-store-common.hpp"
namespace fs = std::filesystem;

/**
 * @brief Destroys a store with the given ID.
 * 
 * @param store_id ID of the store to destroy.
 * 
 * @return true if the store is successfully destroyed; false otherwise
 */
bool destroy_store(int store_id) {
    std::string store_path = utils::getStorePath(store_id);
    if (!fs::exists(store_path)) {
        std::cerr << "Error: Store '" << store_id << "' does not exist" << std::endl;
        return false;
    }

    try {
        fs::remove_all(store_path);
        std::cout << "Successfully removed store: " << store_id << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error removing store: " << e.what() << std::endl;
        return false;
    }
}