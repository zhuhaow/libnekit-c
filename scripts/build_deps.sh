#!/usr/bin/env bash

CURRENTPATH=`pwd`

pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

LIBUV_PATH=$SCRIPTPATH/../deps/libuv/

cd $LIBUV_PATH
sh autogen.sh
./configure --prefix=`pwd`/../deps_build/
make
make install

CHECK_PATH=$SCRIPTPATH/../test/deps/check/
cd $CHECK_PATH
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=`pwd`/../../../../deps/deps_build/ ..
make
make install

cd $CURRENTPATH

