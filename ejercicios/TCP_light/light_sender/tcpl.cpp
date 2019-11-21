#include "tcpl.h"

union Data
{
    int seq_num;
    char str[4];
}data;

tcpl::~tcpl()
{
    delete[] this->shared_buffer;
    delete[] this->package;
    close(this->socket_fd);
}

tcpl::tcpl(char *my_port, char *ip, char *other_port, char *file_name)
{

    this->my_port = my_port;
    this->destiny_ip = ip;
    this->destiny_port = other_port;
    this->file_name = file_name;

    this->file_read_flag = false;
    this->setup_failure = false;

    this->buffer_flag = 'L';

    this->shared_buffer = new char[PACK_THROUGHPUT];
    this->package = new char[PACK_SIZE];

    this->socket_fd = 0;
    this->RN = 0;
    this->SN = 0;

    net_setup(&me,&other,my_port,ip,other_port);
    file.open(file_name);
    if(!file)
    {
        std::cerr << "Sender: Error opening file: " << file_name << ", aborting program." << std::endl;
        this->setup_failure = true;
    }
}

//---------------------------------------------------------------------------------------
//funciones de utilidad
char* tcpl::my_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }

    return dest;
}

//Net Setup, initialize registers and setup socket
void tcpl::net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port)
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

char* tcpl::make_pakage(char* data_block)
{
    //Esto genera posible fuga mejor tener un solo paquete y caerle encima
    char* package = new char[PACK_SIZE];

    data.seq_num = this->SN;
    my_strncpy( package, data.str, 4 );
    package[4] = SEND;
    my_strncpy( package+5, data_block, PACK_THROUGHPUT );
    return package;
}

void tcpl::start_sending()
{
    #pragma omp parallel num_threads(3) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
    {
        int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
        //este hilo se encarga de meter la solicitud en la bolsa
        if (my_thread_n == 0)
        {
			
		}
		//este hilo se encarga de la lectura en la bolsa (revisa las solicitudes y las envía)
        if (my_thread_n == 1)
        {
				
		}
		//este hilo se encarga de sacar de la bolsa (espera acks para eliminar de las bolsas)
        if (my_thread_n == 2)
        {
			
        }
    }
}

//--------------------------------------------
