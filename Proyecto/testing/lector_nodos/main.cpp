#include "lectorcsv.h"
#include <iostream>

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cout << "C mamó mande archivos archivo pls" << std::endl;
        return 0;
    }

    lectorCSV lector(argv[1], argv[2]);

    return 0;
}
