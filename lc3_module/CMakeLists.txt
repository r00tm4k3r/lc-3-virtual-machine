cmake_minimum_required(VERSION 3.10)

project(lc3_module)

add_library(lc3_module STATIC lc3.cpp lc3.hpp)

target_include_directories(
    lc3_module 
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    )