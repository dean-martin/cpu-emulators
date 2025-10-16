#include "8080.h"
#include "common.h"
#include <stdio.h>

global_variable State8080 State;

// @TODO: green colors because it's cool
void pass(const char *msg);
void fail(const char *msg);
void reset(State8080 *cpu);

void test_DAD_RP(State8080 *cpu)
{
// (H) (L) <- (H) (L) + (rh) (rl)
// The content of the register pair rp is added to the
// content of the register pair Hand L. The result is
// placed in the register pair Hand L. Note: Only the
// CY flag is affected. It is set if there is a carry out of
// the double precision add; otherwise it is reset.
	reset(cpu);
	cpu->memory[0] = DAD_B;
	cpu->h = 0x12;
	cpu->l = 0x34;
}

void test_DCX_RP(State8080 *cpu)
{
// DCX rp (Decrement register pair)
// (rh) (rl) <- (rh) (rl) - 1
// The content of the register pair rp is decremented by
// one. Note: No condition flags are affected.
	// B
	cpu->b = 0x12;
	cpu->c = 0x00;
	cpu->pc = 0;
	cpu->memory[0] = DCX_B;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'b') == (0x1200-1));
	// D
	cpu->d = 0x13;
	cpu->e = 0x00;
	cpu->pc = 0;
	cpu->memory[0] = DCX_D;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'd') == (0x1300-1));
	// H
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
	cpu->a = 0;
	cpu->b = 0;
	cpu->c = 0;
	cpu->d = 0;
	cpu->e = 0;
	cpu->h = 0;
	cpu->l = 0;
	cpu->sp = 0;
	cpu->pc = 0;
}
