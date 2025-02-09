# src/analysis/CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(vichess_analysis)

add_library(vichess_analysis
    evaluation.cpp
    search.cpp
)

target_include_directories(vichess_analysis
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

# We link to core if we need Board or Moves
target_link_libraries(vichess_analysis
    PRIVATE
    vichess_core
)
