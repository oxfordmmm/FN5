#/bin/bash

set -xe

mkdir -p build
cd build

#Compile fn5 and test runner
cmake ../src
make

#Move fn5 to main level
mv fn5 ..
cd ..

