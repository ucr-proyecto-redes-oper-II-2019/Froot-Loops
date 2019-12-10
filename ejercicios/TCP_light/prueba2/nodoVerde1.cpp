#include <iostream>
#include "tcpl.h"

using namespace std;

int main(int argc, char* argv[])
{
	char ip[] = "127.0.0.1";
	char port[] = "8100";
	char msg[] = "omg does this work?";
	char msg2[] = "Los portals tienen mejores gráficos en linux porque\n rastarfarersher blah blah holi cómo están\n";
		//nodo verde
    tcpl sender(7300,7200,0);//construcción del tcpl
    sender.send(msg, ip, port);
    sleep(1);
    sender.send(msg, ip, port);
    sleep(1);
    sender.send(msg, ip, port);
    sleep(1);
    sender.send(msg, ip, port);
    sleep(1);
    sender.send(msg2, ip, port);
    cout << "Hello World!" << endl;
    return 0;
}

