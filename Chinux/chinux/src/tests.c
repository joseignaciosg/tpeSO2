/********************************** 
*
*  tests.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*	Reznik, Luciana
*		ITBA 2011
*
***********************************/

/***	Proyect Includes	***/
#include "../include/defs.h"
#include "../include/stdio.h"
#include "../include/tests.h"


extern int keypressed;
extern int enterpressed;
char sTest[BUFFER_SIZE+1];

void
printfTest(){
	char * s = "Hola mundo";
	double dval = 0.332;
	double dval2 = 5776;
	int ival = 23431;
	printf("This prints a string: %s\n" , s);
	printf("This prints two strings: %s - %s , and they are equal\n", s, s);
	printf("This prints an integer: %d\n", ival);
	printf("This prints two integers: %d - %d , "
			"and the second one is the double\n", ival, ival*2);
	printf("This prints a negative integer : %d\n", -ival);
	printf("This prints a double: %f\n", dval);
	printf("This prints a negative double: %f\n", -dval);
	printf("This prints an integer as a double: %f\n", dval2);
	printf("This prints a negative integer as a double: %f", -dval2);
	
	return;
}


void
scanStringTest(){

	int i=0;
	scanf("%s",sTest);
	printf("Your typed string is: %s\n",sTest);
	for ( ; i<BUFFER_SIZE;i++){
		sTest[i] = 0;
	}

	return;
}


void
scanIntTest(){
	int* iTest;
	scanf("%d",iTest);
	printf("Your typed integer is: %d\n",*iTest);

	return;
}


void
scanDoubleTest(){
	double* dTest;
	scanf("%f",dTest);
	printf("Your typed double is: %f\n",*dTest);

	return;
}

