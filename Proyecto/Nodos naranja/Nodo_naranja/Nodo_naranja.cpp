#include "Nodo_naranja.h"

union Data
{
    int seq_num;
    char str[4];
}data;


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

//---------Funcionalidades de nodo naranja--------------//

void Nodo_naranja::start_listening()
{
    socklen_t recv_size = sizeof(this->other);
    NODO temp_node;
    while(true) //
    {
        ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);

        //Si me llega un mensaje de un nodo verde y quedan nodos por instanciar
        if(bytes_received > 0 && contador_nodos_verdes > 0)
        {

            //Repetir hasta conseguir una respuesta positiva:

            std::map<NODO , std::list<int>>::iterator it;
            bool inst_flag = false;

            for ( it = this->grafo.begin(); it != this->grafo.end() && !inst_flag ; it++ )
            {
                if(it->first.instantiated == false )
                {
                    //Seleccionar el siguiente nodo verde que aún no ha sido instanciado en la topología
                    temp_node = it->first;
                    inst_flag = true;
                }
            }

            //Repetir hasta que reciba respuesta de los demás nodos naranjas:

            //Enviar mensaje a los vecinos naranjas (REQUEST POS 205)

            srand( time(NULL)) ;

            int request_number = rand() * 100; //<--RANDOM
            short int inicio = 0;
            char tarea_a_realizar[1];
            tarea_a_realizar[0] = 205;
            short int priority = 0;

            data.seq_num = request_number;
            my_strncpy(package, data.str, REQUEST_NUM);
            my_strncpy(package+4, (char*)&inicio, BEGIN_CONFIRMATION_ANSWER);
            my_strncpy(package+6, tarea_a_realizar ,TASK_TO_REALIZE);
            my_strncpy(package+8, (char*)&priority, PRIORITY_SIZE);

            int positive_confirmations = 0;
            //repetir n veces
            for(int destiny_node = 0; destiny_node < 3; ++destiny_node)
            {
                //Envío request 205 a los demás nodos naranja
                ssize_t bytes_send = sendto(this->socket_fd, package, ORANGE_MESSAGE_SIZE, 0, (struct sockaddr*)&this->other, recv_size);
                usleep(100000);

                //Espero la confirmación para el nodo naranja correspondiente
                ssize_t bytes_received = recvfrom(socket_fd, package, ORANGE_MESSAGE_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
                if(bytes_received > 0)
                {
                    char confirmation[2];
                    char request_pos_ack[1];

                    strncpy( confirmation, package+4, BEGIN_CONFIRMATION_ANSWER );
                    strncpy( request_pos_ack, package+6, TASK_TO_REALIZE);

                    if( atoi(confirmation) == 1 && atoi(request_pos_ack) == 206 )
                    {
                        positive_confirmations++;
                    }
                }
            }

            if(positive_confirmations == 3)
            {
                //ENVIAR MENSAJE DE CONFIRMACIÓN A LOS NARANJAS
                //ENVIAR MENSAJE CON LOS VECINOS AL VERDE SOLICITANTE
            }
        }

        usleep(500000);
    }
}


