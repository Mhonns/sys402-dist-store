#include <grpcpp/grpcpp.h>
#include <proto/hello.grpc.pb.h>
#include <proto/hello.pb.h>
#include <iostream>

int main() {
    // Create a channel to connect to the server.
    auto channel = grpc::CreateChannel("localhost:9999", grpc::InsecureChannelCredentials());

    // Create a stub for calling the gRPC service.
    std::unique_ptr<ProcessingSevice::Stub> stub = ProcessingSevice::NewStub(channel);

    // Prepare the request message.
    Point3 request;
    request.set_x(10.0); // Example value for x
    request.set_y(20.0); // Example value for y
    request.set_z(3.0); // Example value for z

    // Prepare the response message.
    Numeric response;

    // Context for the client call.
    grpc::ClientContext context;

    // Make the RPC call to the `computeSum` method.
    grpc::Status status = stub->computeSum(&context, request, &response);

    // Process the response.
    if (status.ok()) {
        std::cout << "Sum of the points: " << response.value() << std::endl;
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
    }

    return 0;
}
