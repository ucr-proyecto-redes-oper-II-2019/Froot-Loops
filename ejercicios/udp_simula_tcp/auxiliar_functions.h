#ifndef AUX_H
#define AUX_H

#define false 0
#define true 1

#include <stdio.h>
#include "list.h"


int check_emptyness(char* str, int size);
void flush( list_t* list, int my_RN, int ack_RN );
int refresh_rn( list_t* list, int current_rn);

#endif
