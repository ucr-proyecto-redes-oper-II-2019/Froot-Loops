#include "Nodo_naranja.h"

union Data
{
    int seq_num;
    char str[4];
}data;

bool operator<(const NODO_V& n1, const NODO_V& n2)
{
    return n1.name < n2.name;
}

//Constructor de nodo naranja
Nodo_naranja::Nodo_naranja(char* my_ip, char* my_port, char* filename, char* orange_filename)
{

    this->my_ip = my_ip;
    this->my_port = my_port;

    this->filename = filename;
    this->file.open(filename);

    this->orange_filename = orange_filename;
    this->orange_file.open(orange_filename);

    this->contador_nodos_verdes = 0;
    this->contador_nodos_naranjas = 0;
    this->setup_failure = false;

    omp_init_lock(&writelock);
    this->map_flag = 'L';

    if(!file)
    {
        std::cout << "FATAL ERROR: " << filename << " NOT FOUND IN DIRECTORY, ABORTING PROGRAM..." << std::endl;
        this->setup_failure = true;
    }
    else if(!orange_file)
    {
        std::cout << "FATAL ERROR: " << orange_filename << " NOT FOUND IN DIRECTORY, ABORTING PROGRAM..." << std::endl;
        this->setup_failure = true;
    }
    else
    {
        read_graph_from_csv();
        read_orange_neighbours_from_file();

        show_green_graph();
        show_orange_graph();
    }

    this->package = new char[ORANGE_MESSAGE_SIZE];
    this->orange_pack = new char[ORANGE_MESSAGE_SIZE];

    net_setup(&this->me, this->my_port);
    run();

}

//Destructor nodo naranja
Nodo_naranja::~Nodo_naranja()
{
    delete this->package;
    delete this->orange_pack;
    omp_destroy_lock(&writelock);
    close(this->socket_fd);
    std::cout << "Señal recibida, nodo naranja desconectándose." << std::endl;

}

//-------Funciones de Utilidad----------//


//Simple inicializador con los datos de red proporcionados
void Nodo_naranja::net_setup(struct sockaddr_in* me, char* my_port)
{
    bzero(me, sizeof(&me)); //se limpian ambos registros de antemano

    me->sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
    me->sin_port = htons(atoi(my_port));//Mi puerto donde estoy escuchando
    me->sin_family = AF_INET;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd == -1)
    {
        std::cerr << "Sender: Error: Could not create socket correctly, aborting probram." << std::endl;
        this->setup_failure = true;
    }

    int check_bind = bind(socket_fd,(struct sockaddr*)&this->me,sizeof(this->me));
    if(check_bind == -1)
    {
        std::cerr << "Sender: Error: Could not bind correctly, aborting probram." << std::endl;
        this->setup_failure = true;
    }
}

//Función custom de strncpy
char *Nodo_naranja::my_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }

    return dest;
}


//Función que se encarga de leer del archivo CSV el grafo con el formato específicado en el proyecto
void Nodo_naranja::read_graph_from_csv()
{
    std::string line, word, temp;

    std::cout << std::endl;
    //Lee el archivo CSV con formato de fila: NODO,VECINO1,VECINO2, ... , VECINO N
    while( !(this->file.eof() ))
    {
        int num_elementos = 0;

        //leer una fila completa y dejarla en "line"
        getline( file, line );
        std::stringstream stream(line);

        if( !line.empty() )
        {
            std::cout << "Linea " << contador_nodos_verdes << ": ";
            std::list<int> temp_list;
            NODO_V temporal_node;

            while(getline(stream, word, ','))
            {
                if(num_elementos == 0)
                {
                    temporal_node.name = std::stoi(word);
                    std::cout << "Key: (" << temporal_node.name << "), Vecinos:" ;
                }
                else
                {
                    int vecino = std::stoi(word);
                    std::cout << " " << vecino;
                    temp_list.push_back(vecino);
                }
                num_elementos++;
            }

            this->grafo_v.insert( std::pair< NODO_V, std::list<int> >(temporal_node, temp_list));
            /*std::list<int>::iterator it;
            for(it = temp_list.begin(); it != temp_list.end(); it++)
                std::cout << *it << std::endl;*/
            std::cout << std::endl;
            this->contador_nodos_verdes++;
        }
    }
    this->file.close();
}

//Función que lee el archivo de nodos naranja y llena la estructura de datos de los mismos
void Nodo_naranja::read_orange_neighbours_from_file()
{
    std::cout << std::endl;
    std::string line;
    while( std::getline( this->orange_file, line))
    {
        //Conseguimos una linea del archivo
        std::istringstream iss(line);
        std::string delimitador = ":";
        std::string word;
        struct sockaddr_in temp; //Se utiliza como temporal para asociarle al entero del nodo naranja actual una estructura de red

        size_t pos = 0;
        while (( pos = line.find(delimitador)) != std::string::npos)
        {
            word = line.substr(0, pos);
            //std::cout << "IP: "<< word << ", PORT: ";
            line.erase( 0, pos + delimitador.length());
        }

        std::cout << "Linea " << this->contador_nodos_naranjas << ": IP: "<< word << ", PORT:" << line << std::endl;

        std::cout << "Convertida a char*: " << word.c_str() << ", " << line.c_str() << std::endl;
        //Creamos la estructura de red para el nodo de esta linea del archivo
        temp.sin_addr.s_addr = inet_addr( word.c_str() );
        temp.sin_port = htons( atoi( line.c_str() ));
        temp.sin_family = AF_INET;

        if( (strcmp( this->my_ip, word.c_str())) || (strcmp( this->my_port, line.c_str())) ) //Si la linea que lei no corresponde a mi IP, lo agrego al grafo naranja
        {
            this->grafo_n.insert( std::pair< int, sockaddr_in >( this->contador_nodos_naranjas, temp));
            this->contador_nodos_naranjas++;
        }
    }
    this->orange_file.close();
}

//Simple getter de la variable contador_nodos_verdes
int Nodo_naranja::get_num_nodos_verdes()
{
    return this->contador_nodos_verdes;
}

int Nodo_naranja::get_num_nodos_naranjas()
{
    return this->contador_nodos_naranjas;
}

ssize_t Nodo_naranja::call_send_tcpl(struct sockaddr_in destiny)
{
    //std::cout << "Voy a enviar a " << inet_ntoa(destiny->sin_addr) << " con puerto: " << ntohs(destiny->sin_port)<< std::endl;

    socklen_t recv_size = sizeof(destiny);
    ssize_t bytes_sent = sendto(this->socket_fd, this->package, ORANGE_MESSAGE_SIZE, 0, (struct sockaddr*)&destiny, recv_size);
    return bytes_sent;
}

ssize_t Nodo_naranja::call_recv_tcpl(struct sockaddr_in* source)
{
    socklen_t recv_size = sizeof(source);
    ssize_t bytes_recieved = recvfrom(this->socket_fd, this->package, ORANGE_MESSAGE_SIZE, 0, (struct sockaddr*)source, &recv_size);
     //bytes_received = recvfrom(socket_fd, package, ORANGE_MESSAGE_SIZE, MSG_DONTWAIT, (struct sockaddr*)&g_return_data, &recv_size);
    return bytes_recieved;
}

//Función de utilidad para desplegar el grafo leído del CSV
void Nodo_naranja::show_green_graph()
{
    std::cout << std::endl << "Show Green Graph" << std::endl;
    std::map< NODO_V , std::list<int>>::iterator it;

    for (it = this->grafo_v.begin(); it != this->grafo_v.end(); ++it)
    {
        std::cout << "Nodo #" <<it->first.name << " tiene los vecinos: ";

        std::list<int>::iterator list_it;

        for(list_it = it->second.begin(); list_it != it->second.end(); ++list_it)
        {
            std::cout << *list_it << ", ";
        }
        std::cout << std::endl;

    }

    std::cout << "Contador nodos verdes: " << this->contador_nodos_verdes <<std::endl;
}

void Nodo_naranja::show_orange_graph()
{
    std::cout << std::endl << "Show Orange Graph" << std::endl;
    std::map < int, sockaddr_in >::iterator it;

    for( it = this->grafo_n.begin(); it != this->grafo_n.end(); ++it)
    {
        struct sockaddr_in temp;
        temp = it->second;
        std::cout << "Vecino #" << it->first << " tiene IP: " << inet_ntoa(temp.sin_addr) << " y Puerto:" << ntohs(temp.sin_port) << std::endl;
    }

    std::cout << "Contador nodos naranjas: " << this->contador_nodos_naranjas <<std::endl;
}

void Nodo_naranja::run()
{
    #pragma omp parallel num_threads(2) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
    {
        int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
        if (my_thread_n == 0)
        {
            start_listening();
        }
        if (my_thread_n == 1)
        {
            start_responding();
        }
    }

}

//---------Funcionalidades de nodo naranja--------------//

void Nodo_naranja::start_listening()
{
    socklen_t recv_size = sizeof(this->other);
    NODO_V temp_node;
    ssize_t bytes_received = 0;
    temp_node.name = -1;
    temp_node.instantiated = false;
    bool attending = false;
    struct sockaddr_in g_return_data, last_attending;
    g_return_data.sin_family = AF_INET;
    last_attending.sin_family = AF_INET;
    while(true) //
    {

        if(!attending)
        {
             bytes_received = call_recv_tcpl(&g_return_data);
             std::cout << "Recibo: " << bytes_received << " bytes" << std::endl;

            if( last_attending.sin_addr.s_addr != g_return_data.sin_addr.s_addr || last_attending.sin_port != g_return_data.sin_port )
            {
                last_attending.sin_addr.s_addr = g_return_data.sin_addr.s_addr;
                last_attending.sin_port = g_return_data.sin_port;
                std::cout << "Entre al if" << std::endl;
            }
            else
            {
                std::cout << "Entre al else" << std::endl;
                bytes_received = 0;
            }
        }
        char task = CONNECT;
        //Si me llega un mensaje de un nodo verde y quedan nodos por instanciar
        if(bytes_received > 0 && contador_nodos_verdes > 0 && package[6] == task)
        {

            attending = true;
            //Repetir hasta conseguir una respuesta positiva:

            std::map<NODO_V , std::list<int>>::iterator it;
            bool inst_flag = false;

            //ADQUIRIR CONTROL DEL MAPA
            while(this->map_flag != 'L')
                ;

            omp_set_lock(&this->writelock);

            for ( it = this->grafo_v.begin(); it != this->grafo_v.end() && !inst_flag ; it++ )
            {
                if(it->first.instantiated == false && temp_node.name != it->first.name )
                {
                    std::cout << "He encontrado un nodo no instanciado" << std::endl;
                    //Seleccionar el siguiente nodo verde que aún no ha sido instanciado en la topología
                    temp_node = it->first;
                    inst_flag = true;
                }
            }

            //DEVOLVER CONTROL DEL MAPA
            this->map_flag = 'R';
            omp_unset_lock(&this->writelock);

            int confirmations = 0;
            int positive_confirmations = 0;
            bool negative_confirmations = false;
            //repetir n veces
            while(confirmations < this->contador_nodos_naranjas && !negative_confirmations)
            {

                std::map<int , sockaddr_in>::iterator it_n;
                for ( it_n = this->grafo_n.begin(); it_n != this->grafo_n.end(); it_n++ )
                {
                    make_package_n(temp_node.name,REQUEST_POS,this->my_priority);
                    //Envío request 205 a los demás nodos naranja
                    ssize_t bytes_send = call_send_tcpl(it_n->second);
                    usleep(100000);
                    //Espero la confirmación para el nodo naranja correspondiente
                    ssize_t bytes_received = call_recv_tcpl(&this->other);
                    if(bytes_received > 0)
                    {
                        char confirmation[2];
                        char request_pos_ack[1];

                        my_strncpy( confirmation, package+4, BEGIN_CONFIRMATION_ANSWER );
                        my_strncpy( request_pos_ack, package+6, TASK_TO_REALIZE);

                        if(it_n->second.sin_addr.s_addr == this->other.sin_addr.s_addr)//Recordar añadir: && it_n->second.sin_addr == this->other.sin_addr
                        {
                            confirmations++;
                            if( atoi(confirmation) == 1 && atoi(request_pos_ack) == REQUEST_POS_ACK)
                            {
                                ++positive_confirmations;
                            }
                            else if( atoi(confirmation) == 0 && atoi(request_pos_ack) == 206 )
                            {
                                negative_confirmations = true;
                            }
                        }
                    }
                }

                usleep(500000);
            }

            if(confirmations == positive_confirmations)
            {
                //ADQUIRIR CONTROL DEL MAPA
                while(this->map_flag != 'L')
                    ;


                std::list <int> temp = grafo_v[temp_node];
                this->grafo_v.erase(temp_node);
                temp_node.instantiated = true;
                this->grafo_v.insert( std::pair< NODO_V, std::list<int> >(temp_node, temp));

                //DEVOLVER CONTROL DEL MAPA
                this->map_flag = 'R';
                omp_unset_lock(&this->writelock);

                //ENVIAR MENSAJE DE CONFIRMACIÓN A LOS NARANJAS
                send_confirmation_n();  //Recordar que hay que usar el Send de TCPL dentro de la función
                make_package_v(CONNECT_ACK,temp_node);

                 std::cout << "Voy a mandar " << this->package << std::endl;

                //ENVIAR MENSAJE CON LOS VECINOS AL VERDE SOLICITANTE
                 ssize_t bytes_send = call_send_tcpl(g_return_data);

                 attending = false;
                 bytes_received = 0;
            }
        }

        usleep(500000);
        //sleep(5);
    }
}

void Nodo_naranja::start_responding()
{
    struct sockaddr_in o_return_data;
    while(true)
    {
        ssize_t bytes_received = call_recv_tcpl(&o_return_data);
        std::cout << "Responding: Recibí: " << bytes_received << " bytes" << std::endl;

        char request_pos = REQUEST_POS;
        char confirm_pos = CONFIRM_POS;

        //Si me llega un mensaje de un nodo naranja y respondemos según la solicitud
        if(bytes_received > 0)
        {
            if(package[6] == request_pos) //Me preguntan sobre un nodo instanciado
            {
                my_strncpy(data.str, package+REQUEST_NUM, BEGIN_CONFIRMATION_ANSWER);
                NODO_V temp_node;
                temp_node.name = data.seq_num;
                bool answer = false;

                //ADQUIRIR CONTROL DEL MAPA
                while(this->map_flag != 'R')
                    ;

                omp_set_lock(&this->writelock);

                std::map<NODO_V , std::list<int>>::iterator it;

                for ( it = this->grafo_v.begin(); it != this->grafo_v.end(); it++ )
                {
                    if(it->first.name == temp_node.name)
                    {
                        if(it->first.instantiated)
                        {
                            answer = true;
                        }
                    }
                }

                if( answer )//Si ya tenía el Nodo instanciado
                {
                    char answer = 0;
                    make_package_n(answer, REQUEST_POS_ACK, my_priority);
                    call_send_tcpl(o_return_data);
                }
                else
                {
                    char answer = 1;
                    make_package_n(answer, REQUEST_POS_ACK, my_priority);
                    call_send_tcpl(o_return_data);
                }

                //DEVOLVER CONTROL DEL MAPA
                this->map_flag = 'L';
                omp_unset_lock(&this->writelock);

            }
            else if(package[6] == confirm_pos) //Me confirman la instanciación de un nodo
            {
                my_strncpy(data.str, package+REQUEST_NUM, BEGIN_CONFIRMATION_ANSWER);
                NODO_V temp_node;
                temp_node.name = data.seq_num;

                //ADQUIRIR CONTROL DEL MAPA
                while(this->map_flag != 'R')
                    ;

                omp_set_lock(&this->writelock);

                std::list <int> temp_list = grafo_v[temp_node];
                this->grafo_v.erase(temp_node);
                temp_node.instantiated = true;
                this->grafo_v.insert( std::pair< NODO_V, std::list<int> >(temp_node, temp_list));

                //DEVOLVER CONTROL DEL MAPA
                this->map_flag = 'L';
                omp_unset_lock(&this->writelock);
            }
        }
        usleep(300000);
    }
}

void Nodo_naranja::send_confirmation_n()
{
    socklen_t recv_size = sizeof(this->other);
    std::map<int , sockaddr_in>::iterator it_n;
    for ( it_n = this->grafo_n.begin(); it_n != this->grafo_n.end(); it_n++ )
    {
        make_package_n(it_n->first,CONFIRM_POS,this->my_priority);
        ssize_t bytes_send = call_send_tcpl(it_n->second);
    }
}

void Nodo_naranja::make_package_n(short int inicio, int task, short int priority)
{
    srand( time(nullptr));
    int request_number = rand() % INT_MAX-1; //<--RANDOM


    char tarea_a_realizar[1];
    tarea_a_realizar[0] = task;

    data.seq_num = request_number;
    my_strncpy(orange_pack, data.str, REQUEST_NUM);
    my_strncpy(orange_pack+4, (char*)&inicio, BEGIN_CONFIRMATION_ANSWER);
    my_strncpy(orange_pack+6, tarea_a_realizar ,TASK_TO_REALIZE);
    my_strncpy(orange_pack+8, (char*)&priority, PRIORITY_SIZE);
}

void Nodo_naranja::make_package_v(int task, NODO_V nodo)
{
    srand( time(nullptr));
    int request_number = rand() % INT_MAX-1; //<--RANDOM

    char tarea_a_realizar[1];
    tarea_a_realizar[0] = task;
    data.seq_num = request_number;
    my_strncpy(package, data.str, REQUEST_NUM);

    my_strncpy(package+4,(char*)&nodo.name, 2);
    my_strncpy(package+6,tarea_a_realizar,TASK_TO_REALIZE);
    std::list<int>::iterator list_it;

    int offset = ORANGE_HEADER_SIZE;
    for(list_it = this->grafo_v[nodo].begin(); list_it != this->grafo_v[nodo].end() ;++list_it)
    {
        my_strncpy(package+offset,(char*)&(*list_it),4);
        offset += 4;
    }

}
