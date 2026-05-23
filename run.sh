#!/usr/bin/env bash

rm -rf build
cmake -S . -B build &&
cmake --build build &&
./build/homework_05/hw_05 homework_05/data/good.txt