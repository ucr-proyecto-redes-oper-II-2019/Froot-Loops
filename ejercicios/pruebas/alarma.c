#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
//Declaramos los prototipos de funciones
void manejador(int signum);
//Variable global
int bandera = 1;
 
//FunciÃ³n principal
int main(int argc, char **argv)
{
 
	//Capturamos la señal SIGALRM
	signal(SIGALRM,manejador);

	//printf("EN 10 segundos se creara una alarma\n");
	//Crear alarma en segundos
	alarm(1);
	 
	//Mientras bandera sea 1, no finalizar el programa
	//while(bandera);
	
	for(int i = 0; i < 3; i++)
	{
		while(bandera)
			;
		
		bandera = 1;
		alarm(1);
	
	}

	while(bandera)
		;

	return 0;
 
}
 
//Funcion manejador
void manejador (int signum)
{
	printf("\nRecibi una alarma\n");
	//signal(SIGINT,SIG_DFL);

	bandera = 0;
}
