#include "Nodo_naranja.h"

//Constructor de nodo naranja
Nodo_naranja::Nodo_naranja(char* my_port, char* filename)
{

    this->filename = filename;
    this->file.open(filename);
    this->contador_nodos_verdes = 0;

    if(!file)
    {
        std::cout << "FATAL ERROR: " << filename << " NOT FOUND IN DIRECTORY, ABORTING PROGRAM..." << std::endl;
    }
    else
    {
        read_graph_from_csv();
    }

    this->package = new char[ORANGE_MESSAGE_SIZE];

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

    //Lee el archivo CSV con formato de fila: NODO,VECINO1,VECINO2, ... , VECINO N
    while( !(this->file.eof() ))
    {
        int num_elementos = 0;
        int vecino = 0;

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
                    vecino = std::stoi(word);                     
                    std::cout << " " << vecino;
                    temp_list.push_back(vecino);
                }

                this->grafo_v.insert( std::pair< NODO_V, std::list<int> >(temporal_node, temp_list));
                num_elementos++;
            }

            std::cout << std::endl;
            this->contador_nodos_verdes++;
        }
    }

    this->file.close();
    //show_map();

}

//Simple getter de la variable contador_nodos_verdes
int Nodo_naranja::get_num_nodos_verdes()
{
    return this->contador_nodos_verdes;
}

//Función de utilidad para desplegar el grafo leído del CSV
void Nodo_naranja::show_map()
{
    std::map< NODO_V , std::list<int>>::iterator it;

    for (it = this->grafo_v.begin(); it != this->grafo_v.end(); ++it)
    {
       std::cout << it->first.name << ": ";

       std::list<int>::iterator list_it;

       for(list_it = it->second.begin(); list_it != it->second.end(); ++list_it)
       {
           std::cout << *list_it << " ";
       }

       std::cout << std::endl;

    }

    std::cout << "Contador nodos verdes: " << this->contador_nodos_verdes <<std::endl;
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
            //repetir n veces
            while(confirmations < ORANGE_NODES)
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

                        if(/* DISABLES CODE */ (false))//Recordar añadir: && it_n->second.sin_addr == this->other.sin_addr
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
                 attending = false;

                //ENVIAR MENSAJE DE CONFIRMACIÓN A LOS NARANJAS
                //ENVIAR MENSAJE CON LOS VECINOS AL VERDE SOLICITANTE
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

void Nodo_naranja::make_package_n(short int inicio, int task, short int priority )
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


