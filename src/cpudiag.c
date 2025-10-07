#include "8080.h"

int main(int argc, char **argv)
{
	State8080 CPU = {};
	if (!InitCPU(&CPU) && LoadROMFile(&CPU, "roms/cpudiag.bin") == 0) {
		fprintf(stderr, "Failed to init CPU or load cpudiag.bin ROM");
        exit(1);
	}
	printf("hello world!\n");
	CPU.pc = 0x100;
	while (true) 
	{
		Emulate8080Op(&CPU);
	}
}
