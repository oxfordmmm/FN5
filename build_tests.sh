#/bin/bash

set -e

mkdir -p build
cd build

#Compile fn5 and test runner
[ -e CMakeCache.txt ] && rm CMakeCache.txt
cmake ../test -Wno-dev
make

#Move test runner to test
mv run_tests ../test
cd ..

