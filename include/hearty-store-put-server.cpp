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
#include <openssl/md5.h>
#include <sstream>

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

std::string calculateMD5(const std::string& data) {
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)data.c_str(), data.length(), result);
    
    std::stringstream ss;
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    return ss.str();
}

void writeLogEntry(int store_id, const LogEntry& entry) {
    std::ofstream log_file(utils::getLogPath(store_id), std::ios::app);
    // Write log entry details based on type
    log_file << static_cast<int>(entry.type) << "|";
    switch(entry.type) {
        case LogEntry::ALLOCATE:
            log_file << entry.block_index << "\n";
            break;
        case LogEntry::PUT_FILE:
            log_file << entry.block_index << "|" << entry.checksum << "|" 
                    << entry.old_block_data << "\n";
            break;
        case LogEntry::ADD_ENTRY:
            log_file << entry.object_id << "|" << entry.block_index << "|" 
                    << entry.data_size << "|" << entry.file_path << "\n";
            break;
        case LogEntry::COMMIT:
            log_file << "COMMIT\n";
            break;
    }
    log_file.flush();
}

// Helper function to allocate a block
int allocateBlock(int store_id, const std::string& file_path, std::vector<BlockMetadata>& block_metadata) {
    int block_num = -1;
    for (size_t i = 0; i < NUM_BLOCKS; i++) {
        if (!block_metadata[i].is_used) {
            block_num = i;
            break;
        }
        else if (block_metadata[i].file_path == file_path) {
            block_num = i;
            break;
        }
    }

    if (block_num != -1) {
        // Log block allocation
        LogEntry allocate_entry{LogEntry::ALLOCATE, block_num};
        writeLogEntry(store_id, allocate_entry);
        
        // Update block allocation in metadata
        block_metadata[block_num].is_used = true;
    }

    return block_num;
}

// Helper function to write content to a block
bool putContentToBlock(int store_id, int block_num, const std::string& file_content) {
    if (file_content.size() > BLOCK_SIZE) {
        std::cerr << "File too large (max 1MB)" << std::endl;
        return false;
    }

    // Get old data for WAL
    std::string old_data;
    {
        std::ifstream data_file(utils::getDataPath(store_id), std::ios::binary);
        data_file.seekg(block_num * BLOCK_SIZE);
        std::vector<char> buffer(BLOCK_SIZE);
        data_file.read(buffer.data(), BLOCK_SIZE);
        old_data = std::string(buffer.data(), BLOCK_SIZE);
    }

    // Log file content update
    LogEntry put_entry{
        LogEntry::PUT_FILE,
        block_num,
        calculateMD5(file_content),
        old_data
    };
    writeLogEntry(store_id, put_entry);

    // Write actual file content
    {
        std::fstream data_file(utils::getDataPath(store_id), 
                             std::ios::binary | std::ios::in | std::ios::out);
        data_file.seekp(block_num * BLOCK_SIZE);
        data_file << file_content;
    }

    return true;
}

// Helper function to update metadata
bool updateMetadata(int store_id, int block_num, const std::string& object_id, 
                   const std::string& file_path, uintmax_t file_size,
                   StoreMetadata& store_metadata, std::vector<BlockMetadata>& block_metadata) {
    // Log metadata update
    LogEntry metadata_entry{
        LogEntry::ADD_ENTRY,
        block_num,
        "",
        "",
        object_id,
        file_size,
        file_path
    };
    writeLogEntry(store_id, metadata_entry);

    // Update metadata
    strncpy(block_metadata[block_num].object_id, object_id.c_str(), 
            sizeof(block_metadata[block_num].object_id) - 1);
    block_metadata[block_num].data_size = file_size;
    block_metadata[block_num].timestamp = std::time(nullptr);
    strncpy(block_metadata[block_num].file_path, file_path.c_str(), 
            sizeof(block_metadata[block_num].file_path) - 1);
    store_metadata.used_blocks++;

    // Save metadata
    std::ofstream meta_file(utils::getMetadataPath(store_id), std::ios::binary);
    if (!meta_file) {
        return false;
    }

    meta_file.write(reinterpret_cast<char*>(&store_metadata), sizeof(StoreMetadata));
    for (const auto& block : block_metadata) {            
        meta_file.write(reinterpret_cast<const char*>(&block), sizeof(BlockMetadata));
    }
    meta_file.flush();

    return !meta_file.fail();
}

bool readStoreMetadata(int store_id, 
                      StoreMetadata& store_metadata, 
                      std::vector<BlockMetadata>& block_metadata) {
    std::ifstream meta_file(utils::getMetadataPath(store_id), std::ios::binary);
    std::ifstream data_file(utils::getDataPath(store_id), std::ios::binary);
    
    if (!meta_file) {
        std::cerr << "Failed to open metadata file" << std::endl;
        return false;
    }

    meta_file.read(reinterpret_cast<char*>(&store_metadata), sizeof(StoreMetadata));
    
    for (auto& block : block_metadata) {
        data_file.read(reinterpret_cast<char*>(&block), sizeof(BlockMetadata));
        if (data_file.fail()) {
            std::cerr << "Failed to read block metadata" << std::endl;
            return false;
        }
    }
    
    return true;
}

void recoverFromLog(int store_id) {
    std::ifstream log_file(utils::getLogPath(store_id));
    if (!log_file) return;

    std::vector<LogEntry> uncommitted_entries;
    bool found_commit = false;
    std::string line;
    
    // Read log entries until we find last COMMIT
    while (std::getline(log_file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::getline(ss, token, '|');
        
        int type = std::stoi(token);
        if (type == LogEntry::COMMIT) {
            found_commit = true;
            uncommitted_entries.clear();
            continue;
        }

        LogEntry entry;
        entry.type = static_cast<LogEntry::LogType>(type);
        
        // Parse entry based on type
        switch(entry.type) {
            case LogEntry::ALLOCATE:
                std::getline(ss, token);
                entry.block_index = std::stoi(token);
                break;
            case LogEntry::PUT_FILE:
                // Parse PUT_FILE entry
                std::getline(ss, token, '|');
                entry.block_index = std::stoi(token);
                std::getline(ss, token, '|');
                entry.checksum = token;
                std::getline(ss, token);
                entry.old_block_data = token;
                break;
            case LogEntry::ADD_ENTRY:
                // Parse ADD_ENTRY entry
                std::getline(ss, token, '|');
                entry.object_id = token;
                std::getline(ss, token, '|');
                entry.block_index = std::stoi(token);
                std::getline(ss, token, '|');
                entry.data_size = std::stoul(token);
                std::getline(ss, token);
                entry.file_path = token;
                break;
            default:
                break;
        }
        
        uncommitted_entries.push_back(entry);
    }
    
    // If we didn't find a commit, we need to rollback changes
    std::cout << "Uncommitted entries: " << uncommitted_entries.size() << std::endl;
    if (!uncommitted_entries.empty()) {
        // Load metadata
        StoreMetadata store_metadata{};
        std::vector<BlockMetadata> block_metadata(NUM_BLOCKS);
        for (auto it = uncommitted_entries.rbegin(); it != uncommitted_entries.rend(); ++it) {
            // Rollback each operation
            switch(it->type) {
                case LogEntry::ALLOCATE:
                    // Reset block allocation
                    std::cout << "Allocating block for " << it->file_path << std::endl;
                    allocateBlock(store_id, it->file_path, block_metadata);
                    break;
                case LogEntry::PUT_FILE:
                    // Restore old block data
                    std::cout << "Restoring old block data for " << it->file_path << std::endl;
                    putContentToBlock(store_id, it->block_index, it->old_block_data);
                    break;
                case LogEntry::ADD_ENTRY:
                    // Remove metadata entry
                    std::cout << "Removing metadata entry for " << it->file_path << std::endl;
                    updateMetadata(store_id, it->block_index, it->object_id, it->file_path,
                                  0, store_metadata, block_metadata);
                    break;
                default:
                    break;
            }
        }

        LogEntry commit_entry{LogEntry::COMMIT};
        writeLogEntry(store_id, commit_entry);
    }
    
    // Clear log file after recovery
    std::ofstream clear_log(utils::getLogPath(store_id), std::ios::trunc);
}

// Main put function
std::string put(int store_id, const std::string& file_path, const std::string& file_content) {
    // Check if we need to recover from previous crashes
    recoverFromLog(store_id);

    // 0. Load metadata
    StoreMetadata store_metadata{};
    std::vector<BlockMetadata> block_metadata(NUM_BLOCKS);
    if (!readStoreMetadata(store_id, store_metadata, block_metadata)) {
        return "";
    }

    std::string object_id = generateUniqueId();
    // 1. Find free block or replace existing file if file path matches
    int block_num = allocateBlock(store_id, file_path, block_metadata);

    // If no free blocks available, return empty string
    if (block_num == -1) {
        std::cerr << "No free blocks available" << std::endl;
        return "";
    }

    // 2. Write content to block
    if (!putContentToBlock(store_id, block_num, file_content)) {
        return "";
    }

    // 3. Update metadata
    if (!updateMetadata(store_id, block_num, object_id, file_path, file_content.size(), store_metadata, block_metadata)) {
        return "";
    }

    // 4. Write commit log
    LogEntry commit_entry{LogEntry::COMMIT};
    writeLogEntry(store_id, commit_entry);

    return object_id;
}
