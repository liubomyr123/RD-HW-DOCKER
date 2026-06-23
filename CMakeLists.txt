cmake_minimum_required(VERSION 3.20)

project(section2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_ASAN "Enable AddressSanitizer" ON)

enable_testing()
include(CTest)

# clangd читає compile_commands.json і завдяки цьому бачить ті самі include
# paths, defines і flags, що й реальна CMake-збірка.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Домашні роботи. По мірі появи додаємо кожну окремим add_subdirectory.
# add_subdirectory(homework_04)
# add_subdirectory(homework_05)
# add_subdirectory(homework_06)
add_subdirectory(homework_07)

# Demo-код для заняття 2.4: локальне і віддалене відлагодження через GDB.
# add_subdirectory(demos/lesson_2_4/debug_probe)
