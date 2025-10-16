#include "8080.h"
#include "common.h"
#include <stdio.h>

global_variable State8080 State;

void test_STAX_D(State8080 *cpu)
{
	cpu->a = 25;
	cpu->d = 0x34;
	cpu->e = 0x12;
	cpu->pc = 0;
	cpu->memory[0] = STAX_D;
	Emulate8080Op(cpu);
	assert(cpu->memory[0x1234] == 25);
}

void fail(const char *msg)
{
	fprintf(stderr, msg);
	exit(1);
}

int main(int argc, char **argv)
{
	if(!InitCPU(&State))
	{
		fail("failed to init CPU\n");
	}
	test_STAX_D(&State);
	printf("henlo wurld\n");
}

