#!/bin/bash

# Checks if an argument was provided
if [ -z "$1" ]; then
    echo "Usage: $0 [debug|release]"
    exit 1
fi

MODE=$(echo "$1" | tr '[:upper:]' '[:lower:]')

if [ "$MODE" == "debug" ]; then
    BUILD_DIR_NAME="_DEBUG_"
    CMAKE_BUILD_TYPE="DEBUG"
elif [ "$MODE" == "release" ]; then
    BUILD_DIR_NAME="_RELEASE_"
    CMAKE_BUILD_TYPE="RELEASE"
else
    echo "Invalid build mode specified. Use 'debug' or 'release'."
    exit 1
fi

# Gets the directory where the script is located, and change to it
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
cd "$SCRIPT_DIR"

# Creates and enters the specific build directory
if [ ! -d "$BUILD_DIR_NAME" ]; then
    mkdir "$BUILD_DIR_NAME"
fi
cd "$BUILD_DIR_NAME"

cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. -G "Unix Makefiles"
make

