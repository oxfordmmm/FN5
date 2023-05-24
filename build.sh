#/bin/bash

set -e

mkdir -p build
cd build

#Compile fn5 and test runner
cmake ../src
make

#Move fn5 to main level & test runner to test
mv fn5 ..
mv run_tests ../test
cd ..

