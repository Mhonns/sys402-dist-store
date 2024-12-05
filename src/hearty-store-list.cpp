#include "hearty-store-common.hpp"

int main(int argc, char* argv[]) {
    auto stub = create_stub();
    
    listRequest request;
    listResponse response;
    grpc::ClientContext context;
    
    grpc::Status status = stub->List(&context, request, &response);
    
    std::cout << "List request sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: " << response.message();
    }
    
    return 0;
} 