@echo off

cl -nologo src/8080.cc && 8080.exe rom\invaders.h 32

REM IF NOT EXIST build mkdir build
REM pushd build
REM
REM set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW -DHANDMADE_WIN32=1  -FAsc -Z7 -Fmchip8.map
REM set CommonLinkerFlags=-opt:ref user32.lib Gdi32.lib winmm.lib
REM
REM REM 64-bit build
REM cl %CommonCompilerFlags% w:\chip-8\main.cc /link %CommonLinkerFlags%
REM popd
