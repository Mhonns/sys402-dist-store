#include "hearty-store-common.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <store_name>" << std::endl;
        return 1;
    }

    auto stub = create_stub();
    
    initRequest request;
    initResponse response;
    grpc::ClientContext context;
    
    request.set_store_name(argv[1]);
    grpc::Status status = stub->Init(&context, request, &response);
    
    std::cout << "Init request sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: " << response.message() << std::endl;
    }
    
    return 0;
} 