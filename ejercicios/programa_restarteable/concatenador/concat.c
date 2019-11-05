#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#define FINAL_FILE_NAME "archivos_concatenados.txt"
#define FILE_BUFFER_SIZE 4096
#define FILE_NAME_SIZE 32

int main(int argc, char* argv[])//argv[1] = # de archivos a concatenar, los otros parámetros son dichos archivos
{
	FILE* final_file = fopen(FINAL_FILE_NAME, "wb");
	if (final_file == NULL)
	{
		printf("Error creando el archivo resultante\n");
		return EXIT_FAILURE;
	}
	
	//buffer usado para ir copiando pedacitos de 4KB de los archivos en el resultado
	char buffer[FILE_BUFFER_SIZE];
	
	//se obtiene la cantidad de archivos por concatenar
	int file_count = argc;
	
	//estructura usada para obtener el tamaño de cada archivo
	struct stat status;
  
	int number_of_files = atoi(argv[1]);
  
	//se escribe en el archivo concatenado el número de archivos que se van a concatenar
	fwrite(&number_of_files, sizeof(int), 1, final_file);
	
	int file_size = 0;
	
	for (int i = 2; i < file_count; i++)
	{
		//se obtiene el nombre del i-ésimo archivo
    char file_name[FILE_NAME_SIZE];
    bzero(file_name,FILE_NAME_SIZE);
    strcpy(file_name,argv[i]);
    
		//char* file_name = argv[i];
    
		//int len = strlen(argv[i]);
		
		//se obtiene el estado del archivo para obtener el tamaño
		stat(file_name, &status);
		file_size = status.st_size;
		
		FILE* read_file = fopen(file_name, "rb");
		if ( read_file == NULL )
		{
			printf("Error creando el archivo de lectura\n");
			return EXIT_FAILURE;
		}
		
		//se escribe el nombre del archivo en el archivo concatenado
		fwrite(&file_name, sizeof(char), FILE_NAME_SIZE, final_file);
    
		int bytes_read = 0;
		
		//se escribe el tamaño de cada archivo antes de escribir los datos
		fwrite(&file_size, sizeof(int), 1, final_file);
		
		while ((bytes_read = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, read_file)))//ciclo termina cuando se lean 0B
		{
			int bytes_written = fwrite(buffer, sizeof(char), bytes_read, final_file);
			
			if (!bytes_written)
			{
				printf("Error escribiendo en el archivo resultante\n");
				return EXIT_FAILURE;
			}
			
		}
		
		fclose(read_file);
	}
	
	fclose(final_file);
	
	return 0;
}
