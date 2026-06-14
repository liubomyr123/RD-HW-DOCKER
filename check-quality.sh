#!/usr/bin/env bash

set -e

echo "===================="
echo "CLANG-FORMAT CHECK"
echo "===================="

FILES="
homework_06/include/ballistics.hpp
homework_06/src/ballistics.cpp
homework_06/src/main.cpp
homework_06/tests/ballistics_tests.cpp
"

clang-format --dry-run --Werror $FILES

echo "✔ clang-format OK"

echo ""
echo "===================="
echo "CLANG-TIDY"
echo "===================="

clang-tidy -p build/debug \
  homework_06/src/ballistics.cpp \
  homework_06/src/main.cpp \
  homework_06/tests/ballistics_tests.cpp

echo "✔ clang-tidy OK"