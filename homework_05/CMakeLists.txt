cmake_minimum_required(VERSION 3.20)

project(telemetry_check CXX)

add_library(telemetry_lib src/telemetry.cpp)
target_include_directories(telemetry_lib 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_options(telemetry_lib 
    PRIVATE 
    -Wall -Wextra -pedantic
)

add_executable(telemetry_check src/main.cpp)
target_link_libraries(telemetry_check 
    PRIVATE 
    telemetry_lib
)
target_compile_options(telemetry_check 
    PRIVATE 
    -Wall -Wextra -pedantic
)