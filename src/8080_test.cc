#include "8080.h"
#include "common.h"
#include <stdio.h>

global_variable State8080 State;

// @TODO: green colors because it's cool
void pass(const char *msg);
void fail(const char *msg);
void reset(State8080 *cpu);

void test_JNZ(State8080 *cpu)
{
// Jcondition addr	(Conditional jump)
// If (CCC),
// 	(PC) <- (byte 3) (byte 2)
// If the specified condition is true, control is trans-
// ferred to the instruction whose address is specified in
// byte 3 and byte 2 of the current instruction; other-
// wise, control continues sequentially.

	// JNZ
	// CCC == 000
	// NZ = not zero (Z = 0) (condition flags)


}

// @TODO: Assert flag changes
void test_DCR(State8080 *cpu)
{
// DCR r	(Decrement Register)
// (r) <- (r)-1
// The content of register r is decremented by one.
// Note: All condition flag~ except CY are affected.

	// DCR B
	reset(cpu);
	cpu->b = 0x5;
	cpu->memory[0] = DCR_B;
	Emulate8080Op(cpu);
	assert(cpu->b == 0x4);

	// DCR M
	reset(cpu);
	cpu->h = 0x12;
	cpu->l = 0x34;
	cpu->memory[0] = DCR_M;
	cpu->memory[0x1234] = 0x6;
	Emulate8080Op(cpu);
	assert(cpu->memory[0x1234] == 0x5);

	// @TODO: other registers.

	// DCR A
	reset(cpu);
	cpu->a = 0x3;
	cpu->memory[0] = DCR_A;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x2);

	pass("DCR passed\n");
}

void test_LDA(State8080 *cpu)
{
// LOA addr	(Load Accumulator direct)
// (A) <- ((byte 3) (byte 2))
// The content of the memory location, whose address
// is specified in byte 2 and byte 3 of the instruction, is
// moved to register A.
	reset(cpu);
	// opcode lowbyte highbyte
	cpu->memory[0] = LDA;
	cpu->memory[1] = 0x34;
	cpu->memory[2] = 0x12;
	cpu->memory[0x1234] = 0x42;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x42);

	pass("LDA passed\n");
}

void test_ANA(State8080 *cpu)
{
// ANA r	(AND Register)
// (A) <- (A) /\ (r)
// The content of register r is logically anded with the
// content of the accumulator. The result is placed in
// the accumulator. The CY flag is cleared.

	// B
	reset(cpu);
	cpu->a = 0x32;
	cpu->b = 0xF0;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_B;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x30);
	assert(cpu->cc.cy == 0);

	// C
	reset(cpu);
	cpu->a = 0x32;
	cpu->c = 0xF;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_C;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x02);
	assert(cpu->cc.cy == 0);

	// D
	reset(cpu);
	cpu->a = 0x35;
	cpu->d = 0xF;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_D;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x05);
	assert(cpu->cc.cy == 0);

	// E
	reset(cpu);
	cpu->a = 0x35;
	cpu->e = 0xF;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_E;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x05);
	assert(cpu->cc.cy == 0);

	// H
	reset(cpu);
	cpu->a = 0x35;
	cpu->h = 0xF;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_H;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x05);
	assert(cpu->cc.cy == 0);

	// L
	reset(cpu);
	cpu->a = 0x35;
	cpu->l = 0xF;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_L;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x05);
	assert(cpu->cc.cy == 0);

	// M
	reset(cpu);
	cpu->a = 0x35;
	cpu->h = 0x12;
	cpu->l = 0x34;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_M;
	cpu->memory[0x1234] = 0x0F;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x05);
	assert(cpu->cc.cy == 0);

	// A
	reset(cpu);
	cpu->a = 0x35;
	cpu->cc.cy = 1;
	cpu->memory[0] = ANA_A;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x35 & 0x35);
	assert(cpu->cc.cy == 0);

	pass("ANA passed\n");
}

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
	test_LDA(&State);
	test_ADD(&State);
	test_DCR(&State);
	test_DAD_RP(&State);
	test_DCX_RP(&State);
	test_STAX_D(&State);
	test_ANA(&State);
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
