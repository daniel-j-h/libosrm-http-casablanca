#!/usr/bin/env bash

set -ex


# Casablanca Fetch
cmake -E make_directory third_party
cmake -E chdir third_party git clone --depth 1 --branch v2.8.0 https://github.com/Microsoft/cpprestsdk.git casablanca

# Casablanca Build
cmake -E make_directory third_party/casablanca/Release/build
cmake -E chdir third_party/casablanca/Release/build cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=OFF -G Ninja
cmake -E chdir third_party/casablanca/Release/build cmake --build .


# OSRM Fetch
cmake -E make_directory third_party
cmake -E chdir third_party git clone --depth 1 --branch rewrite/new-api https://github.com/Project-OSRM/osrm-backend.git osrm

# OSRM Build
cmake -E make_directory third_party/osrm/build
cmake -E chdir third_party/osrm/build cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake -E chdir third_party/osrm/build cmake --build .
