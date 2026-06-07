#!/usr/bin/env bash

rm -rf build
cmake -S . -B build &&
cmake --build build &&

cd build/homework_07 && ./ballistics_hw_7