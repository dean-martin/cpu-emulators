#!/bin/env bash
set -euo pipefail
pushd build
gcc ../src/cpudiag.c -o cpudiagtest
./cpudiagtest
popd
