#ifndef F_READER_H
#define F_READER_H

#include <stdio.h>
#define true 1
#define false 0
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)

void file_reader(char* file_name, char* data_block, int* all_data_read, 
	int* block_is_free);

#endif
