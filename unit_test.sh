#!/bin/bash

set -e

#Make sure binaries are built
echo "Building..."
./build_tests.sh
echo "Done!"

echo


#Actually run the tests
echo "Testing..."
cd test
mkdir -p cases/dummy/saves
./run_tests
cd ..
echo "Done!"