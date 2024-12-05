#ifndef HEARTY_STORE_COMMON_HPP
#define HEARTY_STORE_COMMON_HPP

#include <string>
#include <cstring>
#include <vector>
#include <filesystem>

const size_t BLOCK_SIZE = 1024 * 1024;              // 1MB
const size_t NUM_BLOCKS = 1024;                     // 1024 blocks
const std::string BASE_PATH = "/tmp/hearty";        // Default path to storage
const std::string DATA_FILENAME = "/data.bin";      // Actual data file name
const std::string META_FILENAME = "/metadata.bin";  // Meta data file name
const std::string STORE_DIR = "/store_";            // Default path to storage
const std::string PARITY_FILENAME = "/parity.bin";   // parity filename

struct BlockMetadata {
    bool is_used;           // Is this block currently storing an object
    char object_id[256];    // Unique identifier for the object in this block
    size_t data_size;       // Actual size of data in the block
    time_t timestamp;       // Last modification time will be used for object ID
};

struct StoreMetadata {
    int store_id;
    size_t total_blocks;     // Always 1024
    size_t block_size;       // Always 1MB
    size_t used_blocks;      // Number of blocks currently in use
};

// Utility functions
namespace utils {
    inline std::string getStorePath(int store_id) {
        return BASE_PATH + STORE_DIR + std::to_string(store_id);
    }

    inline std::string getDataPath(int store_id) {
        return getStorePath(store_id) + DATA_FILENAME;
    }

    inline std::string getMetadataPath(int store_id) {
        return getStorePath(store_id) + META_FILENAME;
    }

    // Checks if a store exists
    inline bool storeExists(int store_id) {
        return std::filesystem::exists(getStorePath(store_id));
    }
}

#endif // HEARTY_STORE_COMMON_HPP

bool initialize(int store_id);
std::string put(int store_id, const std::string& file_path);
std::string list_stores();
std::string get(int store_id, const std::string object_id);
bool destroy_store(int store_id);