#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#ifndef __cplusplus__
#define inline  
#endif

// @TODO: clean up, make 8080.cc a proper include?

typedef struct ConditionCodes {
	u8 z:1;
	u8 s:1;
	u8 p:1;
	u8 cy:1;
	u8 ac:1; // not implemented.
	u8 pad:3; // padding to align to 8bit boundary? should review K&R. thought it auto happens, and not critical here too.
} ConditionCodes;

// @see: https://gcc.gnu.org/onlinedocs/gcc-4.4.4/gcc/Structure_002dPacking-Pragmas.html
// @see: https://stackoverflow.com/a/40643512
// Prevent padding here for register pair memory.
#pragma pack(push,1)
typedef struct State8080 {
    // @IMPORTANT: The register ordering is important here, see "_RP" functions.
    u8	a;
    u8	b;
    u8	c;
    u8	d;
    u8	e;
    u8	h;
    u8	l;
    // EOF
    u16 sp;
    u16 pc;
    u8 *memory;
    u8 interrupt_enabled;

    ConditionCodes cc;

    bool DebugPrint;
    u64 Steps;

    u8 int_enable;

} State8080;
#pragma pack(pop)

typedef union {
    // so we read in Big Endianness, but Little Endian looks like this, ok ok.
    struct {
		u8 low;
		u8 high;
    };
    u16 data;
} bytes; // @TODO: better name


char *ByteToBinary(unsigned char num);
void PrintBinary(unsigned x);
int ParseInt(char *str);

bool InitCPU(State8080 *CPU);

int Disassemble8080Op(unsigned char *buffer, int pc);
int Emulate8080Op(State8080 *state);
int LoadROMFile(State8080 *state, const char *FileName);

