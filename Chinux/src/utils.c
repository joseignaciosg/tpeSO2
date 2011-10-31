/********************************** 
*
*  utils.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

/***	Project Includes	***/
#include "../include/defs.h"
#include "../include/utils.h"
#include "../include/stdio.h"

char string_number[MAX_NUM];

int
str_len(char *s) {
	int i;

	for (i = 0; s[i]; i++)
		;
	return i;
}


void
reverse(char s[]) {
	int i, j;
	char c;
	j = str_len(s)-1;
	for (i = 0; i < j ; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}

	return;
}

int
strcmp(char * s1, char * s2) {
	unsigned int i = 0;
	unsigned int j = 0;
	while (s1[i] != 0 && s2[j] != 0) {
		if (s1[i] != s2[j] || s1[i] == 0 || s2[j] == 0) {
			return FALSE;
		}
		i++;
		j++;
	}
	if (s1[i] == 0 && s2[j] == 0) {
		return TRUE;
	}
	return FALSE;
}


void
strcopy(char * copy , char * s, int size) {
	unsigned int i = 0;
	while (i < size) {
		copy[i] = s[i];
		i++;
	}
	copy[size] = 0;
	return;
}


int
itoa(int n) {
	int i = 0, sign = FALSE;

	if (n < 0){
		sign = TRUE; 	
		n = -n;
	}
	do { 							
		string_number[i++] = n % 10 + '0'; 		
		if(i-1 >= MAX_NUM){
			if (sign)
				string_number[i++] = '-';
			string_number[i] = 0;
			reverse(string_number);
			return printstr(string_number);
		}
	} while ((n /= 10) > 0); 		
	if (sign)
		string_number[i++] = '-';
	string_number[i] = 0;
	reverse(string_number);
	return printstr(string_number);
}



int
ltoa(unsigned long int n) {
	int i = 0, sign = FALSE;

	if (n < 0){
		sign = TRUE; 	
		n = -n;
	}
	do { 							
		string_number[i++] = n % 10 + '0'; 		
		if(i-1 >= MAX_NUM){
			if (sign)
				string_number[i++] = '-';
			string_number[i] = 0;
			reverse(string_number);
			return printstr(string_number);
		}
	} while ((n /= 10) > 0); 		
	if (sign)
		string_number[i++] = '-';
	string_number[i] = 0;
	reverse(string_number);
	return printstr(string_number);
}



int
ftoa(double n) {
	int i, j, MAX_DEC, sign;
	double aux;

	i = j = sign = FALSE;
	MAX_DEC = 6;

	if (n < 0){
		sign = TRUE;
		n = -n;
	}
	aux = n;

	do { 							
		string_number[i++] = (int)aux % 10 + '0';
		if(i >= MAX_NUM){				
			if (sign)				
				string_number[i++] = '-';
			string_number[i] = 0;
			reverse(string_number);
			return printstr(string_number);
		}
	} while ((int)(aux /= 10) > 0);

	aux = n - (int) n;
	if(aux == 0){					
		if (sign)
			string_number[i++] = '-';
		string_number[i] = 0;
		reverse(string_number);
		return printstr(string_number);
	}

	if(sign)					
		string_number[i++] = '-';
	string_number[i] = 0;
	reverse(string_number);
	string_number[i++] = '.';

	do{						
		aux *= 10;
		j++;
		string_number[i++] = (int)aux % 10 + '0';
		if(j == MAX_DEC){
			string_number[i] = 0;
			return printstr(string_number);
		}
	}while((aux -= (int)aux) > 0);
	string_number[i] = 0;

	return printstr(string_number);
}

char *
strcat(char *dest, const char *src)
{
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

