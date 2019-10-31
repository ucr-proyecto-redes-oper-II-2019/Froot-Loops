#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define FINAL_FILE_NAME "archivos_concatenados.txt"
#define FILE_BUFFER_SIZE 4096

int main(int argc, char* argv[])//por parámetro se ingresan los nombres de los archivos que se quieren concatenar
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
	
	int file_size = 0;
	
	for (int i = 1; i < file_count; i++)
	{
		//se obtiene el nombre del i-ésimo archivo
		char* file_name = argv[i];
		
		//se obtiene el estado del archivo para obtener el tamaño
		stat(file_name, &status);
		file_size = status.st_size;
		
		FILE* read_file = fopen(file_name, "rb");
		if ( read_file == NULL )
		{
			printf("Error creando el archivo de lectura\n");
			return EXIT_FAILURE;
		}
		
		int bytes_read = 0;
		
		//se escribe el tamaño de cada archivo antes de escribir los datos
		int size_written = fwrite(&file_size, sizeof(int), 1, final_file);
		
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
