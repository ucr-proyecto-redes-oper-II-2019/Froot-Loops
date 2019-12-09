#include <iostream>
#include "tcpl.h"

using namespace std;

int main(int argc, char* argv[])
{
    tcpl sender(7100,7200,7300);//construcción del tcpl
    sender.start_sending();
    cout << "Hello World!" << endl;
    return 0;
}
