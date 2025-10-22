#include <stdio.h>
#include <stdlib.h>

int16 *Stack;
int16 *StackStart;

void push(int16 Byte)
{
    *Stack++ = Byte;
}

int16 pop()
{
	if(Stack == StackStart)
	{
		return(*Stack);
	}

	return(*--Stack);
}

int main()
{
    // 4kb is 512 bytes. we mathin.

    // Traditionally, CHIP-8 had at least 16 two-byte entries.
    Stack = StackStart = (int16 *)malloc(16);

    int16 StackEntry = 0;
    char *mychar = (char *)&StackEntry;
    *mychar = 'A';
    *(mychar+1) = 'B';

    // C is such a great language.

    push(StackEntry);
    printf("pop:%c\n", pop());
    printf("pop:%c\n", pop());
    printf("pop:%c\n", *++mychar);

    printf("hello, world!\n");
    return 0;
}
