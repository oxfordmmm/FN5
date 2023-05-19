cmake_minimum_required (VERSION 3.0)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3 -pthread")

# Our project name
project (fn5)

#Compile all of the classes defined
file(GLOB src "*.cpp")

#Add all appropiate C++ files for running tests
file(GLOB tests 
"argparse.cpp" 
"sample.cpp"
"comparisons.cpp"
"../test/test_runner.cpp"
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(fn5 ${src})


# Make sure the compiler can access the items

INCLUDE_DIRECTORIES(../src/include)

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(../test)

add_executable(
  run_tests
  ${tests}
)
target_link_libraries(
  run_tests
  gtest_main
)

include(GoogleTest)
#gtest_discover_tests(run_tests)
