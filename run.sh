#!/bin/bash

sudo apt update
sudo apt install build-essential cmake -y
cd ./build || exit
cmake ..
make
./RemoteCli
