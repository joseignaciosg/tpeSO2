#include "defs.h"
#include "kc.h"
int contador=0;

//Proceso Idle
int Idle(int argc, char* argv[])
{
	
	
	Sti();
	while(1)
	{
	}
}	

//Proceso payaso que dibuja
int Payaso(int argc, char* argv[])
{
	char* video= (char*)0xb8000;
	Sti();

	while(1)
	{
		Cli();
		//debugBuenaOnda();
			message((char*)(contador%33),23,contador%140);
			contador++;
		Sti();
	}
}

//Proceso Mensaje bailarin
int DancingMessage(int argc,char* argv[])
{
	int i,j,k;
	char a=0;
	Cli();
	i=0;
	j=0;
	k=0;
	k_clear_screen();
	if(argc<2)
	{
		puterr("Insuficientes Parametros");
		return 1;
	}
	while(a!=68)
	{
		Cli();
			a=GetKey();
			for(i=0;i<argc;i++)
			{
				blank(strlen2(argv[i]),(j+(i/2)*10),(k+(i%2)*60));
			}
			if(j==0)
				if(k==0)
				{
					j++;
				}
				else
					k-=2;
			else if(j==8)
				if(k==40)
					j--;
				else
					k+=2;
			else if(k==0)
				j++;
			else
				j--;

			for(i=0;i<argc;i++)
			{
				message(argv[i],(j+(i/2)*10),(k+(i%2)*60));	
			}
				
		Sti();
		Sleep(5);
		int8();
	}

}
