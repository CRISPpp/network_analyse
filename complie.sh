#!/bin/bash

rm -rf ./bin/*
rm -rf ./ebpf_for_service/.output
rm -rf ./ebpf_for_client/.output

g++ ./service/server.cpp -o ./bin/server
g++ ./service/client.cpp -o ./bin/client
g++ ./test.cpp -o ./bin/test

make -C ./ebpf_for_service -j
make -C ./ebpf_for_client -j

rm -rf ./ebpf_for_service/.output
rm -rf ./ebpf_for_client/.output