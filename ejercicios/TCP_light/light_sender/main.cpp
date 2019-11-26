#include <iostream>
#include "tcpl.h"

using namespace std;

int main(int argc, char* argv[])
{
    tcpl sender(TCPL_PORT);//construcción del tcpl
    cout << "Hello World!" << endl;
    return 0;
}
