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

struct BlockMetadata {
    bool is_used;           // Is this block currently storing an object
    char object_id[32];     // Unique identifier for the object in this block
    size_t data_size;       // Actual size of data in the block
    time_t timestamp;       // Last modification time will be used for object ID
    char file_path[128];    // File path of the object will be used for replacement
};

struct StoreMetadata {
    int store_id;
    size_t total_blocks;     // Always 1024
    size_t block_size;       // Always 1MB
    size_t used_blocks;      // Number of blocks currently in use
};

struct LogEntry {
    enum LogType {
        ALLOCATE,
        PUT_FILE,
        ADD_ENTRY,
        COMMIT
    } type;
    int block_index;
    std::string checksum;
    std::string old_block_data;
    std::string object_id;
    size_t data_size;
    std::string file_path;
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

    inline std::string getLogPath(int store_id) {
        return getStorePath(store_id) + "-log.txt";
    }

    // Checks if a store exists
    inline bool storeExists(int store_id) {
        return std::filesystem::exists(getStorePath(store_id));
    }
}

std::string calculateMD5(const std::string& data);
void writeLogEntry(int store_id, const LogEntry& entry);
void recoverFromLog(int store_id);

#endif // HEARTY_STORE_COMMON_HPP

bool initialize(int store_id);
std::string put(int store_id, const std::string& file_path, const std::string& file_content);
std::string list_stores();
std::string get(int store_id, const std::string object_id);
bool destroy_store(int store_id);