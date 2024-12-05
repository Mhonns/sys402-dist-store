# Install required dependencies
sudo apt update
sudo apt install -y build-essential autoconf libtool pkg-config cmake git

# Clone and install Protocol Buffers
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git submodule update --init --recursive
./autogen.sh
./configure
make
sudo make install
sudo ldconfig

# Go back to parent directory
cd ..

# Clone and install gRPC
git clone --recurse-submodules -b v1.60.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      ../..
make -j $(nproc)
sudo make install
sudo ldconfig

# Verify installations
protoc --version
which grpc_cpp_plugin