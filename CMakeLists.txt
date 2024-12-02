cmake_minimum_required(VERSION 3.20)
project(heartystore)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_library(protolib proto/hearty-store.proto)
target_link_libraries(protolib gRPC::grpc++)
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET protolib LANGUAGE cpp)
protobuf_generate(TARGET protolib LANGUAGE grpc
GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_executable(server src/hearty-store-server.cpp)
target_link_libraries(server protolib)