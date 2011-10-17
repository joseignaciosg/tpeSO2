#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int i = 1000000000;
	char * c;
	while(i)
	{
		i = i-1;
		c = malloc(200);
		free(c);
	}

//printf("process running\n");
	
	printf("process ended\n");
	
	return 0;
}
