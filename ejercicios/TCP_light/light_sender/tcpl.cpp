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
    delete[] this->tcpl_package;
    close(this->socket_fd);
}

tcpl::tcpl(int port)
{

    this->my_port = port;

    this->file_read_flag = false;
    this->setup_failure = false;

    this->buffer_flag = 'L';

    this->shared_buffer = new char[PACK_THROUGHPUT];
    this->package = new char[PACK_SIZE];
    this->tcpl_package = new char[TCPL_PACK_SIZE];

    this->socket_fd = 0;
  	this->inner_socket_fd = 0;
    this->RN = 0;
    this->SN = 0;

    net_setup(&me,&other,my_port);
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
void tcpl::net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, int my_port)
{
    bzero(source, sizeof(&source)); //se limpian ambos registros de antemano
    bzero(dest, sizeof(&dest));

    source->sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
    source->sin_port = htons(my_port);//Mi puerto donde estoy escuchando
    source->sin_family = AF_INET;
	
		dest->sin_family = AF_INET;
	/*
    dest->sin_addr.s_addr = inet_addr(destiny_ip); //IP destino se especifica en el 2do parametro de linea de comando
    dest->sin_port = htons(atoi(destiny_port)); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando
	*/
	
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd == -1)
    {
        std::cerr << "Sender: Error: Could not create socket correctly, aborting probram." << std::endl;
        this->setup_failure = true;
    }

    int check_bind = bind(socket_fd,(struct sockaddr*)&source,sizeof(source));
    if(check_bind == -1)
    {
        std::cerr << "Sender: Error: Could not bind correctly, aborting probram." << std::endl;
        this->setup_failure = true;
    }
}

void tcpl::make_pakage(char* data_block)
{
    //Esto genera posible fuga mejor tener un solo paquete y caerle encima
    //char* package = new char[PACK_SIZE];

    data.seq_num = this->SN;
    my_strncpy( package, data.str, 4 );
    package[4] = SEND;
    my_strncpy( package+5, data_block, PACK_THROUGHPUT );

}

void tcpl::start_sending()
{
    #pragma omp parallel num_threads(3) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
    {
        int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
        //este hilo se encarga de meter la solicitud en la bolsa
        if (my_thread_n == 0)
        {
					insertar();
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

//-----------------funciones principales de tcpl------------------------

//esta función sólo va a ser invocada por el nodo verde que quiera utilizar tcpl
void tcpl::send(char* sending_package, char* IP, char* send_to_port)
{
	
	//se saca el tamaño en bytes de la dirección IP
	int ip_address_size = strnlen(IP,15);
	data.seq_num = ip_address_size;
	//se construye el paquete de tcpl
	my_strncpy(tcpl_package, data.str, 4);// se copian 4B del tamaño (en bytes) de la IP
	my_strncpy(tcpl_package+4, IP, 15);// se copian 15B de IP porque es el tamaño máximo
	my_strncpy(tcpl_package+19, send_to_port, 4);// se copian 4B del puerto
	make_pakage(sending_package);//se crea el paquete a enviar, el resultado queda en package
	my_strncpy(tcpl_package+23, package, PACK_THROUGHPUT);
	
	other.sin_addr.s_addr = inet_addr("127.0.0.1");
	other.sin_port = htons(TCPL_PORT);
	int len = sizeof(other);
	sendto(socket_fd, tcpl_package, TCPL_PACK_SIZE, 0, (struct sockaddr*)&other, len);
	SN++;
}

void tcpl::insertar()
{
  //se crea un socket propio para el hilo que inserta (porque de otra manera recibiría paquetes de todo mundo)
  struct sockaddr_in tcpl_sock;
  bzero(&tcpl_sock, sizeof(&tcpl_sock)); //se limpian ambos registros de antemano

  tcpl_sock.sin_addr.s_addr = inet_addr("127.0.0.1");
  tcpl_sock.sin_port = htons(TCPL_PORT);//Mi puerto donde estoy escuchando
  tcpl_sock.sin_family = AF_INET;
  
  inner_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  bind(inner_socket_fd,(struct sockaddr*)&tcpl_sock,sizeof(tcpl_sock));
  
  unsigned int len = sizeof(other);
  int bytes_read = 0;

  while(true)
  {
    bytes_read = recvfrom(inner_socket_fd, tcpl_package, TCPL_PACK_SIZE, 0, (struct sockaddr*)&other, &len);
    if (bytes_read > 0)
    {
      Element my_element;
      my_element.ttl = 5;
      
      /*se tiene que desempaquetar el paquete interno de tcpl que consta de
       * 4B de tamaño de ip
       * 15B de IP
       * 4B de puerto
       * 1024B de paquetes
       * 
       * y todos esos datos se almacenan en la estructura my_element, y luego
       * se tiene que almacenar en la bolsa de tcpl
       * */
      
			my_strncpy(data.str, tcpl_package, 4);
      int ip_size = data.seq_num;
      my_element.element_ip_size = ip_size;
      
      my_element.element_ip = new char[ip_size];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
      my_strncpy(my_element.element_ip, tcpl_package+4, ip_size);
      
      my_element.element_port =  new char[4];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
      my_strncpy(my_element.element_port, tcpl_package+19, 4);

      my_element.element_package =  new char[PACK_THROUGHPUT];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
      my_strncpy(my_element.element_package, tcpl_package+23, PACK_SIZE);
      
			//se recupera el número de paquete para usarlo como llave en la bolsa
			my_strncpy(data.str, my_element.element_package, 4);
			int request_number = data.seq_num;
      
			//se procede a meter en la bolsa
			bag[request_number] = my_element;
    }
  }
  
} // You rock homie <3 //

void tcpl::leer()
{
	while(true)
	{
		if(!bag.empty())
		{
			for (auto iterator : bag)
			{
				other.sin_addr.s_addr = inet_addr(iterator.second.element_ip);
				other.sin_port = htons(atoi(iterator.second.element_port));
				sendto(socket_fd, iterator.second.element_package, PACK_SIZE, 0, (struct sockaddr*)&other, sizeof(other));
				iterator.second.ttl--;
			}
		}
	}
}
