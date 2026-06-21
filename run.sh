#!/usr/bin/env bash

set -e

case "$1" in
    asan)
        echo "=== Building project ==="
        rm -rf build
        cmake --preset asan
        cmake --build --preset asan

        echo "=== Running with AddressSanitizer + LeakSanitizer ==="
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:verbosity=1 \
        cd "build/asan/homework_07"
        ./ballistics_hw_7
        ;;

    ubasan)
        echo "=== Building ASAN + UBSAN ==="
        rm -rf build
        cmake --preset asan-ubsan
        cmake --build --preset asan-ubsan

        echo "=== Running ASAN + UBSAN ==="
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:verbosity=1 \
        UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
        ./build/asan-ubsan/homework_07/ballistics_hw_7
        ;;

    valgrind)
        echo "=== Building project ==="
        rm -rf build
        cmake --preset debug
        cmake --build --preset debug

        echo "=== Running with Valgrind ==="
        cd "build/debug/homework_07"
        valgrind \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --error-exitcode=1 \
            ./ballistics_hw_7
        ;;

    *)
        echo "Usage:"
        echo "  ./run.sh asan"
        echo "  ./run.sh valgrind"
        exit 1
        ;;
esac
