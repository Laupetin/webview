#!/bin/bash

# Go to repository root
cd "$(dirname "$0")/.." || exit 2

CLANG_FORMAT_BIN="${CLANG_FORMAT_BIN:-clang-format}"

find ./examples ./include -iname '*.h' -o -iname '*.cpp' | xargs $CLANG_FORMAT_BIN --verbose -i
