#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <iostream>
 
//Declaramos los prototipos de funciones
void manejador(int signum);
//Variable global
int bandera = 1;
 
//FunciÃ³n principal
int main(int argc, char **argv)
{
 
	//Capturamos la señal SIGALRM
	signal(SIGALRM,manejador);

	#pragma omp parallel num_threads(2)
	{
		int my_id = omp_get_thread_num();
		
		
		if(my_id == 0)
		{
			//Crear alarma en segundos
			alarm(1);
			for(int i = 0; i < 3; i++)
			{
				while(bandera)
					;
				
				bandera = 1;
				alarm(1);
			
			}
		}
		else
		{
			for(int i = 0; i < 3; i++)
			{
				std::cout << "Say hi" << std::endl;
				sleep(2);
			}
				
		}
		
		
	}


	
	 
	//Mientras bandera sea 1, no finalizar el programa
	//while(bandera);
	
	

	return 0;
 
}
 
//Funcion manejador
void manejador (int signum)
{
	printf("\nRecibi una alarma\n");
	//signal(SIGINT,SIG_DFL);

	bandera = 0;
}
