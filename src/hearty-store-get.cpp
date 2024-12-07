#include "hearty-store-common.hpp"
#include "hearty-store-cache.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <store_name> <file_identifier>" << std::endl;
        return 1;
    }

    auto stub = create_stub();
    
    ClientCache cache;
    std::string content = cache.cacheableGetRequest(argv[1], argv[2], stub);
    if (content.empty()) {
        std::cout << "Error: Content is empty" << std::endl;
        return 1;
    } else {
        std::cout << content << std::endl;
    }
    
    return 0;
} 