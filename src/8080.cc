#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char *ByteToBinary(unsigned char num);
int Disassemble8080Op(unsigned char *buffer, int pc);
int ParseInt(char *str);

static int GlobalRemaining;

int main(int argc, char **argv)
{
    if (argc < 2) {
	fprintf(stderr, "pass a rom file dummy");
	exit(1);
    }
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
	fprintf(stderr, "failed to open file :(");
	exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    unsigned char *buffer = (unsigned char *) malloc(fsize);

    fread(buffer, fsize, 1, fp);
    fclose(fp);

    int pc = 0;

    int maxops = 1000;
    if (argc > 2)
	maxops = ParseInt(argv[2]);

    printf("addr|opcode\n");
    while (pc < fsize && pc < maxops) {
	int ops = Disassemble8080Op(buffer, pc);
	if (ops > 1)
	    maxops+=(ops-1);
	pc += ops;
    }

    printf("remaining: %d\n", GlobalRemaining);

    return 0;
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

char *ByteToBinary(unsigned char num)
{
    char *str = (char *) calloc(1, sizeof(char[8+1])); 

    for (int i = 7; i >= 0; i--) {
	str[i] = (num & 01) ? '1' : '0';
	num >>= 1;
    }

    return str;
}


void binprintf(unsigned x)
{
    printf("%s\n", ByteToBinary(x));
}

int Disassemble8080Op(unsigned char *codebuffer, int pc)
{
    unsigned char *code = &codebuffer[pc];
    int opbytes = 1;
    printf("%04x ", pc);
    printf("0x%02x ", *code);
    char *codebin = ByteToBinary(*code);
    printf("%sb ", codebin);
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
	case 0x0f: printf("RRC"); break;    
	case 0x0e: printf("MVI C, #$%02x", code[1]); opbytes = 2; break;    
	case 0x0d: printf("DCR C"); break;    
	case 0x11: printf("LXI D, #$%02x%02x", code[2], code[1]); opbytes = 3; break;    
	case 0x13: printf("INX D"); break;    
	case 0x16: printf("MVI D, #$%02x", code[1]); opbytes = 2; break;    
	case 0x19: printf("DAD D"); break;    
	case 0x1a: printf("LDAX D"); break;    
	case 0x1c: printf("INR E"); break;    
	case 0x1d: printf("DCR E"); break;    
	case 0x21: printf("LXI H, #$%02x%02x", code[2], code[1]); opbytes = 3; break;    
	case 0x22: printf("SHLD #$%02x%02x", code[2], code[1]); opbytes = 3; break;    
	case 0x23: printf("INX H"); break;    
	case 0x24: printf("INR H"); break;    
	case 0x25: printf("DCR H"); break;    
	case 0x26: printf("MVI H,#$%02x", code[1]); opbytes = 2; break;    
	case 0x27: printf("DAA"); break;    
	case 0x29: printf("DAD H"); break;    
	case 0x2a: printf("LHLD #$%02x%02x", code[2], code[1]); opbytes = 3; break;    
	case 0x2b: printf("DCX H"); break;    
	case 0x31: printf("LXI SP, #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0x32: printf("STA ($%02x%02x)", code[2], code[1]); opbytes = 3; break;
	case 0x35: printf("DCR M"); break;
	case 0x3a: printf("LDA ($%02x02x)", code[2], code[1]); opbytes = 3; break;
	case 0x3c: printf("INR A"); break;
	case 0x3d: printf("DCR A"); break;
	case 0x3e: printf("MVI %c, #$%02x", 'A', code[1]); opbytes = 2; break;
	case 0x4c: printf("MOV C,H"); break;
	case 0x46: printf("MOV B,M"); break;
	case 0x5f: printf("MOV E,A"); break;    
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
	case 0xc1: printf("POP B"); break;
	case 0xc2: printf("JNZ #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc3: printf("JMP #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc4: printf("CNZ #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc5: printf("PUSH B"); break; 
	case 0xc6: printf("ADI #$%02x", code[1]); opbytes = 2; break; 
	case 0xc9: printf("RET"); break; 
	case 0xca: printf("JZ #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xcd: printf("CALL #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xd1: printf("POP D"); break;
	case 0xd2: printf("JNC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xd3: printf("OUT #$%02x", code[1]); opbytes = 2; break;
	case 0xd4: printf("CNC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xd5: printf("PUSH D"); break;
	case 0xd6: printf("SUI #$%02x", code[1]); opbytes = 2; break;
	case 0xd7: printf("RST 2"); break;
	case 0xd8: printf("RC"); break;
	case 0xda: printf("JC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xdb: printf("IN #$%02x", code[1]); opbytes = 2; break;
	case 0xdc: printf("CC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xde: printf("SBI #$%02x", code[1]); opbytes = 2; break;
	case 0xe1: printf("POP H"); break;
	case 0xe5: printf("PUSH H"); break;
	case 0xe6: printf("ANI #$%02x", code[1]); opbytes = 2; break;
	case 0xeb: printf("XCHG"); break;
	case 0xf1: printf("POP PSW"); break;
	case 0xf5: printf("PUSH PSW"); break;
	case 0xfb: printf("EI"); break;
	case 0xfe: printf("CPI #$%02x", code[1]); opbytes = 2; break;

	default: printf("__TODO__ Opcode: 0x%02x %sb", *code, codebin); GlobalRemaining++; break;
    }


    printf("\n");

    free(codebin);
    codebin = NULL;

    return opbytes;
}

