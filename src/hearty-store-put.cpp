#include "hearty-store-common.hpp"
#include "hearty-store-cache.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <store_name> <file_path>" << std::endl;
        return 1;
    }

    auto stub = create_stub();
    ClientCache cache;
    
    std::string file_id = cache.cacheablePutRequest(argv[1], argv[2], stub.get());
    if (file_id.empty()) {
        std::cout << "Failed to get file id" << std::endl;
        return 1;
    }
    
    std::cout << "File id: " << file_id << std::endl;
    
    return 0;
} 