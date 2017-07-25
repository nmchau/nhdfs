#!/usr/bin/env bash

mkdir -p build_deps/libhdfs3/build || exit 1
mkdir -p build_deps/libhdfs3/dist || exit 1
cd build_deps/libhdfs3/build || exit 1
../../../deps/bootstrap --prefix=../dist || exit 1
make && make install || exit 1
cd ../ || exit 1
rm -fr build