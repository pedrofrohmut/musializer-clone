#! /usr/bin/env sh

set -xe # debug mode and exit on error

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:./build

echo $1

./build/musializer.out $1
