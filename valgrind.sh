#!/usr/bin/env bash

docker run --rm -v ${PWD}:/app -w /app zhuhaow/libnekit-test sh -c "
mkdir /libnekit && \
cd /libnekit && \
cp -r /app/* . && \
./scripts/build_deps.sh && \
rm -rf build && \
mkdir build && \
cd build && \
cmake -DCMAKE_BUILD_TYPE=Debug .. && \
make && \
ctest -V
"
