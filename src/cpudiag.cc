#include "8080.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	State8080 cpu = {};
	if(!InitCPU(&cpu))
	{
		fprintf(stderr, "Failed to init cpu\n");
		exit(1);
	}
	// @TODO: why isn't relative path working?
	const char *FileName = "/home/dwm/work/cpu-emulators/rom/cpudiag.bin";
	if (LoadROMFile(&cpu, FileName) != 1) 
	{
        exit(1);
	}

	printf("cpu diagnostic test\n");

	//Set first instruction to JMP 0x100
	cpu.memory[0] = 0xc3;
	cpu.memory[1] = 0;
	cpu.memory[2] = 0x01;

	// @TODO: what?
    //Fix the stack pointer from 0x6ad to 0x7ad    
    // this 0x06 byte 112 in the code, which is    
    // byte 112 + 0x100 = 368 in memory    
    cpu.memory[368] = 0x7;  
	
	// Skip DAA test
	cpu.memory[0x59c] = 0xc3; // JMP
	cpu.memory[0x59d] = 0xc2;
	cpu.memory[0x59e] = 0x05;

	while (true) 
	{
		u8 *opcode = (u8*)&cpu.memory[cpu.pc];
		printf("opcode: 0x%x\n", *opcode);
		if (*opcode == CALL)
		{
			if (5 == ((opcode[2] << 8) | opcode[1]))
			{
				u16 offset = Bytes(cpu.d, cpu.e);
				char *str = (char *)&cpu.memory[offset+3]; // skip the prefix bytes;
				while (*str != '$')
					printf("%c", *str++);
				printf("\n");
			}
			else if (cpu.c == 2)
			{
				printf("print char routine called\n");
			}
			// success?
			else if (0 == (opcode[2] << 8) | opcode[1])
			{
				printf("success?\n");
				exit(0);
			}
		}
		Emulate8080Op(&cpu);
		getchar();
	}
}
