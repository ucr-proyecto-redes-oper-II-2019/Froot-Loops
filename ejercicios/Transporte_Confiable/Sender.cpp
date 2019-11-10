#include "Sender.h"
#include <omp.h>

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

    file_reader();
}


int Sender::get_socket()
{
    return this->socket_fd;
}

char* Sender::get_read_data()
{
    return this->read_data;
}

void Sender::file_reader()
{
    int pack_count = 0;
    file.open(file_name);

    while( !(file.eof()) )
    {
        //REVISAR USO DE LOCK
        omp_set_lock(&writelock1); //ADQUIERE EL LOCK

        if( pack_count == 0 ) //Si es el pack 0, los primeros 50 bytes son del nombre del archivo
        {
            file.read( read_data+50, 462 ); //Deja los primeros 50 bytes vacios y el resto de datos
        }
        else //Para todos los demas, leee los 512 bytes completos
        {
            file.read( read_data, 512 );
        }
        omp_unset_lock(&writelock1); //SUELTA EL LOCK

    }
    //Prende la bandera que indica que el archivo termino
    file_read = true;
    file.close();

}
