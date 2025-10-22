#include "8080.h"
#include "common.h"
#include <stdio.h>

global_variable State8080 State;

// @TODO: green colors because it's cool
void pass(const char *msg);
void fail(const char *msg);
void reset(State8080 *cpu);

// @TODO: an automated compliance test with steps progressed in the program
// counter would be welcome.

// @TODO: test
void test_STA_D8(State8080 *cpu)
{
}

// @TODO: test
void test_ANI(State8080 *cpu);

void test_CALL(State8080 *cpu)
{
// CALL addr		(Call)
// ((SP) - 1) <- (PCH)
// ((SP) - 2) <- (PCl)
// (SP) <- (SP) - 2
// (PC) <- (byte 3) (byte 2)
// The high-order eight bits of the next instruction ad-
// dress are moved to the memory location whose
// address is one less than the content of register SP.
// The low-order eight bits of the next instruction ad-
// dress are moved to the memory location whose
// address is two less than the content of register SP.
// The content of register SP is decremented by 2. Con-
// trol is transferred to the instruction whose address is
// specified in byte 3 and byte 2 of the current
// instruction.
	reset(cpu);
	cpu->pc = 0x100;
	cpu->memory[0x100] = CALL;
	cpu->memory[0x101] = 0x42; // low-order data (byte 2)
	cpu->memory[0x102] = 0x69; // high-order data (byte 3)
	cpu->memory[0x103] = INR_C;
	cpu->memory[0x6942] = INR_A;
	cpu->memory[0x6943] = RET;
	cpu->sp = 0x2000;
	Emulate8080Op(cpu);

	assert(cpu->memory[0x2000-1] == 0x01);
	assert(cpu->memory[0x2000-2] == 0x03);
	assert(cpu->sp == 0x2000-2);
	assert(cpu->pc == 0x6942);

	// INR_A
	Emulate8080Op(cpu);
	assert(cpu->a == 1);

// RET		(Return)
// 	(PCL) <- ((SP));
// 	(PCH) <- ((SP) + 1);
// 	(SP) <- (SP) + 2;
// The content of the memory location whose address
// is specified in register SP is moved to the low-order
// eight bits of register PC. The content of the memory
// location whose address is one more than the content
// of register SP is moved to the high-order eight bits of
// register PC. The content of register SP is incremented
// by 2.
	Emulate8080Op(cpu);
	assert(cpu->pc == 0x103);
	assert(cpu->sp == 0x2000);

	// INR_C (The next instruction after CALL)
	Emulate8080Op(cpu);
	assert(cpu->c == 1);

	pass("CALL + RET passed\n");
}

void test_LHLD(State8080 *cpu)
{
// LHLD addr		(Load Hand L direct)
// (L) <- ((byte 3)(byte 2))
// (H) <- ((byte 3) (byte 2) + 1)
// The content of the memory location, whose address
// is specified in byte 2 and byte 3 of the instruction, is
// moved to register L. The content of the memory loca-
// tion at the succeeding address is moved to register H.
	reset(cpu);
	cpu->memory[0] = LHLD;
	cpu->memory[1] = 0x34; // low-order data (byte 2)
	cpu->memory[2] = 0x12; // high-order data (byte 3)
	cpu->memory[0x1234] = 0x42;
	cpu->memory[0x1235] = 0x73;
	Emulate8080Op(cpu);

	assert(cpu->l == 0x42);
	assert(cpu->h == 0x73);

	pass("LHLD passed\n");
}

void test_SHLD(State8080 *cpu)
{
// SHLD addr		(Store Hand L direct)
// ((byte 3) (byte 2)) <- (L)
// ((byte 3)(byte 2) + 1) <- (H)
// The content of register L is moved to the memory Jo-
// cation whose address is specified in byte 2 and byte
// 3. The content of register H is moved to the succeed-
// ing memory location.
	reset(cpu);
	cpu->l = 0x11;
	cpu->h = 0x22;
	cpu->memory[0] = SHLD;
	cpu->memory[1] = 0x34; // low-order data (byte 2)
	cpu->memory[2] = 0x12; // high-order data (byte 3)
	Emulate8080Op(cpu);

	assert(cpu->memory[0x1234] == 0x11);
	assert(cpu->memory[0x1235] == 0x22);

	passed("SHLD passed\n");
}

void test_LXI_RP(State8080 *cpu)
{
// LXI rp, data 16		(Load register pair immediate)
// (rh) <- (byte 3),
// (rl) <- (byte 2)
// Byte 3 of the instruction is moved into the high-order
// register (rh) of the register pair rp. Byte 2 of the in-
// struction is moved into the low-order register (rl) of
// the register pair rp.
	reset(cpu);
	// B
	cpu->memory[0] = LXI_B;
	cpu->memory[1] = 0x34; // low-order data (byte 2)
	cpu->memory[2] = 0x12; // high-order data (byte 3)
	Emulate8080Op(cpu);
	assert(cpu->b == 0x12);
	assert(cpu->c == 0x34);

	// D
	reset(cpu);
	cpu->memory[0] = LXI_D;
	cpu->memory[1] = 0x34; // low-order data (byte 2)
	cpu->memory[2] = 0x12; // high-order data (byte 3)
	Emulate8080Op(cpu);
	assert(cpu->d == 0x12);
	assert(cpu->e == 0x34);

	// H
	reset(cpu);
	cpu->memory[0] = LXI_H;
	cpu->memory[1] = 0x34; // low-order data (byte 2)
	cpu->memory[2] = 0x12; // high-order data (byte 3)
	Emulate8080Op(cpu);
	assert(cpu->h == 0x12);
	assert(cpu->l == 0x34);

	pass("LXI_RP passed\n");
}

void test_LDAX(State8080 *cpu)
{
// LOAX rp	(Load accumulator indirect)
// (A) <- ((rp))
// The content of the memory location, whose address
// is in the register pair rp, is moved to register A. Note:
// only register pairs rp=B (registers B and C·) or rp=D
// (registers D and E) may be specified.
	// B
	reset(cpu);
	cpu->b = 0x12;
	cpu->c = 0x34;
	cpu->memory[0x1234] = 0x69;
	cpu->memory[0] = LDAX_B;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x69);

	// D
	reset(cpu);
	cpu->d = 0x12;
	cpu->e = 0x34;
	cpu->memory[0x1234] = 0x69;
	cpu->memory[0] = LDAX_D;
	Emulate8080Op(cpu);
	assert(cpu->a == 0x69);

	pass("LDAX passed\n");
}

void test_INX_RP(State8080 *cpu)
{
// INX rp	(Increment register pair)
// (rh) (rl) <- (rh) (rl) + 1
// The content of the register pair rp is incremented by
// one. Note: No condition flags are affected.
	reset(cpu);
	cpu->b = 0;
	cpu->c = 0xFF;
	cpu->memory[0] = INX_B;
	Emulate8080Op(cpu);
	assert(RegisterPair(cpu, 'b') == 0x00FF+1);

	// @TODO: other registers

	// @TODO: separate test
	// INR B
	reset(cpu);
	cpu->b = 0xFF;
	cpu->cc.z = 0;
	cpu->memory[0] = INR_B;
	Emulate8080Op(cpu);
	assert(cpu->cc.z == 1);

	pass("INX_RP passed\n");
}

void test_RRC(State8080 *cpu)
{
// RCC	(Rotate right)
// (A[n]) <- (A[n-1]); (A[7]) <- (A[0])
// (CY) <- (A[0])
// The content of the accumulator is rotated right one
// position. The high order bit and the CY flag are both
// set to the value shifted out of the low order bit posi-
// tion. Only the CY flag is affected.
	reset(cpu);
	cpu->a = 0b01000001;
	cpu->memory[0] = RRC;
	Emulate8080Op(cpu);

	assert(cpu->a == 0b10100000);
	assert(cpu->cc.cy == 1);

	// arrange
	reset(cpu);
	cpu->a = 0b00000010;
	cpu->memory[0] = RRC;
	Emulate8080Op(cpu);

	// act
	assert(cpu->a == 0b00000001);
	assert(cpu->cc.cy == 0);

	pass("RCC passed\n");
}

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
	reset(cpu);
	cpu->memory[0] = JNZ;
	cpu->memory[1] = 0x33;
	cpu->memory[2] = 0x44;
	cpu->memory[0x4433] = JMP;
	cpu->memory[0x4434] = 0x55;
	cpu->memory[0x4435] = 0x66;
	Emulate8080Op(cpu);

	assert(cpu->pc == 0x4433);

	// JMP
	Emulate8080Op(cpu);
	assert(cpu->pc == 0x6655);

	cpu->memory[0x6655] = INR_A;
	Emulate8080Op(cpu);
	assert(cpu->a == 1);

	pass("JNZ passed\n");
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
	test_INX_RP(&State);
	test_LXI_RP(&State);
	test_LDAX(&State);
	test_SHLD(&State);
	test_LDA(&State);
	test_ADD(&State);
	test_LHLD(&State);
	test_DCR(&State);
	test_DAD_RP(&State);
	test_DCX_RP(&State);
	test_STAX_D(&State);
	test_ANA(&State);
	test_CALL(&State);
	test_JNZ(&State);
	test_RRC(&State);
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
