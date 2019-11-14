#include "Sender.h"
//#include <omp.h>

union Data
{
  int seq_num;
  char str[4];
}data;

//destructor
Sender::~Sender()
{
	delete[] this->read_data;
    close(this->socket_fd);
    omp_destroy_lock(&writelock1);
    omp_destroy_lock(&writelock2);
}

int Sender::get_socket()
{
    return this->socket_fd;
}

char* Sender::get_read_data()
{
    return this->read_data;
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
    //Esto genera posible fuga mejor tener un solo paquete y caerle encima
    char* package = new char[516];

    package[0] = SEND;
    data.seq_num = this->SN;
    my_strncpy( package+1, data.str, 3 );
    my_strncpy( package+4, data_block, PACK_THROUGHPUT );
    return package;
}

void Sender::start_sending()
{
    #pragma omp parallel num_threads(3) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
    {
        int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
        if (my_thread_n == 0)
            file_reader();
        if (my_thread_n == 1)
            packer();
        if (my_thread_n == 2)
            dummy_sender();
    }



    /*#pragma omp parallel num_threads(3) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
    {
        int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
        if (my_thread_n == 0)
            file_reader();
        if (my_thread_n == 1)
            packer();
        if (my_thread_n == 2)
            send_package_receive_ack();
    }*/
}

void Sender::file_reader()
{
    int pack_count = 0;
    file.open(file_name);

    while( !(file.eof()) )
    {

        while(this->buffer_flag != 'L')
            ;

        omp_set_lock(&writelock1); //ADQUIERE EL LOCK

        if( pack_count == 0 ) //Si es el pack 0, los primeros 50 bytes son del nombre del archivo
        {
            my_strncpy(read_data,file_name,strlen(file_name));
            file.read( read_data+50, 462 ); //Deja los primeros 50 bytes vacios y el resto de datos
        }
        else //Para todos los demas, leee los 512 bytes completos
        {
            file.read( read_data, 512 );
        }

        this->buffer_flag = 'E';
        omp_unset_lock(&writelock1); //SUELTA EL LOCK
        pack_count++;
         //std::cout << "aún no termino" << std::endl;
    }

    //Prende la bandera que indica que el archivo termino
    file_read = true;
    file.close();
    std::cout << "Se terminó de leer el archivo con un total de: " << pack_count << " paquetes" << std::endl;

}

void Sender::packer()
{
    char* new_package = nullptr;
    while(!this->file_read)
    {
        while(this->buffer_flag != 'E')
            ;

        //std::cout << "packer toma candado buffer" << std::endl;
        omp_set_lock(&this->writelock1);
        new_package = make_pakage(this->read_data);
        omp_unset_lock(&this->writelock1);
        //std::cout << "packer suelta candado buffer" << std::endl;

        //Espero mi turno para usar la lista

        while(this->list_flag != 'I')
            ;

        //std::cout << "packer toma candado lista" << std::endl;
        omp_set_lock(&this->writelock2);
        if(this->packages.size() < 10)
        {
            //std::cout << " voy a insertar "<< packages.size() << std::endl;
            this->packages.push_back(new_package);
            //std::cout << "List s: " << packages.size() << std::endl;
            this->buffer_flag = 'L';
        }

        this->list_flag = 'E';
        omp_unset_lock(&this->writelock2);
        //std::cout << "packer suelta candado lista" << std::endl;

        //std::cout << "Estoy en el packer" << std::endl;

    }
}


void Sender::send_package_receive_ack()
{
    while( !packages.empty() || !this->file_read )
	{
		while( this->list_flag != 'E')
			;

        omp_set_lock(&writelock2);
        socklen_t recv_size = sizeof(this->other);
        ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
		if(bytes_received > 0 && package[0] == 1)
		{
			my_strncpy(data.str, package+1, 3);
            flush(&RN, data.seq_num);
		}
        this->list_flag = 'I';
        omp_unset_lock(&writelock2);
		//falta mandar todos los paquetes
        std::list<char*>::iterator it;
        for (it = this->packages.begin(); it != this->packages.end(); ++it)
        {
            sendto(this->socket_fd, *it,PACK_SIZE,0, (struct sockaddr*)&this->other, recv_size);
        }

	}
}

void Sender::flush(int* my_RN, int ack_RN)
{
    while ( *my_RN < ack_RN )
    {
        char* ptr = this->packages.front();
        this->packages.pop_front();
        delete ptr;
        *(my_RN)++;
    }
}

void Sender::reader_dummy()
{

    int pack_count_dummy = 0;
    std::ofstream archivo_dummy;
    archivo_dummy.open("dummy.jpg");

    while ( !this->file_read )
    {
        while( this->buffer_flag != 'E')
            ;

        omp_set_lock(&writelock1);

        if(pack_count_dummy == 0)
        {
            char dummy_name[50] ;
            my_strncpy( dummy_name, read_data, 50 );
            std::cout << "Nombre Archivo: " << dummy_name << std::endl;

            archivo_dummy.write( read_data+50, 462 );
        }
        else
        {
            archivo_dummy.write( read_data, PACK_THROUGHPUT );
        }

        this->buffer_flag = 'L';
        omp_unset_lock(&writelock1);
        pack_count_dummy++;
    }

    archivo_dummy.close();
    std::cout << "Archivo Dummy Finalizado!" << std::endl;

}

void Sender::dummy_sender()
{
    int pack_count_dummy = 0;
    std::ofstream archivo_dummy;
    archivo_dummy.open("dummy.jpg");

    //for(int z = 0; z < 288; ++z)
    while( packages.empty() == false || this->file_read == false )
    {
        //std::cout << "List s: " << packages.size() << std::endl;

        while( this->list_flag != 'E')
            ;

        //std::cout << "sender toma candado lista" << std::endl;
        omp_set_lock(&writelock2);
        if(packages.size() >= 1)
        {
            //std::cout << "Antes " << std::endl;
            std::list<char*>::iterator it;
            for (it = this->packages.begin(); it != this->packages.end(); ++it)
            {
                //std::cout << "Despues " << std::endl;
                if(pack_count_dummy == 0)
                {
                    char dummy_name[50];
                    my_strncpy( dummy_name, *it+4, 50 );
                    std::cout << "Nombre Archivo: " << dummy_name << std::endl;
                    archivo_dummy.write( *it+54, 462 );
                }
                else
                {
                    archivo_dummy.write( *it+4, PACK_THROUGHPUT );
                }
                pack_count_dummy++;
            }
        }
        //flush
        if(packages.size() >= 1)
        {
            for(int i = 0; i < packages.size(); ++i)
            {
                char* ptr = this->packages.front();
                this->packages.pop_front();
                delete ptr;
            }
        }

        this->list_flag = 'I';
        omp_unset_lock(&writelock2);
        //std::cout << "sender suelta candado lista" << std::endl;
    }

    archivo_dummy.close();
    std::cout << "Archivo Dummy Finalizado!" << std::endl;
}
