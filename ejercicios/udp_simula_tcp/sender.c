/*
 * Grupo Froot Loops
 * Integrantes:
Daniel Barrantes
Antonio Alvarez
Steven Barahona

Programa encargado de enviar un jpg por paquetes de 516B
*/
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <omp.h>
#include "list.h"
#include "auxiliar_funtions.h"
#include "file_reader.h"
#include "packer.h"

int main(int argc, char* argv[]) //argv[1] = <my_port>, argv[2] = <destiny_ip>, argv[3] = <destiny_port> argv[4] = file_path
{
	//programación preventiva, se asegura que se hayan ingresado los parámetros necesarios
	if(argc < 4)//validacion de parametros
	{
	printf("No enough arguments given\n Usage: <my_port>, argv[2] = <destiny_ip>, argv[3] = <destiny_port> argv[4] = file_path\n");
	return EXIT_FAILURE;
	}

	list_t list;// la lista que servirá como ventana del emisor
	list_init(&list); // se inicializa sa lista
	char* data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos
	//int last_package_read = false; //bandera compartida que indica que el último paquete se leyó
	//int SN = 0;
	//int RN = 0;
	int all_data_read = false;
	//int wait_flag = false;
	//int give_up_flag = false;
	int block_is_free = true;

	//-----------------------------inicio de la sección paralela----------------------------------//

	#pragma omp parallel num_threads(4) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
	{
		int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
		//--------------------------------- inicio hilo 0 ------------------------------------------//
		/*
		el hilo 0 se encarga de traer los bloques de datos y ponerlos en el data_block para
		que el hilo 1 lo empaquete y lo añada a la ventana del emisor
		*/
		if (my_thread_n == 0)
		{
			
			file_reader(argv[4], data_block, &all_data_read, 
				&block_is_free);
				
			//printf("Soy hilo 0 termié mi tarea\n");

		}
		//------------------------------------fin del hilo 0 -----------------------------------------//
		if (my_thread_n == 1)
		{
			packer(data_block,&all_data_read,&block_is_free,&list);
			
			//printf("Soy hilo 1 termié mi tarea\n");
		}



		//----------------------------------fin de la sección paralela ------------------------------------//
	}
	free(data_block);
	destroy(&list);
	return 0;

}

//----------------funciones(subrutinas) utilizadas en el programa del emisor----------------------//







