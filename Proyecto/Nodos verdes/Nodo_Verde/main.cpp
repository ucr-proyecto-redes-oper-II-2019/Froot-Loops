#include <iostream>
#include "nodo_verde.h"

using namespace std;

int main(int argc, char** argv)
{

    if(argc < 3)
    {
        cout << "GREEN BOI: ERROR: NOT ENOUGH ARGUMENTS\nUSAGE: ./N_V <my_port> <orange_ip> <orange_port>" << endl;
        return 0;
    }

    Nodo_Verde(argv[1], argv[2], argv[3]);

    return 0;
}

