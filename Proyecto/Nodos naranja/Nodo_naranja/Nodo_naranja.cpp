#include "Nodo_naranja.h"

Nodo_naranja::Nodo_naranja(char* my_port, char* filename)
{

    this->filename = filename;
    this->file.open(filename);
    this->contador_nodos_verdes = 0;

    if(!file)
    {
        std::cout << "FATAL ERROR: " << filename << " NOT FOUND IN DIRECTORY" << std::endl;
    }
    else
    {
        read_graph_from_csv();
    }

}


void Nodo_naranja::read_graph_from_csv()
{
    std::list<int> prueba;
    std::string line, word, temp, end_line;
    end_line = '\n';

    //Lee el archivo CSV con formato de fila: NODO,VECINO1,VECINO2, ... , VECINO N
    while( !(this->file.eof() ))
    {
        int num_elementos = 0;
        NODO temporal_node;
        int vecino = 0;

        //leer una fila completa y dejarla en "line"
        getline( file, line );
        std::stringstream stream(line);

        if( !line.empty() )
        {
            std::cout << "Linea " << contador_nodos_verdes << ": ";
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
                    this->grafo[temporal_node].push_back(vecino);
                    std::cout << " " << vecino ;
                }

                num_elementos++;
            }

            std::cout << std::endl;
            this->contador_nodos_verdes++;
        }
    }

    //show_map();

}

int Nodo_naranja::get_num_nodos_verdes()
{
    return this->contador_nodos_verdes;
}

void Nodo_naranja::show_map()
{
    std::map<NODO , std::list<int>>::iterator it;

    for (it = this->grafo.begin(); it != this->grafo.end(); ++it)
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


void Nodo_naranja::start_listening()
{
    socklen_t recv_size = sizeof(this->other);
    NODO temp_node;
    while(true)
    {
        ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
        if(bytes_received > 0 && contador_nodos_verdes > 0)
        {
            std::map<NODO , std::list<int>>::iterator it;
            bool inst_flag = false;
            for ( it = this->grafo.begin(); it != this->grafo.end() && !inst_flag ; it++ )
            {
                if(it->first.instantiated)
                {
                    temp_node = it->first;
                    inst_flag = true;
                }
            }
        }
        usleep(500000);
    }

}

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
