#include <iostream>
#include "nodo_verde.h"

using namespace std;

int main(int argc, char** argv)
{

    //Verificación de Argumentos
    if(argc < 3)
    {
        cout << "GREEN NODE: ERROR: NOT ENOUGH ARGUMENTS\nUSAGE: ./N_V <my_port> <orange_ip> <orange_port>" << endl;
        return 0;
    }

    //Si los argumentos son válidos, levantamos el nodo verde
    Nodo_Verde(argv[1], argv[2], argv[3]);

    return 0;
}

