#include "Sender.h"

//destructor
Sender::~Sender()
{
	delete[] this->read_data;
    close(this->socket_fd);
    omp_destroy_lock(&writelock1);
    omp_destroy_lock(&writelock2);
}

void Sender::start_sending()
{
    char hilera[] = "Hola";
    packages.push_back(hilera);
}


int Sender::get_socket()
{
    return this->socket_fd;
}

char* Sender::get_read_data()
{
    return this->read_data;
}
