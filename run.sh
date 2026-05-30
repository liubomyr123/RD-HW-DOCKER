#!/usr/bin/env bash

rm -rf build

cmake --preset debug &&
cmake --build --preset debug &&
# ./build/debug/homework_05/telemetry_check homework_05/data/bad_invalid_number.txt
# ./build/debug/homework_05/telemetry_check homework_05/data/bad_missing_field.txt
# ./build/debug/homework_05/telemetry_check homework_05/data/bad_zero_delta.txt
# ./build/debug/homework_05/telemetry_check homework_05/data/empty.txt
# ./build/debug/homework_05/telemetry_check homework_05/data/good.txt

./build/debug/homework_06/ballistics_check homework_06/data/sample_vog17.txt

# cmake --preset release &&
# cmake --build --preset release &&
# ./build/release/homework_05/telemetry_check homework_05/data/bad_invalid_number.txt
# ./build/release/homework_05/telemetry_check homework_05/data/bad_missing_field.txt
# ./build/release/homework_05/telemetry_check homework_05/data/bad_zero_delta.txt
# ./build/release/homework_05/telemetry_check homework_05/data/empty.txt
# ./build/release/homework_05/telemetry_check homework_05/data/good.txt

# cmake --preset relwithdebinfo &&
# cmake --build --preset relwithdebinfo &&
# ./build/relwithdebinfo/homework_05/telemetry_check homework_05/data/bad_invalid_number.txt
# ./build/relwithdebinfo/homework_05/telemetry_check homework_05/data/bad_missing_field.txt
# ./build/relwithdebinfo/homework_05/telemetry_check homework_05/data/bad_zero_delta.txt
# ./build/relwithdebinfo/homework_05/telemetry_check homework_05/data/empty.txt
# ./build/relwithdebinfo/homework_05/telemetry_check homework_05/data/good.txt
