#include "Sender.h"

//destructor
Sender::~Sender()
{
	delete[] this->read_data;
    close(this->socket);
    omp_destroy_lock(&writelock1);
    omp_destroy_lock(&writelock2);
}

void Sender::start_sending()
{

}


int Sender::get_socket()
{
	return this->socket;
}

char* Sender::get_read_data()
{
    return this->read_data;
}
