#include "nodo_verde.h"

union Data
{
    int seq_num;
    char str[4];
}data;

Nodo_Verde::Nodo_Verde(char *my_port, char *orange_ip, char *orange_port)
{
    std::cout << "Green Node!" << std::endl;
    this->my_port = my_port;
    this->orange_ip = orange_ip;
    this->orange_port = orange_port;

    this->setup_failure = false;
    net_setup(&this->me, this->my_port);
    this->package = new char[GREEN_MESSAGE_SIZE];

    if(!setup_failure)
        std::cout << "Sending instantiation request..." << std::endl;
        send_instantiation_request();
}

Nodo_Verde::~Nodo_Verde()
{
    delete this->package;
    close(this->socket_fd);
    std::cout << "Señal recibida, nodo verde desconectándose." << std::endl;
}

void Nodo_Verde::net_setup(sockaddr_in *me, char *my_port)
{
    bzero(me, sizeof(&me)); //se limpian ambos registros de antemano
    bzero(&other, sizeof(&other));

    this->me.sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
    this->me.sin_port = htons(atoi(my_port));//Mi puerto donde estoy escuchando
    this->me.sin_family = AF_INET;

    this->other.sin_family = AF_INET;
    this->other.sin_addr.s_addr = inet_addr(orange_ip); //IP destino se especifica en el 2do parametro de linea de comando
    this->other.sin_port = htons(atoi(orange_port)); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando

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

char *Nodo_Verde::my_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }

    return dest;
}


void Nodo_Verde::send_instantiation_request()
{
    ssize_t bytes_recieved = 0;
    bool end = false, last_neighbor = false;
    Data data;

    make_package_n( 1, CONNECT, 0 );
    ssize_t bytes_sent = call_send_tcpl();
    //std::cout << "He enviado: " << bytes_sent << std::endl;

    while(!end)
    {
        bytes_recieved = call_recv_tcpl();

        char tarea = package[6];
        if(bytes_recieved > 0)
        {
            std::cout << "Recibí respuesta de Naranja de " << bytes_recieved << "bytes" << std::endl;
            for(int bytes_count = 15; !last_neighbor; bytes_count += 4)
            {
                my_strncpy(data.str,package+bytes_count,4);

                if(data.seq_num == 0)
                    last_neighbor = true;
                else
                    neighbours.push_back(data.seq_num);
            }
            end = true;
        }

        make_package_n( 1, CONNECT, 0 );
        ssize_t bytes_sent = call_send_tcpl();
        usleep(250000);
    }

    std::cout << "Mis Vecinos son: " << std::endl;
    std::list<int>::iterator list_it;
    for(list_it = neighbours.begin();list_it != neighbours.end();++list_it)
    {
        std::cout << *list_it << ", ";
    }

    std::cout << std::endl;

}

void Nodo_Verde::make_package_n(short int inicio, int task, short int priority)
{
    srand( time(nullptr));
    int request_number = rand() % INT_MAX-1; //<--RANDOM


    char tarea_a_realizar[1];
    tarea_a_realizar[0] = task;

    data.seq_num = request_number;
    my_strncpy(package, data.str, REQUEST_NUM);
    my_strncpy(package+4, (char*)&inicio, BEGIN_CONFIRMATION_ANSWER);
    my_strncpy(package+6, tarea_a_realizar ,TASK_TO_REALIZE);
    my_strncpy(package+8, (char*)&priority, PRIORITY_SIZE);
}

ssize_t Nodo_Verde::call_send_tcpl()
{
    socklen_t recv_size = sizeof(this->other);
    ssize_t bytes_sent = sendto(this->socket_fd, this->package, GREEN_MESSAGE_SIZE, 0, (struct sockaddr*)&this->other, recv_size);
    return bytes_sent;
}

ssize_t Nodo_Verde::call_recv_tcpl()
{
    socklen_t recv_size = sizeof(this->other);
    ssize_t bytes_recieved = recvfrom(this->socket_fd, this->package, GREEN_MESSAGE_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
    return bytes_recieved;
}
