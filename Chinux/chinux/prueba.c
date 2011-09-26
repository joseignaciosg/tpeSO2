#include <stdio.h>

typedef struct{
	int a;
	int stack;
}proceso;

typedef struct{
	char * name;
	int numero;
}stk;

char vec[4][0x400];

int main(void)
{
	proceso proc;
	stk * stack;

	stack = (stk *) (&vec[0][0]);
	
	proc.stack = (int)(&stack);
	proc.a = (int)(&vec[1][0]);
	stack->name = "teta";
	stack->numero = 3;

	printf("%s %d\n", stack->name, stack->numero);		
	
	return 0;
}
