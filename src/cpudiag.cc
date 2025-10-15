#include "8080.h"

int main(int argc, char **argv)
{
	State8080 CPU = {};
	if(!InitCPU(&CPU))
	{
		fprintf(stderr, "Failed to init CPU\n");
		exit(1);
	}
	// @TODO: why isn't relative path working?
	const char *FileName = "rom/cpudiag.bin";
	if (LoadROMFile(&CPU, FileName) != 1) 
	{
        exit(1);
	}
	printf("cpu diagnostic test\n");
	CPU.pc = 0x100;
	while (true) 
	{
		Emulate8080Op(&CPU);
	}
}
