#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;

#define max(a,b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)

int isdigit(char c)
{
    return (c >= '0') && (c <= '9');
}


