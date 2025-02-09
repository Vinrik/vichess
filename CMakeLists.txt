cmake_minimum_required(VERSION 3.14)
project(Vichess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(Qt6 6.2 COMPONENTS Widgets REQUIRED)

# Add subdirectories for each major module
add_subdirectory(src/core)
add_subdirectory(src/analysis)
add_subdirectory(src/engine)
add_subdirectory(src/gamestate)
add_subdirectory(src/gui)
add_subdirectory(tests)
