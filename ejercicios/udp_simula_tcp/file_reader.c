#include "file_reader.h"

/*
este es el ciclo principal del hilo 0, es el que se encarga de leer toda la imagen,
la condición de salida es que ya no se pueda leer nada de la imagen
*/
void file_reader(char* file_name, char* data_block, int* all_data_read, 
	int* block_is_free)
{
	//abre el archivo y chequea que se abriera de  manera correcta
	FILE* file = fopen(file_name, "rb");
	if (file == NULL)
	{
		perror("Sender[0]: Error opening file: ");
	}
	
	//*all_data_read = false;//bandera que indica si toda la imagen fue leída
	
	while(!*all_data_read)
	{
		//hay que escribir siempre que data_block esté vacío, si no lo está, esperamos
		while( !*block_is_free)
			;
		//el acceso al data_block debe ser crítico, pero sabemos que está protegido por la 
		// bandera block_is_free	
		if(fread(data_block, PACK_THROUGHPUT, 1, file) > 0) //si se logran leer 512B, escriba en data_block
			*block_is_free = false;
		else //si ya no se puede leer del archivo, todos los datos fueron leídos
			*all_data_read = true;
			//*block_is_free = false;
		//usleep(200000); //se le da un chance al hilo 1 para que consuma el data_block
		//No hace falta también protegido por el candado
		//printf("He pasado una vez por acá\n");
	}
	
	//*last_package_read = true; //se prende la bandera compartida
	fclose(file); //ya se leyó todo el archivo en este punto
}
