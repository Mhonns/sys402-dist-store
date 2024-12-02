#include <grpc/grpc.h>
#include <proto/hello.grpc.pb.h>
#include <proto/hello.pb.h>
#include <grpcpp/server_builder.h>

class ProcessingImpl : public ProcessingSevice::Service {
    ::grpc::Status computeSum(::grpc::ServerContext* context, const ::Point3* request, ::Numeric* response) {
        std::cout << "Called!" << std::endl;
        response->set_value(request->x() + request->y() + request->z());
        return grpc::Status::OK;  
    }
};

int main() {
    ProcessingImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:9999", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    server->Wait();

    return 0;
}