#include "nodo_verde.h"

union Data
{
    int seq_num;
    char str[4];
}data;

Nodo_Verde::Nodo_Verde(char *my_port, char *orange_ip, char *orange_port)
{
    this->my_port = my_port;
    this->orange_ip = orange_ip;
    this->orange_port = orange_port;

    this->setup_failure = false;
    net_setup(&this->me, this->my_port);
    this->package = new char[GREEN_MESSAGE_SIZE];

    if(!setup_failure)
        send_instantiation_request();


}

Nodo_Verde::~Nodo_Verde()
{
    delete this->package;
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

    int check_bind = bind(socket_fd,(struct sockaddr*)&me,sizeof(me));
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
    socklen_t recv_size = sizeof(this->other);
    ssize_t bytes_received = 0;
    bool end = false;
    Data data;

    while(!end)
    {
        bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&this->other, &recv_size);
        if(bytes_received > 0)
        {
            for(int bytes_count = 15; bytes_count < bytes_received;bytes_count += 4)
            {
                my_strncpy(data.str,package+bytes_count,4);
                neighbours.push_back(data.seq_num);
            }
            end = true;

        }

        //send to tcpl
    }

    cout << "Mis Vecinos son: " << endl;

    std::list<int>::iterator list_it;
    for(list_it = neighbours.begin();list_it != neighbours.end();++list_it)
    {
        cout << *list_it;
    }

    cout << endl;



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
/*
⣿⣿⣿⣿⣿⣿⣿⡿⡛⠟⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⠿⠨⡀⠄⠄⡘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⠿⢁⠼⠊⣱⡃⠄⠈⠹⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⡿⠛⡧⠁⡴⣦⣔⣶⣄⢠⠄⠄⠹⣿⣿⣿⣿⣿⣿⣿⣤⠭⠏⠙⢿⣿⣿
⣿⡧⠠⠠⢠⣾⣾⣟⠝⠉⠉⠻⡒⡂⠄⠙⠻⣿⣿⣿⣿⣿⡪⠘⠄⠉⡄⢹⣿
⣿⠃⠁⢐⣷⠉⠿⠐⠑⠠⠠⠄⣈⣿⣄⣱⣠⢻⣿⣿⣿⣿⣯⠷⠈⠉⢀⣾⣿
⣿⣴⠤⣬⣭⣴⠂⠇⡔⠚⠍⠄⠄⠁⠘⢿⣷⢈⣿⣿⣿⣿⡧⠂⣠⠄⠸⡜⡿
⣿⣇⠄⡙⣿⣷⣭⣷⠃⣠⠄⠄⡄⠄⠄⠄⢻⣿⣿⣿⣿⣿⣧⣁⣿⡄⠼⡿⣦
⣿⣷⣥⣴⣿⣿⣿⣿⠷⠲⠄⢠⠄⡆⠄⠄⠄⡨⢿⣿⣿⣿⣿⣿⣎⠐⠄⠈⣙
⣿⣿⣿⣿⣿⣿⢟⠕⠁⠈⢠⢃⢸⣿⣿⣶⡘⠑⠄⠸⣿⣿⣿⣿⣿⣦⡀⡉⢿
⣿⣿⣿⣿⡿⠋⠄⠄⢀⠄⠐⢩⣿⣿⣿⣿⣦⡀⠄⠄⠉⠿⣿⣿⣿⣿⣿⣷⣨
⣿⣿⣿⡟⠄⠄⠄⠄⠄⠋⢀⣼⣿⣿⣿⣿⣿⣿⣿⣶⣦⣀⢟⣻⣿⣿⣿⣿⣿
⣿⣿⣿⡆⠆⠄⠠⡀⡀⠄⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⡿⡅⠄⠄⢀⡰⠂⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
*/
