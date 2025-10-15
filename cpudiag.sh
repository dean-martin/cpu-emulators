#!/bin/env bash
set -euo pipefail
pushd build
g++ ../src/cpudiag.cc ../src/8080.cc -o cpudiagtest
./cpudiagtest
popd
