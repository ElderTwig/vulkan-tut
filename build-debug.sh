#!/bin/sh

mkdir -p shaders/build
cd shaders/build

ls ../*.* | xargs glslc -O -c

cd ../../
cmake --build build-debug -- -j8
