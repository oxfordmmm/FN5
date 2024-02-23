cmake_minimum_required (VERSION 3.10)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3 -pthread")

# Our project name
project (fn5)

add_definitions(-DPYBIND)
find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG        v2.11.1
)

FetchContent_GetProperties(pybind11)
if(NOT pybind11_POPULATED)
    FetchContent_Populate(pybind11)
    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
endif()

#Compile all of the classes defined
file(GLOB src "../*.cpp")

# Make sure the compiler can access the headers
INCLUDE_DIRECTORIES(../src/include pybind11_SOURCE_DIR)
pybind11_add_module(fn5 ${src})
