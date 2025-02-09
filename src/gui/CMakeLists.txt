# src/gui/CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(vichess_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)  # needed for Qt signals/slots

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Compile your .qrc resource file
set(UI_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/resources.qrc
)

qt6_add_resources(RCC_SOURCES
    ${UI_RESOURCES}
)

add_executable(vichess_gui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    promotiondialog.cpp
    promotiondialog.h
    ${RCC_SOURCES}
)

# Link everything you need
target_link_libraries(vichess_gui
    PRIVATE
    Qt6::Widgets
    vichess_analysis
    vichess_engine
    vichess_core
    vichess_gamestate
)
