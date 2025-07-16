// @TODO: commons.h
#ifndef UNICODE
#define UNICODE
#endif

#include "8080.h"
#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef uint16_t u16;
typedef int32 bool32;

typedef struct win32_buffer
{
    // NOTE: Pixels are always 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;

    void *Memory;
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
