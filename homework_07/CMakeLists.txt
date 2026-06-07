cmake_minimum_required(VERSION 3.20)

project(ballistics_hw_7 CXX)

add_library(ballistics_lib src/ballistics.cpp)
target_include_directories(ballistics_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(ballistics_lib PRIVATE -Wall -Wextra -pedantic)

add_library(logger_lib src/logger.cpp)
target_include_directories(logger_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(logger_lib PRIVATE -Wall -Wextra -pedantic)

add_executable(ballistics_hw_7 src/main.cpp)
target_link_libraries(ballistics_hw_7 PRIVATE ballistics_lib logger_lib)
target_compile_options(ballistics_hw_7 PRIVATE -Wall -Wextra -pedantic)

add_custom_command(
    TARGET ballistics_hw_7 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:ballistics_hw_7>/data

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data/config.json
        $<TARGET_FILE_DIR:ballistics_hw_7>/data/config.json

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data/ammo.json
        $<TARGET_FILE_DIR:ballistics_hw_7>/data/ammo.json

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data/targets.json
        $<TARGET_FILE_DIR:ballistics_hw_7>/data/targets.json
)
