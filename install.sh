#!/bin/sh

set -e

if ! command -v cmake >/dev/null 2>&1; then
  echo 'cmake: program not found!' >&2
  exit 1
fi

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
