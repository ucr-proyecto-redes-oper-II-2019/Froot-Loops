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

char* Sender::my_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
        dest[i] = src[i];

    return dest;
}


void Sender::net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port)
{
  bzero(source, sizeof(*source)); //se limpian ambos registros de antemano
  bzero(dest, sizeof(*dest));

  source->sin_family = AF_INET;
  source->sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
  source->sin_port = htons(atoi(my_port));//Mi puerto donde estoy escuchando

  dest->sin_family = AF_INET;
  dest->sin_addr.s_addr = inet_addr(destiny_ip); //IP destino se especifica en el 2do parametro de linea de comando
  dest->sin_port = htons(atoi(destiny_port)); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando
}

char* Sender::make_pakage(char* data_block)
{
    char* package = new char[516];
    package[1] = SEND;
    data.seq_num = this->SN;
    my_strncpy( package+1, data.str, 3 );
    my_strncpy( package+4, data_block, PACK_THROUGHPUT ); //se c
    return package;
}

void Sender::packer()
{
    while(!this->file_read)
    {
        while(this->buffer_flag != 'E')
            ;

        omp_set_lock(&this->writelock1);
        make_pakage(this->read_data);
        this->buffer_flag = 'L';
        omp_unset_lock(&this->writelock1);

        while(this->list_flag != 'I')
            ;

        //while(this->packages)


    }
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

void Sender::send_package_receive_ack(struct sockaddr_in* source, struct sockaddr_in* dest)
{
	while( !packages.empty() && !file.eof() )
	{
		while( this->list_flag != 'E')
			;
			
		socklen_t recv_size = sizeof(dest);
		int bytes_received = recvfrom(socket_fd, package, PACK_THROUGHPUT, MSG_DONTWAIT, (struct sockaddr*)&dest, &recv_size);
		if(bytes_received > 0 && package[0] == 1)
		{
			my_strncpy(data.str, package+1, 3);
			flush(this->packages, RN, data.seq_num);
		}
		//falta mandar todos los paquetes
	}
}

void Sender::flush(list<char*>* list, int* my_RN, int ack_RN)
{
	while ( my_RN < ack_RN)
    {
        list.pop_front();
        my_RN++;
    }
}
