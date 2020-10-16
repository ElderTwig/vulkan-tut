#!/bin/sh
rm ./compile_commands.json
mkdir -p build-debug

cmake -Bbuild-debug \
      -DCLANG_TIDY=ON \
      -DCMAKE_BUILD_TYPE=Debug
      
      #-G Ninja \      
      #DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang \
      #-DBUILD_TESTING=ON \
