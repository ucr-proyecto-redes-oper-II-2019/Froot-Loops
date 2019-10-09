#ifndef PACKER_H
#define PACKER_H

#include <unistd.h>
#include "list.h"
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)
#define PACK_SIZE 516 //pack_size total, incluyendo headers

void packer(char* data_block, int* all_data_read, int* block_is_free, list_t* list);

#endif
