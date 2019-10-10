#include "auxiliar_functions.h"

/*
check_emptyness es una función utilizada para ver si el data_block está
vacío y por ende disponible para ser escrito
*/
int check_emptyness(char* str, int size)
{
    for( int index = 0; index < size; ++index)
    {
        if( str[index] != 0)
            return false;
    }
    return true;
}

void flush( list_t* list, int my_RN, int ack_RN )
{
    while ( my_RN < ack_RN)
    {
        pop( list );
        my_RN++;
    }
}

int refresh_rn( list_t* list, int current_rn)
{
    int stop = false;
    int max = PACKAGE_LIMIT;
    int moved = 0;

    while(!stop && (moved < max))
    {
        current_rn++;
        if( list->ack_array[current_rn % PACKAGE_LIMIT] == false )
        {
            printf("Detuvo en RN %d, en la posición %d de la lista  \n", current_rn ,(current_rn % PACKAGE_LIMIT));
            stop = true;
        }
        moved++;
    }

    return (current_rn);
}
