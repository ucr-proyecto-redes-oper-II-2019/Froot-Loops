#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//gcc restarteable.c -o restarteable

/*
 * Ejercicio reseteable
 * Integrantes:
 * -Antonio Alvarez
 * -Steven Barahona
 * -Daniel Barrantes
 * */

/*
 * Programa reseteable que cuenta enteros
 * Version 2.0: Ahora funciona despues de n reinicios en lugar de solo uno
 * */

static int flag = 0;

//Sighandler exclusivo para la interrupcion CTRL+C
void sig_handler(int signo)
{
  if (signo == SIGINT)
  {
	  printf("received SIGINT\n");
	  flag = 1;
  }  
}

int main(void)
{
	
	//Asignamos el sighandler
	if (signal(SIGINT, sig_handler) == SIG_ERR)
	{
		printf("\ncan't catch SIGINT\n");
	}
	
	//Se abre el archivo, si no existe se crea
	int cont = 0;
	int file;
	file = open("data", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	
	//Se leen los primeros 4 bytes (contador previo).
	int read_bytes = read(file, &cont, 4);
	printf("READ %d BYTES \n", read_bytes);
	
	//nos movemos al inicio del archivo con lseek, para siempre escribir los primeros 4 bytes
	lseek(file, 0, SEEK_SET);

	while(flag == 0)
	{
	  sleep(1);
	  printf("%d\n",cont);
	  ++cont;	  
	}

	//se escriben 4 bytes con el entero del contador mas reciente
	int writen_bytes = write(file,&cont,4);
	printf("WROTE %d BYTES\n",writen_bytes);
	
	close(file);
	
	
	return 0;
}
