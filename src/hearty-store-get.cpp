#include "hearty-store-common.hpp"
#include "hearty-store-cache.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <store_name> <file_identifier>" << std::endl;
        return 1;
    }

    auto stub = create_stub();
    
    getRequest request;
    getResponse response;
    grpc::ClientContext context;
    
    request.set_store_name(argv[1]);
    request.set_file_identifier(argv[2]);
    
    std::string accumulated_content;
    std::unique_ptr<grpc::ClientReader<getResponse>> reader = stub->Get(&context, request);
    
    std::cout << "Get request sent" << std::endl;
    while (reader->Read(&response)) {
        if (!response.success()) {
            std::cout << "Error: " << response.message() << std::endl;
            return 1;
        }
        accumulated_content += response.file_content();
    }
    
    grpc::Status status = reader->Finish();
    if (status.ok()) {
        std::cout << "Content from get:\n" << accumulated_content << std::endl;
    }
    
    return 0;
} 