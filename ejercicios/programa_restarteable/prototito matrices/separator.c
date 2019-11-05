#include <stdlib.h>
#include <stdio.h>

#define FILE_BUFFER_SIZE 4096

int min( int x, int y );

int main(int argc, char* argv[])//argv[1] debe ser el nombre del archivo que se quiere separar, y los demás parámetros serán los nombres deseados de los archivos resultado
{
	char* source_file_name = argv[1];
	FILE* source_file = fopen(source_file_name, "rb");
	if (source_file == NULL)
	{
		printf("Error abriendo el archivo fuente\n");
		return EXIT_FAILURE;
	}
	
	//buffer usado para ir copiando los datos del archivo fuente a los archivos resultantes
	char buffer[FILE_BUFFER_SIZE];
	
	int file_count = argc;
	
	for (int i = 2; i < file_count; i++)
	{
		//se obtiene el nombre del i-ésimo archivo y se crea para escribirle los datos correspondientes
		char* file_name = argv[i];
		FILE* write_file = fopen(file_name, "wb");
		if (write_file == NULL)
		{
			printf("Error abriendo uno de los archivos resultantes\n");
			return EXIT_FAILURE;
		}
		
		//se guarda el tamaño de bytes que mide el i-ésimo archivo a separar
		int int_buffer = 0; 
		int size_read = fread(&int_buffer, sizeof(int), 1, source_file);
		if (!size_read)
		{
			printf("Error leyendo el tamaño del archivo a separar\n");
			return EXIT_FAILURE;
		}
		printf("El tamaño es de %d\n", int_buffer);
		/*-------------------------------------------------------------------------------*/
		
		int bytes_read = 0;
		int byte_count = 0;
		
		while (int_buffer > 0)
		{
			bytes_read = fread(buffer, sizeof(char), min(int_buffer,FILE_BUFFER_SIZE), source_file);
			if (!bytes_read)
			{
				printf("Error leyendo los datos del archivo a separar\n");
				return EXIT_FAILURE;
			}
			
			int_buffer = int_buffer - bytes_read;
			
			byte_count = byte_count + bytes_read;
			printf("van leídos %d bytes\n", byte_count);
			
			int bytes_written = fwrite(buffer, sizeof(char), bytes_read, write_file);
			if (!bytes_written)
			{
				printf("Error escribiendo los datos en el archivo separado\n");
				return EXIT_FAILURE;
			}
		}
		/*--------------------------------------------------------------------------------*/
		fclose(write_file);
	}
	fclose(source_file);
}

int min( int x, int y )
{
    if( x < y )
    {
        return x;
    }
    else
    {
        return y;
    }
}
