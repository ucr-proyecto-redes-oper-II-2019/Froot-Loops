#include <iostream>
#include "tcpl.h"

using namespace std;

int main(int argc, char* argv[])
{
	char buffer[PACK_THROUGHPUT];
	//instancia nodo verde
    tcpl sender(8300,8200,0);//construcción del tcpl
    sender.receive(buffer);
    cout << buffer << endl;
    sender.receive(buffer);
    cout << buffer << endl;
    sender.receive(buffer);
    cout << buffer << endl;
    sender.receive(buffer);
    cout << buffer << endl;
    return 0;
}
