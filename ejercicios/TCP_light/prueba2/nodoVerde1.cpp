#include <iostream>
#include "tcpl.h"

using namespace std;

int main(int argc, char* argv[])
{
	char ip[] = "127.0.0.1";
	char port[] = "8200";
	char msg[] = "omg does this work?";
		//nodo verde
    tcpl sender(7300,7200,0);//construcción del tcpl
    sender.send(msg, ip, port);
    cout << "Hello World!" << endl;
    return 0;
}

