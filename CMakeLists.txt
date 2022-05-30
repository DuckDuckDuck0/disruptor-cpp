cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_COMPILER /usr/bin/gcc-11)
set(CMAKE_CXX_COMPILER /usr/bin/g++-11)
set(CMAKE_CXX_STANDARD 20)

project(disruptor VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -O3")

add_library(disruptor INTERFACE)
target_include_directories(disruptor INTERFACE "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(test)
