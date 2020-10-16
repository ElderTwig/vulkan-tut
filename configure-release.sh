#!/bin/sh
rm ./compile_commands.json
mkdir -p build-release

cmake -Bbuild-release \
      -DCMAKE_BUILD_TYPE=Release

      #-DBUILD_TESTING=ON \
      #-G Ninja \

