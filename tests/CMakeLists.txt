cmake_minimum_required(VERSION 3.14)
project(vichess_tests)

find_package(GTest REQUIRED)

add_executable(test_visualizer
    test_visualizer.cpp
    
)
target_link_libraries(test_visualizer
    PRIVATE
    GTest::gtest_main
    vichess_analysis   # or link your core/gamestate as needed
)

add_executable(test_analysis
    test_analysis.cpp
)
target_link_libraries(test_analysis
    PRIVATE
    GTest::gtest_main
    vichess_analysis
)


add_executable(test_enpassant
    test_enpassant.cpp
)
target_link_libraries(test_enpassant
    PRIVATE
    GTest::gtest_main
    vichess_core       # or your library name
    vichess_analysis   # if needed
    # or any other needed libs
)

add_test(NAME EnPassantTests COMMAND test_enpassant)

enable_testing()
add_test(NAME VisualizerTests COMMAND test_visualizer)
add_test(NAME AnalysisTests COMMAND test_analysis)
