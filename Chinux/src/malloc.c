/********************************** 
*
*  malloc.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*		ITBA 2011
*
***********************************/

int nextfree = 0x300000;

//Proceso Malloc
void * malloc (int size)
{	
	void * temp = (void*)nextfree;
	nextfree = nextfree + size;
	return temp;
}
