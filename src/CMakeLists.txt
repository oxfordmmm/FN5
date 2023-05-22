cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3 -pthread")

# Our project name
project (fn5)

#Compile all of the classes defined
file(GLOB src "*.cpp")

add_executable(fn5 ${src})

# Make sure the compiler can access the headers
INCLUDE_DIRECTORIES(../src/include)
