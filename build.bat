@echo off

IF NOT EXIST build mkdir build
pushd build

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW -DHANDMADE_WIN32=1  -FAsc -Z7 -Fmchip8.map
set CommonLinkerFlags=-opt:ref user32.lib Gdi32.lib winmm.lib

REM 64-bit build
cl %CommonCompilerFlags% w:\chip-8\main.cc /link %CommonLinkerFlags%
popd
