#include <iostream>
#include "tcpl.h"


using namespace std;

int main(int argc, char* argv[])
{
	//instancia procesadora
	tcpl sender(8100,8200,8300);
	sender.start_sending();
	std::cout << "esta arepa sirve " << std::endl;
    return 0;
}
