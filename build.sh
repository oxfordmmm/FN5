#/bin/bash

set -e

mkdir -p build
rm -rf build
mkdir -p build
cd build

#Compile fn5
[ -e CMakeCache.txt ] && rm CMakeCache.txt
cmake ../src
make

#Move fn5 to main level
mv fn5 ..
cd ..

