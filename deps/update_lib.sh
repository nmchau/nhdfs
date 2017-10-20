#!/usr/bin/env bash

set -ex

git submodule update
pushd incubator-hawq
git pull --ff-only
popd

rsync -r incubator-hawq/depends/libhdfs3/ libhdfs3

git add incubator-hawq
git add libhdfs3

echo "Please commit results"