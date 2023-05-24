#include <gtest/gtest.h>
#include "test_argparse.cpp"
#include "test_sample.cpp"
#include "test_comparisons.cpp"

int main(int argc, char** argv){
    testing::InitGoogleTest();
    int returnCode = RUN_ALL_TESTS();
    return returnCode;
}