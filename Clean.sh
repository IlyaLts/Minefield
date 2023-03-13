#!/bin/bash

# Gets the directory where the script is located, and change to it
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
cd "$SCRIPT_DIR"

find . -name _DEBUG_ -exec rm -rf {} \;
find . -name _RELEASE_ -exec rm -rf {} \;

find -name '*.log' -delete;