cmake_minimum_required(VERSION 3.20)
project(myserver)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(protolib proto/hearty-store.proto)
target_link_libraries(protolib gRPC::grpc++)

target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET protolib LANGUAGE cpp)
protobuf_generate(TARGET protolib LANGUAGE grpc
GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Add a library for common hearty functionalities
add_library(hearty-store-init include/hearty-store-init.cpp)
target_include_directories(hearty-store-init PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-put include/hearty-store-put.cpp)
target_include_directories(hearty-store-put PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-list include/hearty-store-list.cpp)
target_include_directories(hearty-store-list PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-get include/hearty-store-get.cpp)
target_include_directories(hearty-store-get PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-destroy include/hearty-store-destroy.cpp)
target_include_directories(hearty-store-destroy PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(hearty-store-server src/hearty-store-server.cpp)
target_link_libraries(hearty-store-server protolib hearty-store-init hearty-store-put 
                        hearty-store-list hearty-store-get hearty-store-destroy)

add_executable(hearty-store-client src/hearty-store-client.cpp)
target_link_libraries(hearty-store-client protolib)