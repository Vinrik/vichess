# src/core/CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(vichess_core)

# Create a library target for the files in core
add_library(vichess_core
    board.cpp
    moves.cpp
)

target_include_directories(vichess_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}        # so that #include "board.h" etc. works
)
