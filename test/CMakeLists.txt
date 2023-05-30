cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=c++20 -pthread -O0 -g -fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS}")

#Debug build run_tests
file(GLOB tests 
    "../src/argparse.cpp" 
    "../src/sample.cpp"
    "../src/comparisons.cpp"
    "test_runner.cpp"
)

include(FetchContent)
FetchContent_Declare(
googletest
URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(
run_tests
${tests}
)
target_link_libraries(
run_tests
gtest_main
)

include(GoogleTest)

# include(CodeCoverage)
# setup_target_for_coverage_lcov(${PROJECT_NAME}_coverage run_tests coverage)

