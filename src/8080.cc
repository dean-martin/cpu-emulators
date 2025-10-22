#include "8080.h"

// @TODO: revise program counter advancement.

unsigned char cycles8080[] = {
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x00..0x0f
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x10..0x1f
	4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4, //etc
	4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,
	
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, //0x40..0x4f
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
	
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, //0x80..8x4f
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, //0xc0..0xcf
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, 
	11, 10, 10, 18, 17, 11, 7, 11, 11, 5, 10, 5, 17, 17, 7, 11, 
	11, 10, 10, 4, 17, 11, 7, 11, 11, 5, 10, 4, 17, 17, 7, 11, 
};

bool InitCPU(State8080 *CPU)
{
    // 16kb memory
    CPU->memory = (u8 *) calloc(1, 16*1024LL);
    return CPU->memory != NULL;
}

int LoadROMFile(State8080 *state, const char *FileName)
{
    FILE *fp = fopen(FileName, "rb");
    if (fp == NULL) {
		fprintf(stderr, "failed to open file `%s`\n%s\n", FileName, strerror(errno));
		return 0;
    }

    fseek(fp, 0L, SEEK_END);
    int FileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    fread(state->memory, FileSize, 1, fp);
    fclose(fp);

    return 1;
}

void GenerateInterrupt(State8080 *cpu, int interrupt_num)
{
	PUSH_PC(cpu);
	cpu->pc = 8 * interrupt_num;
	// simulate DI
	cpu->interrupt_enabled = 0;
}

// @TODO: this should be a function on the struct.
// Might as well embrace C++
static inline u16 RegisterPair(State8080 *cpu, char pair)
{
	if (pair == 'b')
		return Word16(cpu->b, cpu->c);
	if (pair == 'd')
		return Word16(cpu->d, cpu->e);
	if (pair == 'h')
		return Word16(cpu->h, cpu->l);
	// @TODO: panic!
	return -1;
}

static inline u16 Word16(u8 high, u8 low)
{
	bytes b;
	b.high = high;
	b.low = low;
	return b.data;
}

// Pointer to ((H) (L))
// H&L pair is considered "memory" in the data book.
static inline u8 *MemoryLocation(State8080 *state)
{
    return &state->memory[Word16(state->h, state->l)];
}


// @TODO: account for CPU Diagnostic runs
static inline void WriteMemory(State8080 *state, u16 Address, u8 Value)
{
	// if (Address < 0x2000) 
	// {
	// 	fprintf(stderr, "Writing to ROM memory not allowed %x\n", Address);
	// 	exit(1);
	// 	return;
	// }
	// if (Address >= 0x4000) 
	// {
	// 	fprintf(stderr, "Writing out to Space Invaders RAM not allowed %x\n", Address);
	// 	exit(1);
	// 	return;
	// }
	state->memory[Address] = Value;
}

static inline void
UnimplementedInstruction(State8080 *state)
{
    fprintf(stderr, "Error: unimplemented instruction\nopcode: 0x%02x\naddr: 0x%02x\n",
			state->memory[state->pc], state->pc);
    exit(1);
}

static inline u8
Parity(u8 ans)
{
    return (ans%2) == 0;
}

static inline void
CMP(State8080 *state, u8 word)
{
    u8 x = state->a - word;
    state->cc.z = (x == 0);
    state->cc.s = (0x80 == (x & 0x80));
    state->cc.p = Parity(x);
    state->cc.cy = (state->a < word);
}

static inline void
process_flags(State8080 *state, u16 answer)
{
    state->cc.z = ((answer & 0xff) == 0);
    state->cc.s = ((answer & 0x80) != 0); // 0x80 is the 7th bit.
    state->cc.cy = (0x80 == (answer & 0x80)); // i kinda like this better
    state->cc.p = Parity(answer&0xff);
}

// process condition flags, no carry (NC)
static inline void
process_flags_nc(State8080 *state, u16 answer)
{
    state->cc.z = ((answer & 0xff) == 0);
    state->cc.s = ((answer & 0x80) != 0); // 0x80 is the 7th bit.
    state->cc.p = Parity(answer&0xff);
}

static inline void
ADD(State8080 *state, u8 num)
{
    u16 answer = (u16) state->a + (u16) num;
    process_flags(state, answer);
    state->a = answer & 0xff;
}

static inline void
ADC(State8080 *state, u8 num)
{
    u16 answer = (u16) state->a + (u16) num + (u16) state->cc.cy;
    process_flags(state, answer);
    state->a = answer & 0xff;
}

static inline void
ACI(State8080 *state, u8 word)
{
    u16 answer = (u16) state->a + (u16) word + (u16) state->cc.cy;
    process_flags(state, answer);
    state->a = answer & 0xff;
}

static inline void
SUB(State8080 *state, u8 num)
{
    u16 answer = (u16) state->a - (u16) num;
    process_flags(state, answer);
    state->a = answer & 0xff;
}

static inline void
SBB(State8080 *state, u8 word)
{
    u16 answer = (u16) state->a - (u16) word - (u16) state->cc.cy;
    process_flags(state, answer);
    state->a = answer & 0xff;
}

static inline void _INR_M(State8080 *state)
{
    u16 offset = (state->h<<8) | state->l;
    state->memory[offset] += 1;
    process_flags_nc(state, state->memory[offset]);
}

static inline void _DCR_M(State8080 *state)
{
    u16 offset = (state->h<<8) | state->l;
    state->memory[offset] -= 1;
    process_flags_nc(state, state->memory[offset]);
}

static inline void INX_RP(u8 *rh, u8 *rl)
{
    u16 pair = (*rh << 8) | *rl;
    pair++;
    *rl = pair & 0xFF;
    *rh = (pair >> 8);
}

static inline void DCX_RP(u8 *rh, u8 *rl)
{
    u16 pair = (*rh << 8) | *rl;
    pair--;
    *rl = pair & 0xFF;
    *rh = (pair >> 8) & 0xFF;
}

static inline void DAD_RP(State8080 *state, u8 rh, u8 rl)
{
    u32 hl = (state->h << 8) | state->l;
    u32 bc = (rh << 8) | rl;
    hl += bc;
    state->cc.cy = (hl & 0xffff0000) != 0;
    state->l = (hl & 0xFF);
    state->h = hl >> 8;
}

static inline void _DAA(State8080 *state)
{
/*

    The eight-bit number in the accumulator is adjusted
    to form two four-bit Binary-Coded-Decimal digits by
    the following process:
	1. If the value of the least significant 4 bits of the
	accumulator is greater than 9 or if the AC flag
	is set, 6 is added to the accumulator.
	2. If the value of the most significant 4 bits of the
	accumulator is now greater than 9, or if the CY
	flag is set, 6 is added to the most significant 4
	bits of the accumulator.

    NOTE: All flags are affected
 */
    u8 nibble = (~0 >> 4);
    u8 lsb = (state->a & nibble);
    if (lsb > 9) {
		state->a += 6;
    }
    u8 msb = ((state->a >> 4) & nibble);
    if (msb > 9 || state->cc.cy) {
		msb += 6;
		state->a = (msb << 8) | (state->a & 0xFF);
    }

    process_flags(state, state->a);
}

static inline void
_CALL(State8080 *state)
{
    u8 *opcode = &state->memory[state->pc];
    u16 ret = state->pc+3;
	WriteMemory(state, state->sp-1, (ret >> 8));
	WriteMemory(state, state->sp-2, (ret & 0xFF));
    state->sp -= 2;
    state->pc = (opcode[2] << 8) | opcode[1];
	// @TODO: revise: this is for the ++ after switch statement
	state->pc--;
}

static inline void
_JMP(State8080 *state)
{
    u8 *opcode = &state->memory[state->pc];
    state->pc = (opcode[2] << 8) | opcode[1];
    // @TODO: revise
	assert(state->pc != 0);
    state->pc--;
}

static inline void
_RET(State8080 *state)
{
	u16 ReturnAddress = Word16(state->memory[state->sp+1], state->memory[state->sp]);
    state->pc = ReturnAddress;
    state->sp += 2;
	// @TODO: revise: this is for the ++ after switch statement
	state->pc--;
}

static inline void
RST(State8080 *state, u8 NNN)
{
    u16 ret = state->pc+2;
	WriteMemory(state, state->sp-1, (ret >> 8) & 0xFF);
	WriteMemory(state, state->sp-2, (ret & 0xFF));
    state->sp -= 2;
    state->pc = 8 * NNN;
}

static inline void
ANA(State8080 *state, u8 r)
{
    state->a = state->a & r;
    process_flags(state, state->a);
    state->cc.cy = 0;
}

static inline void
XRA(State8080 *state, u8 r)
{
    state->a = state->a ^ r;
    process_flags(state, state->a);
    state->cc.cy = 0;
    state->cc.ac = 0;
}

static inline void
XRI(State8080 *state)
{
    u8 *word = &state->memory[state->pc+1];
    state->a = state->a ^ *word;
    process_flags(state, state->a);
    state->cc.cy = 0;
    state->cc.ac = 0;
}

static inline void
ORA(State8080 *state, u8 r)
{
    state->a = state->a | r;
    process_flags(state, state->a);
    state->cc.cy = 0;
    state->cc.ac = 0;
}

static inline void
_RLC(State8080 *state)
{
    state->cc.cy = ((state->a >> 7) & 1);
    state->a = (state->a << 1) | ((state->a >> 7) & 1);
}

static inline void
_RAL(State8080 *state)
{
    u8 msb = (state->a >> 7) & 1;
    state->a = (state->a << 1) | state->cc.cy;
    state->cc.cy = msb;
}

static inline void
_RAR(State8080 *state)
{
    u8 x = state->a;
    state->a = (state->cc.cy << 7) | (x >> 1);
    state->cc.cy = ((x & 1) == 1);
}

static inline void
HLT(State8080 *state)
{
    // HALT! IN THE NAME OF THE LAW!
    printf("bye bye\n");
    exit(0);
}

static inline void
IN(State8080 *state)
{
    // @TODO: implement
    state->pc++;
}

static inline void
OUT(State8080 *state)
{
    // @TODO: implement
    state->pc++;
}

static inline void
POP_B(State8080 *state)
{
    state->c = state->memory[state->sp];
    state->b = state->memory[state->sp+1];
    state->sp += 2;
}

static inline void
POP_D(State8080 *state)
{
    state->e = state->memory[state->sp];
    state->d = state->memory[state->sp+1];
    state->sp += 2;
}

static inline void
POP_H(State8080 *state)
{
    state->l = state->memory[state->sp];
    state->h = state->memory[state->sp+1];
    state->sp += 2;
}

static inline void
PUSH_B(State8080 *state)
{
	WriteMemory(state, state->sp-1, state->b);
	WriteMemory(state, state->sp-2, state->c);
    state->sp -= 2;
}

static inline void
PUSH_D(State8080 *state)
{
	WriteMemory(state, state->sp-1, state->d);
	WriteMemory(state, state->sp-2, state->e);
    state->sp -= 2;
}

static inline void
PUSH_H(State8080 *state)
{
	WriteMemory(state, state->sp-1, state->h);
	WriteMemory(state, state->sp-2, state->l);
    state->sp -= 2;
}

static inline void
PUSH_PC(State8080 *state)
{
	WriteMemory(state, state->sp-1, (state->pc >> 8));
	WriteMemory(state, state->sp-2, (state->pc & 0xFF));
	state->sp -= 2;
}

inline void
SPHL(State8080 *state)
{
    u16 hl = (state->h << 8) | (state->l & 0xFF);
    state->sp = hl;
}

inline void
XTHL(State8080 *state)
{
    u8 l = state->l;
    u8 h = state->h;
    state->l = state->memory[state->sp];
    state->h = state->memory[state->sp+1];
	WriteMemory(state, state->sp, l);
	WriteMemory(state, state->sp+1, h);
}

inline void
POP_PSW(State8080 *state)
{
    state->a = state->memory[state->sp+1];
    u8 psw = state->memory[state->sp];
    // @TODO: review these, doesn't line up with databook.
    // e.g. (Z) <- ((SP))sub6
    // why is checking the first bit then?
    state->cc.z  = (0x01 == (psw & 0x01));
    state->cc.s  = (0x02 == (psw & 0x02));
    state->cc.p  = (0x04 == (psw & 0x04));
    state->cc.cy = (0x05 == (psw & 0x08));
    state->cc.ac = (0x10 == (psw & 0x10));
    state->sp += 2;
}

inline void
PUSH_PSW(State8080 *state)
{
	WriteMemory(state, state->sp-1, state->a);
    u8 psw = (state->cc.z |
		    state->cc.s << 1 |
		    state->cc.p << 2 |
		    state->cc.cy << 3 |
		    state->cc.ac << 4 );
	WriteMemory(state, state->sp-2, psw);
    state->sp = state->sp - 2;
}

inline void
_LXI_SP(State8080 *state)
{
    u8 byte2 = state->memory[state->pc+1];
    u8 byte3 = state->memory[state->pc+2];
    state->pc += 2;
    state->sp = (byte3 << 8) | byte2;
}

inline void
LXI_RP(State8080 *state, u8 *rp)
{
    // rp2
    *(rp+1) = state->memory[state->pc+1];  // byte2
    // rp1
    *rp = state->memory[state->pc+2];  // byte3
    state->pc += 2;
}

inline void
LDAX(State8080 *state, u8 *rp)
{
    u16 offset = (*rp << 8) | *(rp+1);
    state->a = state->memory[offset];
}

// Pointer to ((byte 3) (byte 2))
inline u8 *
GetAddressMemory(State8080 *state)
{
    u16 Address = (state->memory[state->pc+2] << 8) | (state->memory[state->pc+1]);
    u8 *Result = &state->memory[Address];
    return(Result);
}

inline void
_LDA(State8080 *state)
{
    state->a = *GetAddressMemory(state);
    state->pc += 2;
}

inline void
_STA(State8080 *state)
{
    u8 *MemoryAddress = GetAddressMemory(state);
    *MemoryAddress = state->a;
    state->pc += 2;
}

int Emulate8080Op(State8080 *state)
{
    state->Steps++;
    u8 *opcode = &state->memory[state->pc];

    if (state->debug) {
		DebugPrint(state);
		char c;
		while((c = getchar()) != '\n') {
			if (c == 'q')
				exit(0);
			if (c == 'd')
				state->debug = 0;
		}
    }

    switch(*opcode)
    {
	case NOP: break;   // NOP is easy!
	case 0x01: LXI_RP(state, &state->b); break;
	case 0x02:
	{
		u16 addr = (state->b << 8) | state->c;
		WriteMemory(state, addr, state->a);
	} break;
	case 0x03: INX_RP(&state->b, &state->c); break;	// INX B
	case 0x04: process_flags_nc(state, ++state->b); break;	// INR B
	case 0x05: process_flags_nc(state, --state->b); break;	// DCR B
	case 0x06: state->b = opcode[1]; state->pc++; break;	// MVI B,D8
	case 0x07: _RLC(state); break; // RLC
	// --
	case 0x09: DAD_RP(state, state->b, state->c); break;	// DAD B
	case 0x0A: LDAX(state, &state->b); break;   // LDAX B
	case 0x0B: DCX_RP(&state->b, &state->c); break;	// DCX B
	case 0x0C: process_flags_nc(state, ++state->c); break;	// INR C
	case 0x0D: process_flags_nc(state, --state->c); break;	// DCR C
	case 0x0E: state->c = opcode[1]; state->pc++; break;   // MVI C,D8
	case 0x0F:  // RRC
	{
	    u8 x = state->a;
	    state->a = ((x & 1) << 7) | (x >> 1);
	    state->cc.cy = (1 == (x&1));
	} break;
	case 0x11: LXI_RP(state, &state->d); break; // LXI D,D16
	case STAX_D:
	{
		u16 rp = Word16(state->d, state->e);
		state->memory[rp] = state->a;
	} break;
	case 0x13: INX_RP(&state->d, &state->e); break;	// INX D
	case 0x14: process_flags_nc(state, ++state->d); break;	// INR D
	case 0x15: process_flags_nc(state, --state->d); break;	// DCR D
	case 0x16: state->d = opcode[1]; state->pc++; break;	// MVI D,D8
	case 0x17: _RAL(state); break; // RAL
	// --
	case 0x19: DAD_RP(state, state->d, state->e); break;	// DAD D
	case 0x1A: LDAX(state, &state->d); break; // LDAX D
	case 0x1B: DCX_RP(&state->d, &state->e); break;	// DCX D
	case 0x1c: process_flags_nc(state, ++state->e); break;	// INR E
	case 0x1d: process_flags_nc(state, --state->e); break;	// DCR E
	case 0x1e: state->e = opcode[1]; state->pc++; break;	// MVI E,D8
	case 0x1F: _RAR(state); break; // RAR
	// --
	case 0x21: LXI_RP(state, &state->h); break;   // LXI H
	case 0x22:	// SHLD addr
	{
		u16 addr = (opcode[2] << 8) | opcode[1];
		state->memory[addr] = state->l;
		state->memory[addr+1] = state->h;
		state->pc+=2;
	} break;
	case 0x23: INX_RP(&state->h, &state->l); break;	// INX H
	case 0x24: process_flags_nc(state, ++state->h); break;	// INR H
	case 0x25: process_flags_nc(state, --state->h); break;	// DCR H
	case 0x26: state->h = opcode[1]; state->pc++; break;   //MVI H,D8
	case 0x27: _DAA(state); break;	// DAA

	case 0x29: DAD_RP(state, state->h, state->l); break;	// DAD H
	case 0x2a:	// LHLD addr
	{
		u16 addr = opcode[2] << 8 | opcode[1];
		state->l = state->memory[addr];
		state->h = state->memory[addr+1];
		state->pc += 2;
	} break;
	case 0x2B: DCX_RP(&state->h, &state->l); break;	// DCX H
	case 0x2c: process_flags_nc(state, ++state->l); break;	// INR L
	case 0x2d: process_flags_nc(state, --state->l); break;	// DCR L
	case 0x2E: state->l = opcode[1]; state->pc++; break;	// MVI L,D8
	case 0x2F:  // CMA (not)
	{
		state->a = ~state->a;
		// does not affect flags
	} break;

	case 0x31: _LXI_SP(state); break;
	case 0x32: _STA(state); break;	// STA addr
	case 0x33: ++state->sp; break;	// INX SP
	case 0x34: _INR_M(state); break;	// INR M
	case 0x35: _DCR_M(state); break;	// DCR L
	case 0x36: *MemoryLocation(state) = opcode[1]; state->pc++; break; // MVI M,D8
	case 0x37: state->cc.cy = 1; break; // STC
	// --
	case 0x39: DAD_RP(state, (state->sp >> 8), (state->sp & 0xFF)); break;	// DAD SP
	case 0x3A: _LDA(state); break;	// LDA addr
	case 0x3B: --state->sp; break;	// DCX SP
	case 0x3c: process_flags_nc(state, ++state->a);break;	// INR A
	case 0x3d: process_flags_nc(state, --state->a); break;	// DCR A
	case 0x3E: state->a = opcode[1]; state->pc++; break;  // MVI A,D8
	case 0x3F: // CMC
	{
	    state->cc.cy = (~state->cc.cy) & 1;
	} break;
	case 0x40: state->b = state->b; break;	// MOV B,B
	case 0x41: state->b = state->c; break;	// MOV B,C
	case 0x42: state->b = state->d; break;	// MOV B,D
	case 0x43: state->b = state->e; break;	// MOV B,E
	case 0x44: state->b = state->h; break;	// MOV B,H
	case 0x45: state->b = state->l; break;	// MOV B,L
	case 0x46: state->b = *MemoryLocation(state); break;	// MOV B,M
	case 0x47: state->b = state->a; break;	// MOV B,A
	case 0x48: state->c = state->b; break;	// MOV C,B
	case 0x49: state->c = state->c; break;	// MOV C,C
	case 0x4a: state->c = state->d; break;	// MOV C,D
	case 0x4b: state->c = state->e; break;	// MOV C,E
	case 0x4c: state->c = state->h; break;	// MOV C,H
	case 0x4d: state->c = state->l; break;	// MOV C,L
	case 0x4e: state->c = *MemoryLocation(state); break;	// MOV C,M
	case 0x4f: state->c = state->a; break;	// MOV C,A
	case 0x52: state->d = state->d; break;	// MOV D,D
	case 0x53: state->d = state->e; break;	// MOV D,E
	case 0x54: state->d = state->h; break;	// MOV D,H
	case 0x55: state->d = state->l; break;	// MOV D,L
	case 0x56: state->d = *MemoryLocation(state); break;	// MOV D,M
	case 0x57: state->d = state->a; break;	// MOV D,A
	case 0x58: state->e = state->b; break;	// MOV E,B
	case 0x59: state->e = state->c; break;	// MOV E,C
	case 0x5a: state->e = state->d; break;	// MOV E,D
	case 0x5b: state->e = state->e; break;	// MOV E,E
	case 0x5c: state->e = state->h; break;	// MOV E,H
	case 0x5d: state->e = state->l; break;	// MOV E,L
	case 0x5E: state->e = *MemoryLocation(state); break;	// MOV E,M
	case 0x5F: state->e = state->a; break;	// MOV E,A
	case 0x60: state->h = state->b; break;	// MOV H,B
	case 0x61: state->h = state->c; break;	// MOV H,C
	case 0x62: state->h = state->d; break;	// MOV H,D
	case 0x63: state->h = state->e; break;	// MOV H,E
	case 0x64: state->h = state->h; break;	// MOV H,H
	case 0x65: state->h = state->l; break;	// MOV H,L
	case 0x66: state->h = *MemoryLocation(state); break;	// MOV H,M
	case 0x67: state->h = state->a; break;	// MOV H,A
	case 0x68: state->l = state->b; break;	// MOV L,B
	case 0x69: state->l = state->c; break;	// MOV L,C
	case 0x6b: state->l = state->e; break;	// MOV L,E
	case 0x6c: state->l = state->h; break;	// MOV L,H
	case 0x6F: state->l = state->a; break;	// MOV L,A
	case 0x70: *MemoryLocation(state) = state->b; break;	// MOV M,B
	case 0x71: *MemoryLocation(state) = state->c; break;	// MOV M,C
	case 0x72: *MemoryLocation(state) = state->d; break;	// MOV M,D
	case 0x73: *MemoryLocation(state) = state->e; break;	// MOV M,E
	case 0x74: *MemoryLocation(state) = state->h; break;	// MOV M,H
	case 0x75: *MemoryLocation(state) = state->l; break;	// MOV M,L
	case 0x7C: state->a = state->h; break; // MOV A,H
	case 0x76: HLT(state); break; // HLT
	case 0x77: *MemoryLocation(state) = state->a; break; // MOV M,A
	case 0x78: state->a = state->b; break;	// MOV A,B
	case 0x79: state->a = state->c; break;	// MOV A,C
	case 0x7A: state->a = state->d; break;	// MOV A,D
	case 0x7B: state->a = state->e; break;	// MOV A,E
	case 0x7D: state->a = state->l; break;	// MOV A,L
	case 0x7E: state->a = *MemoryLocation(state); break;	// MOV A,M
	case 0x80: ADD(state, state->b); break; // ADD B
	case 0x81: ADD(state, state->c); break; // ADD C
	case 0x82: ADD(state, state->d); break; // ADD D
	case 0x83: ADD(state, state->e); break; // ADD E
	case 0x84: ADD(state, state->h); break; // ADD H
	case 0x85: ADD(state, state->l); break; // ADD L
	case 0x86: ADD(state, *MemoryLocation(state)); break; // ADD M
	case 0x87: ADD(state, state->a); break; // ADD A
	case 0x88: ADC(state, state->b); break; // ADC B
	case 0x89: ADC(state, state->c); break; // ADC 
	case 0x8a: ADC(state, state->d); break; // ADC D
	case 0x8b: ADC(state, state->e); break; // ADC E
	case 0x8c: ADC(state, state->h); break; // ADC H
	case 0x8d: ADC(state, state->l); break; // ADC L
	case 0x8e: ADC(state, *MemoryLocation(state)); break; // ADC M
	case 0x8f: ADC(state, state->a); break; // ADC A
	case 0x90: SUB(state, state->a); break; // SUB B
	case 0x91: SUB(state, state->c); break; // SUB C
	case 0x92: SUB(state, state->d); break; // SUB D
	case 0x93: SUB(state, state->e); break; // SUB E
	case 0x94: SUB(state, state->h); break; // SUB H
	case 0x95: SUB(state, state->l); break; // SUB L
	case 0x96: SUB(state, *MemoryLocation(state)); break; // SUB M
	case 0x97: SUB(state, state->a); break; // SUB A
	case 0x98: SBB(state, state->b); break; // SBB A
	case 0x99: SBB(state, state->c); break; // SBB C
	case 0x9a: SBB(state, state->d); break; // SBB D
	case 0x9b: SBB(state, state->e); break; // SBB E
	case 0x9c: SBB(state, state->h); break; // SBB H
	case 0x9d: SBB(state, state->l); break; // SBB L
	case 0x9e: SBB(state, *MemoryLocation(state)); break; // SBB M
	case 0x9f: SBB(state, state->a); break; // SBB A
	case 0xA0: ANA(state, state->b); break; // ANA B
	case 0xA1: ANA(state, state->c); break; // ANA C
	case 0xA2: ANA(state, state->d); break; // ANA D
	case 0xA3: ANA(state, state->e); break; // ANA E
	case 0xA4: ANA(state, state->h); break; // ANA H
	case 0xA5: ANA(state, state->l); break; // ANA L
	case 0xA6: ANA(state, *MemoryLocation(state)); break; // ANA M
	case 0xA7: ANA(state, state->a); break; // ANA A
	case 0xA8: XRA(state, state->b); break; // XRA B
	case 0xA9: XRA(state, state->c); break; // XRA C
	case 0xAA: XRA(state, state->d); break; // XRA D
	case 0xAB: XRA(state, state->e); break; // XRA E
	case 0xAC: XRA(state, state->h); break; // XRA H
	case 0xAD: XRA(state, state->l); break; // XRA L
	case 0xAE: XRA(state, *MemoryLocation(state)); break; // XRA M
	case 0xAF: XRA(state, state->a); break; // XRA A
	case 0xB0: ORA(state, state->b); break; //ORA B
	case 0xB1: ORA(state, state->c); break; //ORA C
	case 0xB2: ORA(state, state->d); break; //ORA D
	case 0xB3: ORA(state, state->e); break; //ORA E
	case 0xB4: ORA(state, state->h); break; //ORA H
	case 0xB5: ORA(state, state->l); break; //ORA L
	case 0xB6: ORA(state, *MemoryLocation(state)); break; //ORA M
	case 0xB7: ORA(state, state->a); break; //ORA A
	case 0xB8: CMP(state, state->b); break; // CMP B
	case 0xB9: CMP(state, state->c); break; // CMP C
	case 0xBA: CMP(state, state->d); break; // CMP D
	case 0xBB: CMP(state, state->e); break; // CMP E
	case 0xBC: CMP(state, state->h); break; // CMP H
	case 0xBD: CMP(state, state->l); break; // CMP L
	case 0xBE: CMP(state, *MemoryLocation(state)); break; // CMP M
	case 0xBF: CMP(state, state->a); break; // CMP A
	case 0xC0: // RNZ
	{
	    if (state->cc.z == 0)
			_RET(state);
	} break;
	case 0xC1: POP_B(state); break; // POP_B
	case 0xC2:  // JNZ addr
	{
	    if (state->cc.z == 0)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xC3:  // JMP addr
	{
	    _JMP(state);
	} break;
	case 0xC4:  // CNZ addr
	{
	    if (state->cc.z == 0)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xC5: PUSH_B(state); break; // PUSH B
	case 0xC7: RST(state, 0); break; // RST 0
	case 0xC8: // RZ
	{
	    if (state->cc.z)
			_RET(state);
	} break;
	case 0xC9: _RET(state); break; // RET
	case 0xCA: // JZ addr
	{
	    if (state->cc.z)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xCC:  // CZ addr
	{
	    if (state->cc.z)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xCD:  // _CALL addr
	{
	    _CALL(state);
	} break;
	case 0xC6: ADC(state, opcode[1]); state->pc++; break; // ADI word
	case 0xCE: ACI(state, opcode[1]); state->pc++; break; // ACI word
	case 0xCF: RST(state, 1); break; // RST 1
	case 0xD0:  // RNC
	{
	    if (state->cc.cy == 0)
			_RET(state);
	} break;
	case 0xD1:POP_D(state); break; // POP D
	case 0xD2:  // JNC addr
	{
	    if (state->cc.cy == 0)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xD3: OUT(state); break; // OUT byte
	case 0xD5: PUSH_D(state); break; // PUSH D
	case 0xD6: SUB(state, opcode[1]); state->pc++; break; // SUI word
	case 0xD7: RST(state, 2); break; // RST 2
	case 0xD8:  // RC
	{
	    if (state->cc.cy)
			_RET(state);
	} break;
	case 0xDA:  // JC addr
	{
	    if (state->cc.cy)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xDB: IN(state); break; // IN byte
	case 0xD4:  // CNC addr
	{
	    if (state->cc.cy == 0)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xDC:  // CC addr
	{
	    if (state->cc.cy)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xDE: SBB(state, opcode[1]); state->pc++; break; // SBI word
	case 0xDF: RST(state, 3); break; // RST 3
	// . . .
	case 0xE0: // RPO (Parity == 0 == odd)
	{
	    if (state->cc.p == 0)
			_RET(state);
	} break;
	case 0xE1: POP_H(state); break; // POP H
	case 0xE2:  // JPO addr (Parity == 0 == odd)
	{
	    if (state->cc.p == 0)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xE3: XTHL(state); break;	// XTHL
	case 0xE4:  // CPO addr
	{
	    if (state->cc.p == 0)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xE5: PUSH_H(state); break; // PUSH H
	case 0xE6:  // ANI  byte
	{
	    u8 x = state->a & opcode[1];
	    process_flags(state, x);
		// @TODO: wtf? there's an AC flag?
	    state->cc.cy = 0; // data book says ANI clears CY
	    state->a = x;
	    state->pc++; // for the data byte
	} break;
	case 0xE7: RST(state, 4); break; // RST 4
	case 0xE8: // RPE
	{
	    if (state->cc.p)
			_RET(state);
	} break;
	case 0xE9: // PCHL
	{
		state->pc = (state->h << 8) | state->l;

	    // @TODO: why am i returning here?
	    return 0;
	} break;
	case 0xEA:  // JPE addr (Parity == 1 == even)
	{
	    if (state->cc.p)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xEB: //XCHG
	{
	    u8 h = state->h;
	    u8 l = state->l;
	    state->h = state->d;
	    state->l = state->e;
	    state->d = h;
	    state->e = l;
	} break;
	case 0xEC:  // CPE addr
	{
	    if (state->cc.p)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xEE: XRI(state); state->pc++; break; // XRI byte
	case 0xEF: RST(state, 5); break; // RST 5
	case 0xF0:  // RP
	{
	    if (state->cc.s == 0)
			_RET(state);
	} break;
	case 0xF1: POP_PSW(state); break; // POP PSW
	case 0xF2:  // JP addr
	{
	    if (state->cc.s == 0)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xF3:  state->interrupt_enabled = 0; break; // DI
	case 0xF4:  // CP addr
	{
	    if (state->cc.s == 0)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
	case 0xF5: PUSH_PSW(state); break; // PUSH PSW
	case 0xF6: ORA(state, state->memory[state->pc+1]); state->pc++; break; // ORI data
	case 0xF7: RST(state, 6); break; // RST 6
	case 0xF8:  // RM
	{
	    if (state->cc.s)
			_RET(state);
	} break;
	case 0xF9: SPHL(state); break; // SPHL
	case 0xFA:  // JM addr
	{
	    if (state->cc.s)
			_JMP(state);
	    else
			state->pc += 2;
	} break;
	case 0xFB:  state->interrupt_enabled = 1; break; // EI
	case 0xFC:  // CM addr
	{
	    if (state->cc.s)
			_CALL(state);
	    else
			state->pc += 2;
	} break;
        case 0xFE:  // CPI byte
	{
	    CMP(state, state->memory[state->pc+1]);
	    state->pc++;
	} break;
	case 0xFF: RST(state, 7); break; // RST 7
	default: UnimplementedInstruction(state); break;
    }

	// @TODO: move to top, refactor all functions >:(
    state->pc += 1;

    return cycles8080[*opcode];
}

void DebugPrint(State8080 *cpu)
{
	Disassemble8080Op(cpu->memory, cpu->pc);
	printf("\tStep: %lld\n", cpu->Steps);
	printf("\tC=%d,P=%d,S=%d,Z=%d\n", cpu->cc.cy, cpu->cc.p,
		cpu->cc.s, cpu->cc.z);
	printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP 0x%04x\n",
		cpu->a, cpu->b, cpu->c, cpu->d,
		cpu->e, cpu->h, cpu->l, cpu->sp);
	u16 spm = cpu->memory[cpu->sp] << 8 | cpu->memory[cpu->sp-1];
	printf("(SP): 0x%04x\n", spm);
}

int ParseInt(char *str)
{
    int n = 0;
    char c;
    while ((c = *str++))
	if (c >= '0' || c <= '9')
	    n = (n*10) + c-'0';

    return n;
}

char *ByteToBinary(unsigned char num, char *output)
{
    for (int i = 7; i >= 0; i--) {
		output[i] = (num & 01) ? '1' : '0';
		num >>= 1;
    }
    output[8] = '\0';

    return output;
}

int Disassemble8080Op(unsigned char *codebuffer, int pc)
{
    unsigned char *code = &codebuffer[pc];
    int opbytes = 1;
    printf("PC: 0x%04x ", pc);
	char codebin[9];
	ByteToBinary(*code, codebin);
    switch (*code)
    {
	case 0x00: printf("NOP"); break;
	case 0x01: printf("LXI B,#$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0x02: printf("STAX   B"); break;
	case 0x03: printf("INX    B"); break;
	case 0x04: printf("INR    B"); break;
	case 0x05: printf("DCR    B"); break;
	case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes=2; break;
	case 0x07: printf("RLC"); break;
	case 0x08: printf("NOP"); break;
	case 0x09: printf("DAD B"); break;
	case 0x0a: printf("LDAX B"); break;
	case 0x0b: printf("DCX B"); break;
	case 0x0c: printf("INR C"); break;
	case 0x0d: printf("DCR C"); break;
	case 0x0e: printf("MVI C,#$%02x", code[1]); opbytes=2; break;
	case 0x0f: printf("RRC"); break;
	case 0x11: printf("LXI D,#$%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0x12: printf("STAX D"); break;
	case 0x13: printf("INX D"); break;
	case 0x14: printf("INR D"); break;
	case 0x15: printf("DCR D"); break;
	case 0x16: printf("MVI D,#$%02x", code[1]); opbytes=2; break;
	case 0x17: printf("RAL"); break;
	case 0x19: printf("DAD D"); break;
	case 0x1a: printf("LDAX D"); break;
	case 0x1b: printf("DCX D"); break;
	case 0x1c: printf("INR E"); break;
	case 0x1d: printf("DCR E"); break;
	case 0x1e: printf("MVI E,#$%02x", code[1]); opbytes=2; break;
	case 0x1f: printf("RAR"); break;
	case 0x21: printf("LXI H,#$%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0x22: printf("SHLD $%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0x23: printf("INX H"); break;
	case 0x24: printf("INR H"); break;
	case 0x25: printf("DCR H"); break;
	case 0x26: printf("MVI H,#$%02x", code[1]); opbytes=1; break;
	case 0x27: printf("DAA"); break;
	case 0x29: printf("DAD H"); break;
	case 0x2a: printf("LHLD #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0x2b: printf("DCX H"); break;
	case 0x2c: printf("INR L"); break;
	case 0x2d: printf("DCR L"); break;
	case 0x2e: printf("MVI L,0x%02x", code[1]); opbytes=2; break;
	case 0x2f: printf("CMA"); break;
	case 0x31: printf("LXI SP, #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0x32: printf("STA ($%02x%02x)", code[2], code[1]); opbytes = 3; break;
	case 0x33: printf("INX SP"); break;
	case 0x34: printf("INR M"); break;
	case 0x35: printf("DCR M"); break;
	case 0x36: printf("MVI M,#$%02x", code[1]); opbytes=2; break;
	case 0x37: printf("STC"); break;
	case 0x3a: printf("LDA $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0x3c: printf("INR A"); break;
	case 0x3d: printf("DCR A"); break;
	case 0x3e: printf("MVI %c, #$%02x", 'A', code[1]); opbytes = 2; break;
	case 0x46: printf("MOV B,M"); break;
	case 0x47: printf("MOV B,A"); break;
	case 0x48: printf("MOV C,B"); break;
	case 0x49: printf("MOV C,C"); break;
	case 0x4a: printf("MOV C,D"); break;
	case 0x4b: printf("MOV C,E"); break;
	case 0x4c: printf("MOV C,H"); break;
	case 0x4d: printf("MOV C,L"); break;
	case 0x4e: printf("MOV C,M"); break;
	case 0x4f: printf("MOV C,A"); break;
	case 0x50: printf("MOV D,B"); break;
	case 0x51: printf("MOV D,C"); break;
	case 0x52: printf("MOV D,D"); break;
	case 0x53: printf("MOV D,E"); break;
	case 0x54: printf("MOV D,H"); break;
	case 0x55: printf("MOV D,L"); break;
	case 0x56: printf("MOV D,M"); break;
	case 0x57: printf("MOV D,A"); break;
	case 0x58: printf("MOV E,B"); break;
	case 0x59: printf("MOV E,C"); break;
	case 0x5a: printf("MOV E,D"); break;
	case 0x5b: printf("MOV E,E"); break;
	case 0x5c: printf("MOV E,H"); break;
	case 0x5d: printf("MOV E,L"); break;
	case 0x5e: printf("MOV E,M"); break;
	case 0x5f: printf("MOV E,A"); break;
	case 0x60: printf("MOV H,B"); break;
	case 0x61: printf("MOV H,C"); break;
	case 0x62: printf("MOV H,D"); break;
	case 0x63: printf("MOV H,E"); break;
	case 0x64: printf("MOV H,H"); break;
	case 0x65: printf("MOV H,L"); break;
	case 0x66: printf("MOV H,M"); break;
	case 0x67: printf("MOV H,A"); break;
	case 0x68: printf("MOV L,B"); break;
	case 0x69: printf("MOV L,C"); break;
	case 0x6a: printf("MOV L,D"); break;
	case 0x6b: printf("MOV L,E"); break;
	case 0x6c: printf("MOV L,H"); break;
	case 0x6d: printf("MOV L,L"); break;
	case 0x6e: printf("MOV L,M"); break;
	case 0x6f: printf("MOV L,A"); break;
	case 0x70: printf("MOV M,B"); break;
	case 0x71: printf("MOV M,C"); break;
	case 0x72: printf("MOV M,D"); break;
	case 0x73: printf("MOV M,E"); break;
	case 0x74: printf("MOV M,H"); break;
	case 0x75: printf("MOV M,L"); break;
	case 0x76: printf("HLT"); break;
	case 0x77: printf("MOV M,A"); break;
	case 0x78: printf("MOV A,B"); break;
	case 0x79: printf("MOV A,C"); break;
	case 0x7a: printf("MOV A,D"); break;
	case 0x7b: printf("MOV A,E"); break;
	case 0x7c: printf("MOV A,H"); break;
	case 0x7d: printf("MOV A,L"); break;
	case 0x7e: printf("MOV A,M"); break;
	case 0x7f: printf("MOV A,A"); break;
	case 0x80: printf("ADD B"); break;
	case 0x81: printf("ADD C"); break;
	case 0x82: printf("ADD D"); break;
	case 0x83: printf("ADD E"); break;
	case 0x84: printf("ADD H"); break;
	case 0x85: printf("ADD L"); break;
	case 0x86: printf("ADD M"); break;
	case 0x87: printf("ADD A"); break;
	case 0x88: printf("ADC B"); break;
	case 0x89: printf("ADC C"); break;
	case 0x8a: printf("ADC D"); break;
	case 0x8b: printf("ADC E"); break;
	case 0x8c: printf("ADC H"); break;
	case 0x8d: printf("ADC L"); break;
	case 0x8e: printf("ADC M"); break;
	case 0x8f: printf("ADC A"); break;
	case 0x90: printf("SUB B"); break;
	case 0x91: printf("SUB C"); break;
	case 0x92: printf("SUB D"); break;
	case 0x93: printf("SUB E"); break;
	case 0x94: printf("SUB H"); break;
	case 0x95: printf("SUB L"); break;
	case 0x96: printf("SUB M"); break;
	case 0x97: printf("SUB A"); break;
	case 0x98: printf("SBB B"); break;
	case 0x99: printf("SBB C"); break;
	case 0x9a: printf("SBB D"); break;
	case 0x9b: printf("SBB E"); break;
	case 0x9c: printf("SBB H"); break;
	case 0x9d: printf("SBB L"); break;
	case 0x9e: printf("SBB M"); break;
	case 0x9f: printf("SBB A"); break;
	case 0xa0: printf("ANA B"); break;
	case 0xa1: printf("ANA C"); break;
	case 0xa2: printf("ANA D"); break;
	case 0xa3: printf("ANA E"); break;
	case 0xa4: printf("ANA H"); break;
	case 0xa5: printf("ANA L"); break;
	case 0xa6: printf("ANA M"); break;
	case 0xa7: printf("ANA A"); break;
	case 0xa8: printf("XRA B"); break;
	case 0xa9: printf("XRA C"); break;
	case 0xaa: printf("XRA D"); break;
	case 0xab: printf("XRA E"); break;
	case 0xac: printf("XRA H"); break;
	case 0xad: printf("XRA L"); break;
	case 0xae: printf("XRA M"); break;
	case 0xaf: printf("XRA A"); break;
	case 0xb0: printf("ORA B"); break;
	case 0xb1: printf("ORA C"); break;
	case 0xb2: printf("ORA D"); break;
	case 0xb3: printf("ORA E"); break;
	case 0xb4: printf("ORA H"); break;
	case 0xb5: printf("ORA L"); break;
	case 0xb6: printf("ORA M"); break;
	case 0xb7: printf("ORA A"); break;
	case 0xb8: printf("CMP B"); break;
	case 0xb9: printf("CMP C"); break;
	case 0xba: printf("CMP D"); break;
	case 0xbb: printf("CMP E"); break;
	case 0xbc: printf("CMP H"); break;
	case 0xbd: printf("CMP L"); break;
	case 0xbe: printf("CMP M"); break;
	case 0xbf: printf("CMP A"); break;
	case 0xc0: printf("RNZ"); break;
	case 0xc1: printf("POP B"); break;
	case 0xc2: printf("JNZ $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc3: printf("JMP $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc4: printf("CNZ $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc5: printf("PUSH B"); break;
	case 0xc6: printf("ADI #$%02x", code[1]); opbytes = 2; break;
	case 0xc7: printf("RST 0"); break;
	case 0xc8: printf("RZ"); break;
	case 0xc9: printf("RET"); break;
	case 0xca: printf("JZ #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xcc: printf("CZ $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xcd: printf("CALL $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xce: printf("ACI #0x%02x", code[1]); opbytes=2; break;
	case 0xcf: printf("RST 1"); break;
	case 0xd0: printf("RNC"); break;
	case 0xd1: printf("POP D"); break;
	case 0xd2: printf("JNC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xd3: printf("OUT #$%02x", code[1]); opbytes = 2; break;
	case 0xd4: printf("CNC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xd5: printf("PUSH D"); break;
	case 0xd6: printf("SUI #$%02x", code[1]); opbytes = 2; break;
	case 0xd7: printf("RST 2"); break;
	case 0xd8: printf("RC"); break;
	case 0xda: printf("JC $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xdb: printf("IN #0x%02x", code[1]); opbytes = 2; break;
	case 0xdc: printf("CC $%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xde: printf("SBI #$%02x", code[1]); opbytes = 2; break;
	case 0xe0: printf("RPO"); break;
	case 0xe1: printf("POP H"); break;
	case 0xe2: printf("JPO $%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xe3: printf("XTHL"); break;
	case 0xe4: printf("CPO $%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xe5: printf("PUSH H"); break;
	case 0xe6: printf("ANI #$%02x", code[1]); opbytes = 2; break;
	case 0xe7: printf("RST 4"); break;
	case 0xe8: printf("RPE"); break;
	case 0xe9: printf("PCHL"); break;
	case 0xea: printf("JPE $%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xeb: printf("XCHG"); break;
	case 0xec: printf("CPE $%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xee: printf("XRI #0x%02x", code[1]); opbytes=2; break;
	case 0xef: printf("RST 5"); break;
	case 0xf0: printf("RP"); break;
	case 0xf1: printf("POP PSW"); break;
	case 0xf2: printf("JP #$%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xf3: printf("DI"); break;
	case 0xf4: printf("CP #$%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xf5: printf("PUSH PSW"); break;
	case 0xf6: printf("ORI #$%02x", code[1]); opbytes=2; break;
	case 0xf7: printf("RST 6"); break;
	case 0xf8: printf("RM"); break;
	case 0xf9: printf("SPHL"); break;
	case 0xfa: printf("JM $%02x%02x", code[2], code[1]); opbytes=3; break;
	case 0xfb: printf("EI"); break;
	case 0xfe: printf("CPI #0x%02x", code[1]); opbytes = 2; break;

	default: printf("__TODO__ Opcode: 0x%02x %sb", *code, codebin);  break;
    }

    printf("\n");

    return opbytes;
}
