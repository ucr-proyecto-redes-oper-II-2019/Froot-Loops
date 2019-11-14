#include <iostream>
#include "Sender.h"

int main(int argc, char** argv)
{
    //argv[1] = <my_port>, argv[2] = <destiny_ip>, argv[3] = <destiny_port> argv[4] = file_path

    if(argc < 4)//validacion de parametros
    {
        printf("No enough arguments given\n Usage: <my_port>, argv[2] = <destiny_ip>, argv[3] = <destiny_port> argv[4] = file_path\n");
        return EXIT_FAILURE;
    }

    //Creamos un nuevo sender con el socket que retorna la función
    Sender sender1(argv[1],argv[2],argv[3],argv[4]);
    sender1.start_sending();

	return 0;
}
