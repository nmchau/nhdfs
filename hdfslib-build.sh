#!/usr/bin/env bash

mkdir -p build_deps/libhdfs3/build || exit 1
mkdir -p build_deps/libhdfs3/dist || exit 1
cd build_deps/libhdfs3/build || exit 1

##openssl mac os work around
DEP=""
platform=`uname`
if [[ "$platform" == 'Darwin' ]]; then
   DEP="--dependency=/usr/local/opt/openssl"
fi
../../../deps/bootstrap --prefix=../dist $DEP || exit 1
make && make install || exit 1
cd ../ || exit 1
rm -fr build