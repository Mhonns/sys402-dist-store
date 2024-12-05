#!/bin/bash
# Clean the storage and build directory
rm -rf /tmp/hearty
# rm -rf ../build
# mkdir ../build

# Create a dummy file for testing
echo "Dummy text for put and get test" > ../build/TestTransfer.txt
echo "Dummy text for Caching test" > ../build/TestCaching.txt

# Build and run the server
cd ../build
cmake ..
make
./hearty-store-server