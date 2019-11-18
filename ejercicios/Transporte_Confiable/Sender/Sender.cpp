#include "Sender.h"

union Data
{
    int seq_num;
    char str[4];
}data;

//Constructor
Sender::Sender(char *my_port, char *ip, char *other_port, char *file_name)
{
    this->my_port = my_port;
    this->destiny_ip = ip;
    this->destiny_port = other_port;
    this->file_name = file_name;

    this->file_read_flag = false;
    this->setup_failure = false;

    this->buffer_flag = 'L';
    this->list_flag = 'I';

    this->shared_buffer = new char[PACK_THROUGHPUT];
    this->package = new char[PACK_SIZE];

    this->socket_fd = 0;
    this->RN = 0;
    this->SN = 0;

    omp_init_lock(&writelock1);
    omp_init_lock(&writelock2);

    net_setup(&me,&other,my_port,ip,other_port);
    file.open(file_name);
    if(!file)
    {
        std::cerr << "Sender: Error opening file: " << file_name << ", aborting program." << std::endl;
        this->setup_failure = true;
    }
}

//Destructor
Sender::~Sender()
{
    delete[] this->shared_buffer;
    delete[] this->package;
    close(this->socket_fd);
    omp_destroy_lock(&writelock1);
    omp_destroy_lock(&writelock2);
}

//Socket Getter
int Sender::get_socket()
{
    return this->socket_fd;
}

//Shared buffer Getter
char* Sender::get_read_data()
{
    return this->shared_buffer;
}

//Setup-flag Getter
bool Sender::get_setup_failure()
{
    return this->setup_failure;
}

//Custom version of strncpy
char* Sender::my_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }

    return dest;
}

//Net Setup, initialize registers and setup socket
void Sender::net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port)
{
    bzero(source, sizeof(&source)); //se limpian ambos registros de antemano
    bzero(dest, sizeof(&dest));

    source->sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
    source->sin_port = htons(atoi(my_port));//Mi puerto donde estoy escuchando
    source->sin_family = AF_INET;

    dest->sin_family = AF_INET;
    dest->sin_addr.s_addr = inet_addr(destiny_ip); //IP destino se especifica en el 2do parametro de linea de comando
    dest->sin_port = htons(atoi(destiny_port)); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd == -1)
    {
        std::cerr << "Sender: Error: Could not create socket correctly, aborting probram." << std::endl;
        this->setup_failure = true;
    }

    int check_bind = bind(socket_fd,(struct sockaddr*)&me,sizeof(me));
    if(check_bind == -1)
    {
        std::cerr << "Sender: Error: Could not bind correctly, aborting probram." << std::endl;
        this->setup_failure = true;
    }
}

//Build package along with net header
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

//Flush old packages from list
void Sender::flush(int ack_RN)
{
    //std::cout << "Haré flush tamño lista:" << this->package_list.size() << std::endl;
    //std::cout << "El ack rn es: " << ack_RN << std::endl;

    while ( this->RN < ack_RN )
    {
        char* ptr = this->package_list.front();
        //if(this->package_list.size() > 0)
        this->package_list.pop_front();
        delete ptr;
        this->RN++;
        //*(my_RN)++;
    }

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
        {
            send_package_receive_ack();
            //dummy_sender();
            //even_dummier_sender();
        }
    }
}


void Sender::file_reader()
{
    int pack_count = 0;

    while( !(file.eof()) )
    {

        while(this->buffer_flag != 'L')
            ;

        omp_set_lock(&writelock1); //ADQUIERE EL LOCK

        if( pack_count == 0 ) //Si es el pack 0, los primeros 50 bytes son del nombre del archivo
        {
            my_strncpy(shared_buffer,file_name,strlen(file_name));
            file.read( shared_buffer+50, 462 ); //Deja los primeros 50 bytes vacios y el resto de datos
        }
        else //Para todos los demas, leee los 512 bytes completos
        {
            file.read( shared_buffer, 512 );
        }

        this->buffer_flag = 'E';
        omp_unset_lock(&writelock1); //SUELTA EL LOCK
        pack_count++;
    }
    //Prende la bandera que indica que el archivo termino
    file_read_flag = true;
    file.close();
    std::cout << "Se terminó de leer el archivo con un total de: " << pack_count << " paquetes" << std::endl;

}

void Sender::packer()
{

    while(!this->file_read_flag)
    {
        while(this->buffer_flag != 'E')
            ;

        omp_set_lock(&this->writelock1);
        char* new_package = make_pakage(this->shared_buffer);
        omp_unset_lock(&this->writelock1);

        //Espero mi turno para usar la lista
        while(this->list_flag != 'I')
            ;

        //std::cout << "packer toma candado lista" << std::endl;
        omp_set_lock(&this->writelock2);
        if(this->package_list.size() < 10)
        {

            this->package_list.push_back(new_package);
            this->SN++;
            //std::cout << "SN depués de insertar:" << this->SN << std::endl;
            this->buffer_flag = 'L';
        }

        //if(!this->file_read_flag)
        this->list_flag = 'E';
        omp_unset_lock(&this->writelock2);
    }
    //this->list_flag = 'E';
}


void Sender::send_package_receive_ack()
{
    //char mi_paquete[] = "Esta es la prueba de los paquetes";
    socklen_t recv_size = sizeof(this->other);
    while( !package_list.empty() || !this->file_read_flag )
    {
        //std::cout << "Empecé el ciclo " << std::endl;
        while( this->list_flag != 'E')
        {
           ; //std::cout << "Estoy esperando"  << std::endl;
        }


        omp_set_lock(&writelock2);
        ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
        if(bytes_received > 0 && package[0] == 1)
        {
            //std::cout << "Recibí mi tamaño es " << this->package_list.size() << std::endl;
            my_strncpy(data.str, package+1, 3);
//            std::cout << "ack rn: " << data.seq_num << std::endl;
//            std::cout << "RN antes de flush: " << this->RN << std::endl;
            flush(data.seq_num);
//            std::cout << "RN después de flush: " << this->RN << std::endl;
//            std::cout << "Tamaño de la lista después: " << this->package_list.size() << std::endl;
        }

        if(!this->file_read_flag)
            this->list_flag = 'I';

        omp_unset_lock(&writelock2);
        //falta mandar todos los paquetes
        std::list<char*>::iterator it;
        for (it = this->package_list.begin(); it != this->package_list.end(); ++it)
        {
//            std::cout << "Paquete enviado con primer byte "<< (int)package[0] << std::endl;
//            std::cout << "Paquete de prueba lleno de texto " << mi_paquete << std::endl;
//            ssize_t bytes_send = sendto(this->socket_fd, *it+54,PACK_SIZE,0, (struct sockaddr*)&this->other, recv_size);
            ssize_t bytes_send = sendto(this->socket_fd, *it,PACK_SIZE,0, (struct sockaddr*)&this->other, recv_size);

//            static_cast<int>
//            ssize_t bytes_send = sendto(this->socket_fd, mi_paquete ,50,0, (struct sockaddr*)&this->other, recv_size);
//            std::cout << "Bytes enviados: " << bytes_send << std::endl;
        }

        //sleep(2);
    }

    bool surrender_flag = false;

    this->shared_buffer[0] = '*';
    this->package = make_pakage(this->shared_buffer);

    for(int i = 0; i < 5 && !surrender_flag; ++i)
    {
        sendto(this->socket_fd,package,PACK_SIZE,0, (struct sockaddr*)&this->other, recv_size);
        usleep(500000);
        ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
        if(bytes_received > 0 && package[0] == 1)
        {
            my_strncpy(data.str, package+1, 3);
            if(data.seq_num == RN)
                surrender_flag = true;
        }

    }
}

//-----------------------------------------------------------TESTING--------------------------------------------------//

void Sender::reader_dummy()
{

    int pack_count_dummy = 0;
    std::ofstream archivo_dummy;
    archivo_dummy.open("dummy.jpg");

    while ( !this->file_read_flag )
    {
        while( this->buffer_flag != 'E')
            ;

        omp_set_lock(&writelock1);

        if(pack_count_dummy == 0)
        {
            char dummy_name[50] ;
            my_strncpy( dummy_name, shared_buffer, 50 );
            std::cout << "Nombre Archivo: " << dummy_name << std::endl;

            archivo_dummy.write( shared_buffer+50, 462 );
        }
        else
        {
            archivo_dummy.write( shared_buffer, PACK_THROUGHPUT );
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
    archivo_dummy.open("dummy.txt");

    //for(int z = 0; z < 288; ++z)
    while( package_list.empty() == false || this->file_read_flag == false )
    {
        //std::cout << "List s: " << packages.size() << std::endl;

        while( this->list_flag != 'E')
            ;

        //std::cout << "sender toma candado lista" << std::endl;
        omp_set_lock(&writelock2);
        if(package_list.size() >= 1)
        {
            //std::cout << "Antes " << std::endl;
            std::list<char*>::iterator it;
            for (it = this->package_list.begin(); it != this->package_list.end(); ++it)
            {

                std::cout << "Esto es lo que tengo: " << *it << std::endl;
                //std::cout << "Despues " << std::endl;
                if(pack_count_dummy == 0)
                {
                    char dummy_name[50];
                    my_strncpy( dummy_name, *it+4, 50 );
                    std::cout << "Nombre Archivo: " << dummy_name << std::endl;
                    archivo_dummy.write( *it+54, 462 );
                    std::cout << "Acompañado de: " << *it+54 << std::endl;
                }
                else
                {
                    archivo_dummy.write( *it+4, PACK_THROUGHPUT );
                }
                pack_count_dummy++;
            }
        }
        //flush
        if(package_list.size() >= 1)
        {
            for(int i = 0; i < package_list.size(); ++i)
            {
                char* ptr = this->package_list.front();
                this->package_list.pop_front();
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

void Sender::even_dummier_sender()
{
    socklen_t recv_size = sizeof(this->other);
    while( !package_list.empty() || !this->file_read_flag )
    {
        std::list<char*>::iterator it;
        for (it = this->package_list.begin(); it != this->package_list.end(); ++it)
        {
            //std::cout << "Voy a mandar estos datos: " << *it+54 << std::endl;
            ssize_t bytes_send = sendto(this->socket_fd, *it+54,PACK_SIZE,0, (struct sockaddr*)&this->other, recv_size);
            std::cout << "Bytes enviados: " << bytes_send << std::endl;
        }
    }
}

