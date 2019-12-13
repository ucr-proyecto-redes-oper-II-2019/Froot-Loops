#include <iostream>
#include "Nodo_naranja.h"

using namespace std;

int main(int argc, char** argv)
{
    std::cout << "Nodo Naranja!";
    if(argc < 5)
    {
        std::cout << "NOT ENOUGH ARGUMENTS\nUSAGE: ./orange <my_ip> <my_port> <green_filename> <orange_filename>" << std::endl;
        return 0;
    }
    Nodo_naranja naranja(argv[1], argv[2], argv[3], argv[4]);
    //USAGE: ./lector <my_ip> <my_port> <green_filename> <orange_filename>

    return 0;
}

