#!/bin/bash

sudo apt update
sudo apt install build-essentials cmake -y
cd ./build || exit
cmake ..
make
./RemoteCli
