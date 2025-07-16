#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "8080.h"

// @TODO: fix JMP and CALL, ugly and broken

static bool GlobalRunning;

void p(int a);
// bonus points for figuring out multiplication using bit shifts. :O
// doesn't work with numbers > 10, but 1-9 just fine.
int binmul(int a, int b)
{
    int max = max(a,b);
    int min = min(a,b);
    int maxb = max;
    int minb = min;
    if (a == 1 || b == 1)
	return max;
    while (min > 0) {
	// @TODO: handle wrapped bits for big numbers.
	if ((min >>= 1) > 0)
	    max <<= 1;
	if (min == 3)
	    max += maxb;
    }
    if (minb & 1)
	max += maxb;
    return max;
}

void p(int a)
{
    char *bin = ByteToBinary(a);
    printf("%s:%d\n", bin, a);
    free(bin);
}

void debug(int a, int b)
{
    p(a);
    p(b);
    p(a*b);
    int res = binmul(a, b);
    p(res);
    if (a*b == res)
	printf("PASS\n");
    else {
	printf("FAIL\n");
	printf("AHHHHHHHH");
	exit(1);
    }
}

int main(int argc, char **argv)
{
#if 0
    for (int i = 1; i < 10; i++)
	for (int j = 9; j > 0; j--)
	    debug(i,j);
    debug(25, 10);
    return 0;
#endif

    fprintf(stderr, "test\n");

    if (argc < 2) {
	fprintf(stderr, "pass a rom file dummy");
	exit(1);
    }

    int pc = 0;

    int maxops = 10000;
    if (argc > 2)
	maxops = ParseInt(argv[2]);

    State8080 *state = (State8080 *) calloc(1, sizeof(State8080));
    if(LoadROMFile(state, "W:\\chip-8\\rom\\invaders") == 0)
    {
	// :(
	exit(1);
    }

    GlobalRunning = true;

    // Run at least one op to match up with 8080js
    state->DebugPrint = 1;
    Emulate8080Op(state);

    char c = 0;
#define MAX_CHAR 256
    char CharBuffer[MAX_CHAR] = {};
    int BufferIndex = 0;
    while (GlobalRunning) {
	while ((c = getchar()) != '\n') {
	    if (c == 'q')
		goto end;
	    CharBuffer[BufferIndex++] = c;
	}
	printf("\b \b");
	char *s = CharBuffer;
	BufferIndex = 0;
	int n = 0;
	while (isdigit((c = *s++)))
	    n = (n * 10) + c-'0';
	if (n <= 0)
	    n = 1;
	// printf("advancing %d steps\n", n);
	// getchar();
	while (n-- > 0) {
	    if (n <= 100)
		state->DebugPrint = 1;
	    else
		state->DebugPrint = 0;
	    Emulate8080Op(state);
	}
	for (int i = 0; i < MAX_CHAR; i++)
	    CharBuffer[i] = 0;
    }

#if 0
    while (pc < fsize && pc < maxops) {
	int ops = Disassemble8080Op(buffer, pc);
	if (ops > 1)
	    maxops+=(ops-1);
	pc += ops;
    }
#endif

end:

    printf("the end\n");

    // @TODO: what are the heap corruption bugs?
    // free(buffer);
    // free(state);

    return 0;
}

