#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char *ByteToBinary(unsigned char num);
int Disassemble8080Op(unsigned char *buffer, int pc);
int ParseInt(char *str);

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
    char *str = (char *) malloc(8+1); 
    str[8] = '\0';

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
    printf("%sb ", ByteToBinary(*code));
    switch (*code)
    {
	case 0x00: printf("NOP"); break;
	case 0x01: printf("LXI\tB,#$%02x%02x", code[2], code[1]); opbytes = 3; break;
        case 0x02: printf("STAX   B"); break;    
        case 0x03: printf("INX    B"); break;    
        case 0x04: printf("INR    B"); break;    
        case 0x05: printf("DCR    B"); break;    
        case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes=2; break;    
        case 0x07: printf("RLC"); break;    
        case 0x08: printf("NOP"); break;    
        case 0x0f: printf("RRC"); break;    
        case 0x21: printf("LXI H, #$%02x%02x", code[2], code[1]); opbytes = 3; break;    
	case 0x32: printf("STA ($%02x%02x)", code[2], code[1]); opbytes = 3; break;
	case 0x35: printf("DCR M"); break;
	case 0x3a: printf("LDA ($%02x02x)", code[2], code[1]); opbytes = 3; break;
	case 0x3e: printf("MVI %c, #$%02x", 'A', code[1]); opbytes = 2; break;
	case 0xa7: printf("ANA A"); break;
	case 0xc3: printf("JMP #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xc5: printf("PUSH B"); break; 
	case 0xc6: printf("ADI #$%02x", code[1]); opbytes = 2; break; 
	case 0xca: printf("JZ #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xcd: printf("CALL #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xd5: printf("PUSH D"); break;
	case 0xda: printf("JC #$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0xdb: printf("IN #$%02x", code[1]); opbytes = 2; break;
	case 0xe5: printf("PUSH H"); break;
	case 0xf5: printf("PUSH PSW"); break;
	case 0xfe: printf("CPI #$%02x", code[1]); opbytes = 2; break;

	default: printf("__TODO__ Opcode: 0x%02x %sb", *code, ByteToBinary(*code)); break;
    }


    printf("\n");

    return opbytes;
}

