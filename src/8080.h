#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

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

// @TODO maybe rename to "Word" or "Data Word" is u16 iirc
// rp:
// B represents the B,C pair with B as the high-order register and C as the
// low-order register;
u16 Bytes(u8 high, u8 low);
u16 RegisterPair(State8080 *cpu, char pair);

char *ByteToBinary(unsigned char num);
void PrintBinary(unsigned x);
int ParseInt(char *str);

bool InitCPU(State8080 *CPU);

int Disassemble8080Op(unsigned char *buffer, int pc);
int Emulate8080Op(State8080 *state);
int LoadROMFile(State8080 *state, const char *FileName);

inline void
PUSH_PC(State8080 *state);

// why did i start this
// @TODO: a lookup table would be great
#define NOP 0x00
#define LXI_B 0x01
#define STAX_B 0x02
#define INX_B 0x03
#define INR_B 0x04
#define DCR_B 0x05
#define MVI_B 0x06
#define RLC 0x07
// -
#define DAD_B 0x09
#define LDAX_B 0x0a
#define DCX_B 0x0b
#define INR_C 0x0c
#define DCR_C 0x0d
#define MVI_C 0x0e
#define RRC 0x0f
// -
#define LXI_D 0x011
#define STAX_D 0x12
#define INX_D 0x13
#define INR_D 0x14
#define DCR_D 0x15
#define MVI_D 0x16
#define RAL 0x17
// - 
#define DAD_D 0x19
#define LDAX_D 0x1a
#define DCX_D 0x1b
#define INR_E 0x1c
#define DCR_E 0x1d
#define MVI_E 0x1e
#define RAR 0x1f // HAVE YOU PAID FOR WINRAR?
// - O_O
#define LXI_H 0x21
#define SHLD 0x22
#define INX_H 0x23
#define INR_H 0x24
#define DCR_H 0x25
#define MVI_H 0x26
#define DAA 0x27
// -
#define DAD_H 0x29
#define LHLD 0x2a
#define DCX_H 0x2b
#define INR_L 0x2c
#define DCR_L 0x2d
#define MVI_L 0x2e
#define CMA 0x2f
// -
#define LXI_SP 0x31
#define STA 0x32
#define INX_SP 0x33
#define INR_M 0x34
#define DCR_M 0x35
#define MVI_M 0x36
#define STC 0x37
// -
#define DAD_SP 0x39
#define LDA 0x3a
#define DCX_SP 0x3b
#define INR_A 0x3c
#define DCR_A 0x3d
#define MVI_A 0x3e
#define CMC 0x3f
#define MOV_B_B 0x40
#define MOV_B_C 0x41
#define MOV_B_D 0x42
#define MOV_B_E 0x43
#define MOV_B_H 0x44

#define CALL 0xcd
