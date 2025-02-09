# src/engine/CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(vichess_engine)

# IMPORTANT: Make sure Automoc is ON here if the engine is a separate library
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(vichess_engine
    enginemanager.cpp
    enginemanager.h
)

target_include_directories(vichess_engine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

target_link_libraries(vichess_engine
    PRIVATE
    Qt6::Core
    # Possibly link Qt6::Widgets if your engine code uses any widget stuff
)
