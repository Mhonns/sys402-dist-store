# cd ../build
# ./hearty-store-init 20
# ./hearty-store-put 20 TestTransfer.txt
# ./hearty-store-list
# ./hearty-store-get 20 <file_id>
# ./hearty-store-destroy 20

./client-coherence-handler 2547 &
./hearty-store-init 20
./hearty-store-put 20 TestTransfer.txt
./hearty-store-list
# ./hearty-store-get 20