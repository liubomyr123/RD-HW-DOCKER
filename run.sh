#!/usr/bin/env bash

rm -rf build
cmake -S . -B build &&
cmake --build build &&
# ./build/homework_04/ugv_odometry homework_04/data/straight.txt
./build/homework_04/ugv_odometry homework_04/data/combined.txt
# ./build/homework_04/ugv_odometry homework_04/data/turn.txt