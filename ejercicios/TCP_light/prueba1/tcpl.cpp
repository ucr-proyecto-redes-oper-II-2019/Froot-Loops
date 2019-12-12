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
    delete[] this->ack_package;
    delete[] this->received_package;
    close(this->socket_fd);
    close(this->inner_socket_fd);
}

//****************************************************************************************************************//
//BIG DISCLAIMER: "port" debe ser diferente entre nodo verde y la instancia procesadora, pero tcpl_port debe ser igual entre ellos dos
//también node_port debe ser igual a port para la instancia procesadora de tcpl
tcpl::tcpl(int port, int tcpl_port, int node_port)
{

    this->my_port = port;
    this->my_tcpl_port = tcpl_port;
    this->my_node_port = node_port;

    this->file_read_flag = false;
    this->setup_failure = false;

    this->buffer_flag = 'L';

    this->shared_buffer = new char[PACK_THROUGHPUT];
    this->package = new char[PACK_SIZE];
    this->ack_package = new char[PACK_SIZE];
    this->tcpl_package = new char[TCPL_PACK_SIZE];
    this->received_package = new char[PACK_SIZE];

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

    int check_bind = bind(socket_fd,(struct sockaddr*)&me,sizeof(me));
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
		SN++;
}

void tcpl::start_sending()
{
    #pragma omp parallel num_threads(6) //shared(data_block, last_package_read, all_data_read, list, wait_flag)
    {
        //obtiene el identificador del hilo
        int my_thread_n = omp_get_thread_num();
        //este hilo se encarga de meter la solicitud en la bolsa
        if (my_thread_n == 0)
        {
					insertar();
				}
				//este hilo se encarga de la lectura en la bolsa (revisa las solicitudes y las envía)
        if (my_thread_n == 1)
        {
					leer();
				}
				//este hilo se encarga de recibir acks y meterlos en su bolsa
        if (my_thread_n == 2)
        {
					recibir_ack();
        }
        //este hilo se encarga de leer la bolsa de acks y eliminar elementos de la bolsa de envíos
        if (my_thread_n == 3)
        {
					eliminar();
				}
				//este hilo se encarga de meter paquetes recibidos en la bolsa de recibidos
				if (my_thread_n == 4)
        {
					recibir_paquete();
				}
				//este hilo se encarga de estar tratando de pasar los paquetes recibidos a capa superior
				if (my_thread_n == 5)
        {
					pasar_capa_superior();
				}
    }
}

//-----------------funciones principales de tcpl----------------------//

//esta función sólo va a ser invocada por el nodo verde que quiera utilizar tcpl
void tcpl::send(char* sending_package, char* IP, char* send_to_port)
{
	//se saca el tamaño en bytes de la dirección IP
	//std::cout << sending_package << std::endl;
	int ip_address_size = strnlen(IP,15);
	data.seq_num = ip_address_size;
	//se construye el paquete de tcpl
	my_strncpy(tcpl_package, data.str, 4);// se copian 4B del tamaño (en bytes) de la IP
	my_strncpy(tcpl_package+4, IP, 15);// se copian 15B de IP porque es el tamaño máximo
	my_strncpy(tcpl_package+19, send_to_port, 4);// se copian 4B del puerto
	make_pakage(sending_package);//se crea el paquete a enviar, el resultado queda en package
	my_strncpy(tcpl_package+23, package, PACK_SIZE);
	
	other.sin_addr.s_addr = inet_addr("127.0.0.1");
	other.sin_port = htons(my_tcpl_port);
	int len = sizeof(other);
	sendto(socket_fd, tcpl_package, TCPL_PACK_SIZE, 0, (struct sockaddr*)&other, len);
}

void tcpl::receive(char* receiving_package)
{
	recvfrom(socket_fd, receiving_package, PACK_THROUGHPUT, 0, NULL, NULL);
}

void tcpl::insertar()
{
  //se crea un socket propio para el hilo que inserta (porque de otra manera recibiría paquetes de todo mundo)
  //este socket sólo recibe de loopback
  struct sockaddr_in tcpl_sock;
  bzero(&tcpl_sock, sizeof(&tcpl_sock)); //se limpian ambos registros de antemano

  tcpl_sock.sin_addr.s_addr = inet_addr("127.0.0.1");
  tcpl_sock.sin_port = htons(my_tcpl_port);//Mi puerto donde estoy escuchando
  tcpl_sock.sin_family = AF_INET;
  
  inner_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  bind(inner_socket_fd,(struct sockaddr*)&tcpl_sock,sizeof(tcpl_sock));
  
  unsigned int len = sizeof(tcpl_sock);

  while(true)
  {
    int bytes_read = recvfrom(inner_socket_fd, tcpl_package, TCPL_PACK_SIZE, 0, NULL, NULL);
    if (bytes_read > 0)
    {
			std::cout << "Sí recibí algo" << std::endl;
      Element my_element;
      my_element.ttl = 1;
      
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
      std::cout << ip_size << std::endl;
      my_element.element_ip_size = ip_size;
      
      my_element.element_ip = new char[ip_size];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
      my_strncpy(my_element.element_ip, tcpl_package+4, ip_size);
      
      my_element.element_port =  new char[4];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
      my_strncpy(my_element.element_port, tcpl_package+19, 4);

      my_element.element_package =  new char[PACK_THROUGHPUT];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
      my_strncpy(my_element.element_package, tcpl_package+23, PACK_SIZE);
      
      char tmp[1024];
      my_strncpy(tmp, my_element.element_package+5, 1024);
      tmp[1023] = 0;
      std::cout << tcpl_package+28 << std::endl;
      
			//se recupera el número de paquete para usarlo como llave en la bolsa
			my_strncpy(data.str, my_element.element_package, 4);
			int request_number = data.seq_num;
      
			//se procede a meter en la bolsa
			#pragma omp critical (bolsa)
			{
			bag[request_number] = my_element;
			}
    }
    bytes_read = 0; //reinicia los bytes leídos porque de otra manera el hilo sigue creyendo que recibió algo
    //sleep(1);
  }
  
} // You rock homie <3 //

void tcpl::leer()
{
	while(true)
	{
		#pragma omp critical (bolsa)
		{
			if(!bag.empty())
			{
				std::cout << "hay algo en la bolsa para enviar" << std::endl;
				for (auto iterator : bag)
				{
					if(!iterator.second.ttl == 0)
					{
						other.sin_addr.s_addr = inet_addr(iterator.second.element_ip);
						//std::cout << iterator.second.element_ip << std::endl;
						other.sin_port = htons(atoi(iterator.second.element_port));
						//std::cout << iterator.second.element_port << std::endl;
						int bytes_enviados = sendto(socket_fd, iterator.second.element_package, PACK_SIZE, 0, (struct sockaddr*)&other, sizeof(other));
						std::cout << "El mensaje a enviar es: " << iterator.second.element_package+5 << std::endl;
						std::cout << "Se enviaron: " << bytes_enviados << " bytes." <<std::endl;
						std::cout << "El ttl es: " << iterator.second.ttl << std::endl;
						bag[iterator.first].ttl--;
					}
				}
			}
			
		}
		usleep(500000);
	}
}

void tcpl::recibir_ack()
{
	while(true)
	{
		int bytes_read = recvfrom(socket_fd, ack_package, PACK_SIZE, 0, NULL, NULL);
		if( bytes_read > 0 && ack_package[4] == ACK)
		{
			std::cout << "Se recibieron: " << bytes_read << " bytes de ack." <<std::endl;
			//se extrae el identificador de paquete para ubicarlo en el mapa (bolsa)
			my_strncpy(data.str, ack_package, 4);
			int key = data.seq_num;
			ack_bag.push_back(key);
		}
	}
}

void tcpl::eliminar()
{
	ack_bag.clear();
	while(true)
	{
		#pragma omp critical (bolsa)
		{
			if(!ack_bag.empty())
			{
				std::cout << "Se entra a eliminar paquetes por ack"<<std::endl;
				std::cout << ack_bag.size() <<std::endl;

				for (int i = ack_bag.size(); i > 0; i--)
				{
					delete[] bag[ack_bag[i]].element_ip;
					delete[] bag[ack_bag[i]].element_package;
					delete[] bag[ack_bag[i]].element_port;
					bag.erase(ack_bag[i]);
					ack_bag.pop_back();
					std::cout << ack_bag.size() <<std::endl;
				}
			}
		}
		//sleep(1);
	}
}

void tcpl::recibir_paquete()
{
	struct sockaddr_in paquet_sock;
  bzero(&paquet_sock, sizeof(&paquet_sock)); //se limpian ambos registros de antemano

  paquet_sock.sin_family = AF_INET;
  unsigned int len = sizeof(paquet_sock);
	while(true)
	{
		int bytes_read = recvfrom(socket_fd, received_package, PACK_SIZE, 0, (struct sockaddr*)&paquet_sock, &len);
		if( bytes_read > 0 && received_package[4] == SEND)
		{
			std::cout << "Se recibieron: " << bytes_read << " bytes." <<std::endl;
			Element my_element;
			//se extrae el identificador de paquete para ubicarlo en el mapa (bolsa)
			my_strncpy(data.str, received_package, 4);
			int key = data.seq_num;
			std::cout << key << std::endl;
			if(received_bag.count(key) == 0)
			{
				my_element.element_package =  new char[PACK_THROUGHPUT];//Recordar hacer delete al eliminar el paquete de la bolsa por ttl o por ack
				my_strncpy(my_element.element_package, received_package+5, PACK_THROUGHPUT);
				received_bag[key] = my_element;
				
				//procede a enviar un ack del paquete recién recibido
				received_package[4] = ACK;
				int b = sendto(socket_fd, received_package, PACK_SIZE, 0, (struct sockaddr*)&paquet_sock, sizeof(paquet_sock));
				std::cout << "ACK " << b << std::endl;
			}
			else
			{
				std::cout << "El paquete ya fue recibido"<<std::endl;
			}
		}
	}
}

void tcpl::pasar_capa_superior()
{
	struct sockaddr_in capa_sock;
  bzero(&capa_sock, sizeof(&capa_sock)); //se limpian ambos registros de antemano

  capa_sock.sin_addr.s_addr = inet_addr("127.0.0.1");
  capa_sock.sin_port = htons(my_node_port);//Mi puerto donde estoy escuchando
  capa_sock.sin_family = AF_INET;
	while(true)
	{
		if(!received_bag.empty())
		{
			for (const auto iterator : received_bag)
			{
				int bytes_sent = sendto(socket_fd, iterator.second.element_package, PACK_SIZE, 0, (struct sockaddr*)&capa_sock, sizeof(capa_sock));
				received_bag.erase(iterator.first);
			}
			
		}
		
	}
}
