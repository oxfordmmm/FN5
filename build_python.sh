#/bin/bash

set -e

mkdir -p build
cd build

#Compile fn5
[ -e CMakeCache.txt ] && rm CMakeCache.txt
cmake ../src/python
make

#Move fn5 to main level
# mv fn5 ..
cd ..

