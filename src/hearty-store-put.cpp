#include "hearty-store-common.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <store_name> <file_path>" << std::endl;
        return 1;
    }

    auto stub = create_stub();
    
    putRequest request;
    putResponse response;
    grpc::ClientContext context;
    
    request.set_store_name(argv[1]);
    request.set_file_path(argv[2]);
    
    grpc::Status status = stub->Put(&context, request, &response);
    
    std::cout << "Put request sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: Success with id: " << response.message() << std::endl;
    }
    
    return 0;
} 