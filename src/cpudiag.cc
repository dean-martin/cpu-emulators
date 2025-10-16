#include "8080.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	State8080 CPU = {};
	if(!InitCPU(&CPU))
	{
		fprintf(stderr, "Failed to init CPU\n");
		exit(1);
	}
	// @TODO: why isn't relative path working?
	const char *FileName = "/home/dwm/work/cpu-emulators/rom/cpudiag.bin";
	if (LoadROMFile(&CPU, FileName) != 1) 
	{
        exit(1);
	}

	printf("cpu diagnostic test\n");

	//Set first instruction to JMP 0x100
	CPU.memory[0] = 0xc3;
	CPU.memory[1] = 0;
	CPU.memory[2] = 0x01;

	// @TODO: review
	// Fix stack pointer? what
	CPU.memory[368] = 0x7;
	
	// Skip DAA test
	CPU.memory[0x59c] = 0xc3; // JMP
	CPU.memory[0x59d] = 0xc2;
	CPU.memory[0x59e] = 0x05;

	while (true) 
	{
		u8 *opcode = (u8*)&CPU.memory[CPU.pc];
		printf("opcode: 0x%x\n", *opcode);
		Emulate8080Op(&CPU);
		// getchar();
	}
}
