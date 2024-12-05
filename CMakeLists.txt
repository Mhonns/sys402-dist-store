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

# Add a library for common hearty functionalities on the server side
add_library(hearty-store-init-server include/hearty-store-init-server.cpp)
target_include_directories(hearty-store-init-server PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-put-server include/hearty-store-put-server.cpp)
target_include_directories(hearty-store-put-server PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-list-server include/hearty-store-list-server.cpp)
target_include_directories(hearty-store-list-server PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-get-server include/hearty-store-get-server.cpp)
target_include_directories(hearty-store-get-server PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_library(hearty-store-destroy-server include/hearty-store-destroy-server.cpp)
target_include_directories(hearty-store-destroy-server PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Add executables for server and client (in directory src/)
add_executable(hearty-store-server src/hearty-store-server.cpp)
target_link_libraries(hearty-store-server protolib hearty-store-init-server hearty-store-put-server
                        hearty-store-list-server hearty-store-get-server hearty-store-destroy-server)

add_executable(hearty-store-init src/hearty-store-init.cpp)
add_executable(hearty-store-put src/hearty-store-put.cpp)
add_executable(hearty-store-list src/hearty-store-list.cpp)
add_executable(hearty-store-get src/hearty-store-get.cpp)
add_executable(hearty-store-destroy src/hearty-store-destroy.cpp)

# Link each executable with the required libraries
foreach(TARGET hearty-store-init hearty-store-put hearty-store-list hearty-store-get hearty-store-destroy)
    target_link_libraries(${TARGET} protolib)
endforeach()