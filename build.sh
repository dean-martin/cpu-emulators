#!/bin/env bash
# @see: https://github.com/libsdl-org/SDL/blob/main/docs/INTRO-cmake.md
cmake -S . -B build
cmake --build build
if [ ! -z sound/shoot.wav ]; then
	pushd sound
	tar -xf all.tar.zst
	popd
fi
cp -r sound/ build/
