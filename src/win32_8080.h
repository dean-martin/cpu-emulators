// @TODO: commons.h
#ifndef UNICODE
#define UNICODE
#endif

#include "8080.h"
#include <windows.h>
#include "common.h"

typedef struct win32_buffer
{
    // NOTE: Pixels are always 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;

    void *Memory;
    int MemorySize;

    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;

} win32_buffer;

typedef struct win32_window_dimension
{
    int Width;
    int Height;
} win32_window_dimension;
