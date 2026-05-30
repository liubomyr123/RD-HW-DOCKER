cmake_minimum_required(VERSION 3.20)

project(ballistics_check CXX)

add_library(ballistics_lib src/ballistics.cpp)
target_include_directories(ballistics_lib 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_options(ballistics_lib 
    PRIVATE 
    -Wall -Wextra -pedantic
)

add_executable(ballistics_check src/main.cpp)
target_link_libraries(ballistics_check 
    PRIVATE 
    ballistics_lib
)
target_compile_options(ballistics_check 
    PRIVATE 
    -Wall -Wextra -pedantic
)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(ballistics_tests
    tests/ballistics_tests.cpp
)

target_link_libraries(ballistics_tests
    PRIVATE
    ballistics_lib
    GTest::gtest_main
)

target_compile_options(ballistics_tests
    PRIVATE
    -Wall -Wextra -pedantic
)

gtest_discover_tests(ballistics_tests)
