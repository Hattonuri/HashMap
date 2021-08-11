cmake_minimum_required(VERSION 3.17)
project(HashMap)

set(CMAKE_CXX_STANDARD 14)

add_executable(HashMap hash_map.h unit_tests.cpp)