#!/bin/bash

if [ ! -d "./bin" ]; then
    mkdir ./bin
fi

rm -rf ./bin/*
rm -rf ./ebpf_for_service/.output
rm -rf ./ebpf_for_client/.output

g++ ./service/server.cpp -o ./bin/server
g++ ./service/client.cpp -o ./bin/client
g++ ./test/test.cpp -o ./bin/test
g++ ./test/readshm.c -o ./bin/readshm
g++ ./test/writeshm.c -o ./bin/writeshm
g++ ./test/test_server.cpp -o ./bin/test_server


make -C ./ebpf_for_service -j
make -C ./ebpf_for_client -j

rm -rf ./ebpf_for_service/.output
rm -rf ./ebpf_for_client/.output

./configure

if [ $? -eq 0 ]; then
    make
else
    echo "Error: configure failed."
    exit 1
fi

if [ $? -eq 0 ]; then
    make -C ./xdp -j
else
    echo "Error: configure failed."
    exit 1
fi