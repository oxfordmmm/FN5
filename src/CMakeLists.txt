cmake_minimum_required (VERSION 3.10)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -O3 -pthread -Wall")

# Our project name
project (fn5)

#Compile all of the classes defined
file(GLOB src 
    "fn5.cpp"
    "argparse.cpp" 
    "sample.cpp"
    "comparisons.cpp"
)

add_executable(fn5 ${src})

# Make sure the compiler can access the headers
INCLUDE_DIRECTORIES(../src/include)
