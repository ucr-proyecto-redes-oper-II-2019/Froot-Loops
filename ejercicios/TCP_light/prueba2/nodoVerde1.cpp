#include <iostream>
#include "tcpl.h"

using namespace std;

int main(int argc, char* argv[])
{
	char ip[] = "127.0.0.1";
	char port[] = "8100";
	char msg[] = "omg does this work?";
	char msg1[] = "omg does this work? 2";
	char msg2[] = "omg does this work? 3";
	char msg3[] = "omg does this work? 4";
		//nodo verde
    tcpl sender(7300,7200,0);//construcción del tcpl
    sender.send(msg, ip, port);
    sleep(1);
    sender.send(msg1, ip, port);
    sleep(1);
    sender.send(msg2, ip, port);
    sleep(1);
    sender.send(msg3, ip, port);
    cout << "Hello World!" << endl;
    return 0;
}

