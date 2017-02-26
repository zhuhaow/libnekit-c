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


cd $CURRENTPATH

