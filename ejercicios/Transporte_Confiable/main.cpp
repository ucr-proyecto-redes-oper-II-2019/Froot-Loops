#include <iostream>
#include "Sender.h"

int main(int argc, char** argv)
{
    //Creamos un nuevo sender con el socket que retorna la función
    Sender sender1(socket(AF_INET, SOCK_DGRAM, 0));
    sender1.start_sending();
	return 0;
}
