#!/bin/sh

cmake -B build
cmake --build build
cmake -DCMAKE_BUILD_TYPE=Release build
cd build
sudo make install
