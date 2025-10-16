#include "8080.h"
#include "common.h"
#include <stdio.h>

global_variable State8080 State;

// @TODO: green colors because it's cool
void pass(const char *msg);
void fail(const char *msg);
void reset(State8080 *cpu);

void test_ADD(State8080 *cpu)
{
// ADD r (Add Register)
// (A) <- (A) + (r)
// The content of register r is added to the content of the
// accumulator. The result is placed in the accumulator.

	// B
	reset(cpu);
	cpu->a = 0x3;
	cpu->b = 0x1;
	cpu->memory[0] = ADD_B;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x4);

	// C
	reset(cpu);
	cpu->a = 0x3;
	cpu->c = 0x1;
	cpu->memory[0] = ADD_C;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x4);

	// D
	reset(cpu);
	cpu->a = 0x3;
	cpu->d = 0x1;
	cpu->memory[0] = ADD_D;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x4);

	// E
	reset(cpu);
	cpu->a = 0x3;
	cpu->e = 0x1;
	cpu->memory[0] = ADD_E;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x4);

	// L
	reset(cpu);
	cpu->a = 0x3;
	cpu->l = 0x1;
	cpu->memory[0] = ADD_L;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x4);

	// M
	reset(cpu);
	cpu->a = 0x5;
	cpu->h = 0x12;
	cpu->l = 0x34;
	cpu->memory[0] = ADD_M;
	cpu->memory[0x1234] = 0x6;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x6+0x5);

	// A
	reset(cpu);
	cpu->a = 0x1;
	cpu->memory[0] = ADD_A;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x2);

	pass("ADD passed\n");
}

void test_DAD_RP(State8080 *cpu)
{
// (H) (L) <- (H) (L) + (rh) (rl)
// The content of the register pair rp is added to the
// content of the register pair H and L. The result is
// placed in the register pair H and L. Note: Only the
// CY flag is affected. It is set if there is a carry out of
// the double precision add; otherwise it is reset.
	// B
	reset(cpu);
	cpu->h = 0x12;
	cpu->l = 0x34;
	cpu->b = 0x42;
	cpu->c = 0x21;
	cpu->memory[0] = DAD_B;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'h') == (0x1234 + 0x4221));
	assert(cpu->cc.cy == 0);
	// D
	reset(cpu);
	cpu->h = 0xFF;
	cpu->l = 0xFF;
	cpu->d = 0x00;
	cpu->e = 0xFF;
	cpu->memory[0] = DAD_D;
	Emulate8080Op(cpu);
	u32 num = 0xFFFF + 0xFF;
	assert(RegisterPair(cpu, 'h') == (num & 0xFF));
	assert(cpu->cc.cy == 1);
	// H
	reset(cpu);
	cpu->h = 0x12;
	cpu->l = 0x34;
	cpu->memory[0] = DAD_H;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'h') == (0x1234 + 0x1234));
	assert(cpu->cc.cy == 0);
	// SP
	reset(cpu);
	cpu->h = 0x00;
	cpu->l = 0x21;
	cpu->sp = 0x1234;
	cpu->memory[0] = DAD_SP;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'h') == (0x0021 + 0x1234));
	assert(cpu->cc.cy == 0);

	pass("DAD_RP passed\n");
}

void test_DCX_RP(State8080 *cpu)
{
// DCX rp (Decrement register pair)
// (rh) (rl) <- (rh) (rl) - 1
// The content of the register pair rp is decremented by
// one. Note: No condition flags are affected.
	// B
	reset(cpu);
	cpu->b = 0x12;
	cpu->c = 0x00;
	cpu->pc = 0;
	cpu->memory[0] = DCX_B;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'b') == (0x1200-1));
	// D
	reset(cpu);
	cpu->d = 0x13;
	cpu->e = 0x00;
	cpu->pc = 0;
	cpu->memory[0] = DCX_D;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'd') == (0x1300-1));
	// H
	reset(cpu);
	cpu->h = 0x14;
	cpu->l = 0x00;
	cpu->pc = 0;
	cpu->memory[0] = DCX_H;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'h') == (0x1400-1));
	pass("DCX_RP passed\n");
}

void test_STAX_D(State8080 *cpu)
{
// STAX rp (Store accumulator indirect)
// ((rp)) <- (A)
// The content of register A is moved to the memory lo-
// cation whose address is in the register pair rp. Note:
// only register pairs rp=B (registers B and C) or rp=D
// (registers D and E) may be specified.
	reset(cpu);
	cpu->a = 25;
	cpu->d = 0x12;
	cpu->e = 0x34;
	cpu->pc = 0;
	cpu->memory[0] = STAX_D;
	Emulate8080Op(cpu);
	assert(cpu->memory[0x1234] == 25);
	pass("STAX_D passed\n");
}

int main(int argc, char **argv)
{
	if(!InitCPU(&State))
	{
		fail("failed to init CPU\n");
	}
	test_ADD(&State);
	test_DAD_RP(&State);
	test_DCX_RP(&State);
	test_STAX_D(&State);
	printf("tests finished\n");
}

void pass(const char *msg)
{
	printf(msg);
}

void fail(const char *msg)
{
	// va_list args;
	fprintf(stderr, msg);
	exit(1);
}

void reset(State8080 *cpu)
{
	u8 *mem = cpu->memory;
	memset(cpu, 0, sizeof(State8080));
	cpu->memory = mem;
}
