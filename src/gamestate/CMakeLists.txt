# src/gamestate/CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(vichess_gamestate)

add_library(vichess_gamestate
    gamestate.cpp
    notation.cpp
    pgn.cpp
)

target_include_directories(vichess_gamestate
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

# Link to vichess_core if you use Board inside gamestate
target_link_libraries(vichess_gamestate
    PRIVATE
    vichess_core
)
