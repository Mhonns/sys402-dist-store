#pragma once
#include <grpcpp/grpcpp.h>
#include <proto/hearty-store.grpc.pb.h>
#include <proto/hearty-store.pb.h>
#include <iostream>
#include <string>

inline std::unique_ptr<ProcessingService::Stub> create_stub() {
    auto channel = grpc::CreateChannel("localhost:2546", grpc::InsecureChannelCredentials());
    return ProcessingService::NewStub(channel);
} 