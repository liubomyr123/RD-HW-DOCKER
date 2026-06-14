#!/usr/bin/env bash

set -e

rm -rf build

for preset in debug 
# release relwithdebinfo; 
do
    echo "===================="
    echo "BUILD: $preset"
    echo "===================="

    cmake --preset $preset
    cmake --build --preset $preset

    echo "Running tests..."
    ctest --test-dir build/$preset \
          --output-on-failure \
          --verbose \
          --progress
done
