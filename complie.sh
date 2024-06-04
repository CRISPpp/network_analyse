#!/bin/bash

rm ./server
rm ./client

g++ ./server.cpp -o server
g++ ./client.cpp -o client
