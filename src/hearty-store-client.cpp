#include <grpcpp/grpcpp.h>
#include <proto/hearty-store.grpc.pb.h>
#include <proto/hearty-store.pb.h>
#include <iostream>

int main() {
    // Create a channel to connect to the server.
    auto channel = grpc::CreateChannel("localhost:2546", grpc::InsecureChannelCredentials());

    // Create a stub for calling the gRPC service.
    std::unique_ptr<ProcessingService::Stub> stub = ProcessingService::NewStub(channel);

    // Prepare the init request message.
    initRequest init_request;
    initResponse init_response;
    grpc::ClientContext init_context;
    init_request.set_store_name("20");
    grpc::Status status = stub->Init(&init_context, init_request, &init_response);
    std::cout << "Client: Init Sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: " << init_response.message() << std::endl;
    }

    // Prepare the put request message.
    putRequest put_request;
    putResponse put_response;
    grpc::ClientContext put_context;
    put_request.set_store_name("20");
    put_request.set_file_path("TestTransfer.txt");
    status = stub->Put(&put_context, put_request, &put_response);
    std::cout << "Client: Put Sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: Success with id:" << put_response.message() << std::endl;
    }

    // Prepare the list request message.
    listRequest list_request;
    listResponse list_response;
    grpc::ClientContext list_context;
    status = stub->List(&list_context, list_request, &list_response);
    std::cout << "Client: List Sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: " << list_response.message();
    }

    // Prepare the get request message.
    getRequest get_request;
    getResponse get_response;
    grpc::ClientContext get_context;
    get_request.set_store_name("20");
    get_request.set_file_identifier(put_response.message());

    // Read the response from the server
    std::unique_ptr<grpc::ClientReader<getResponse>> reader = stub->Get(&get_context, get_request);
    std::cout << "Client: Get Sent" << std::endl;
    if (status.ok()) {
        while (reader->Read(&get_response)) {
            if (!get_response.success()) {
                std::cout << "Error: " << get_response.message() << std::endl;
                return -1;
            }
            // Process the streamed content
            std::cout << get_response.file_content();
        }
        status = reader->Finish();
    }

    // Prepare the destroy request message.
    destroyRequest destroy_request;
    destroyResponse destroy_response;
    grpc::ClientContext destroy_context;
    destroy_request.set_store_name("20");
    status = stub->Destroy(&destroy_context, destroy_request, &destroy_response);
    std::cout << "Client: Destroy Sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: " << destroy_response.message() << std::endl;
    }

    // Prepare the another list request message.
    another_list_request.set_store_name("20");
    another_list_response.clear_message();
    status = stub->List(&another_list_context, another_list_request, &another_list_response);
    std::cout << "Client: Another List Sent" << std::endl;
    if (status.ok()) {
        std::cout << "Status: " << another_list_response.message();
    }

    return 0;
}
