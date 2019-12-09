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
    //AGREGAR NETSETUP

}

//Destructor nodo naranja
Nodo_naranja::~Nodo_naranja()
{
    delete this->package;
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

    int check_bind = bind(socket_fd,(struct sockaddr*)&me,sizeof(me));
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

        if( (strcmp( this->my_ip, word.c_str())) ) //Si la linea que lei no corresponde a mi IP, lo agrego al grafo naranja
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
        std::cout << "Vecino #" << it->first << " tiene IP: " << temp.sin_addr.s_addr << " y Puerto: " << temp.sin_port << std::endl;
    }

    std::cout << "Contador nodos naranjas: " << this->contador_nodos_naranjas <<std::endl;
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
    while(true) //
    {

        if(!attending)
        {
             bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
             attending = true;
        }

        //Si me llega un mensaje de un nodo verde y quedan nodos por instanciar
        if(bytes_received > 0 && contador_nodos_verdes > 0)
        {
            //Repetir hasta conseguir una respuesta positiva:

            std::map<NODO_V , std::list<int>>::iterator it;
            bool inst_flag = false;

            for ( it = this->grafo_v.begin(); it != this->grafo_v.end() && !inst_flag ; it++ )
            {
                if(it->first.instantiated == false && temp_node.name != it->first.name )
                {
                    //Seleccionar el siguiente nodo verde que aún no ha sido instanciado en la topología
                    temp_node = it->first;
                    inst_flag = true;
                }
            }

            make_package_n(temp_node.name,REQUEST_POS,this->my_priority);

            int confirmations = 0;
            int positive_confirmations = 0;
            bool negative_confirmations = false;
            //repetir n veces
            while(confirmations < ORANGE_NODES && !negative_confirmations)
            {

                std::map<int , sockaddr_in>::iterator it_n;
                for ( it_n = this->grafo_n.begin(); it_n != this->grafo_n.end(); it_n++ )
                {
                    //Envío request 205 a los demás nodos naranja
                    ssize_t bytes_send = sendto(this->socket_fd, package, ORANGE_MESSAGE_SIZE, 0, (struct sockaddr*)&it_n->second, recv_size);
                    usleep(100000);
                    //Espero la confirmación para el nodo naranja correspondiente
                    ssize_t bytes_received = recvfrom(socket_fd, package, ORANGE_MESSAGE_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
                    if(bytes_received > 0)
                    {
                        char confirmation[2];
                        char request_pos_ack[1];

                        my_strncpy( confirmation, package+4, BEGIN_CONFIRMATION_ANSWER );
                        my_strncpy( request_pos_ack, package+6, TASK_TO_REALIZE);

                        if(it_n->second.sin_addr.s_addr == this->other.sin_addr.s_addr)//Recordar añadir: && it_n->second.sin_addr == this->other.sin_addr
                        {
                            confirmations++;
                            if( atoi(confirmation) == 1 && atoi(request_pos_ack) == 206 )
                            {
                                ++positive_confirmations;
                            }
                        }
                    }
                }

                usleep(500000);
            }

            if(confirmations == positive_confirmations)
            {
                 //ENVIAR MENSAJE DE CONFIRMACIÓN A LOS NARANJAS
                send_confirmation_n();  //Recordar que hay que usar el Send de TCPL dentro de la función

                //ENVIAR MENSAJE CON LOS VECINOS AL VERDE SOLICITANTE

                 attending = false;
            }
        }

        usleep(500000);
    }
}

void Nodo_naranja::send_confirmation_n()
{
    socklen_t recv_size = sizeof(this->other);
    std::map<int , sockaddr_in>::iterator it_n;
    for ( it_n = this->grafo_n.begin(); it_n != this->grafo_n.end(); it_n++ )
    {
        make_package_n(it_n->first,CONFIRM_POS,this->my_priority);
        ssize_t bytes_send = sendto(this->socket_fd, package, ORANGE_MESSAGE_SIZE, 0, (struct sockaddr*)&it_n->second, recv_size);
    }
}

void Nodo_naranja::make_package_n(short int inicio, int task, short int priority)
{
    srand( time(nullptr)) ;
    int request_number = rand() % INT_MAX-1; //<--RANDOM


    char tarea_a_realizar[1];
    tarea_a_realizar[0] = task;

    data.seq_num = request_number;
    my_strncpy(package, data.str, REQUEST_NUM);
    my_strncpy(package+4, (char*)&inicio, BEGIN_CONFIRMATION_ANSWER);
    my_strncpy(package+6, tarea_a_realizar ,TASK_TO_REALIZE);
    my_strncpy(package+8, (char*)&priority, PRIORITY_SIZE);
}

void Nodo_naranja::make_package_v(short int inicio, int task, short int priority )
{

}


