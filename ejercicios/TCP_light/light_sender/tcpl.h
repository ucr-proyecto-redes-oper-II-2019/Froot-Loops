#ifndef TCPL_H
#define TCPL_H

#include <list>
#include <fstream>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <unordered_map>
#include <string.h>

#define SEND 0
#define PACK_THROUGHPUT 1024
#define PACK_SIZE 1029  //5 bytes de encabezado 1KB(1024bytes) de datos
#define TCPL_PACK_SIZE 1052 //tamaño del paquete interno de tcpl
#define TCPL_PORT 7200

union Data;

struct Element
{
  int element_ip_size;
  char* element_port;
  char* element_ip;
  char* element_package;
	int ttl;
};

typedef std::unordered_map<int,Element> element_map;

/// Avoids instances of a class to be copied
#define DISABLE_COPY_CLASS(ClassName) \
    ClassName(const ClassName& other) = delete; \
    ClassName(ClassName&& temp) = delete; \
    ClassName& operator=(const ClassName& other) = delete; \
    ClassName& operator=(ClassName&& temp) = delete;

class tcpl
{

DISABLE_COPY_CLASS(tcpl)

private:

    bool file_read_flag;
    bool setup_failure;

    int my_port;
    char* destiny_ip;
    char* destiny_port;
    char* package;//buffer usado para enviar y recibir paquetes
    char* tcpl_package;
    char* shared_buffer;

    char buffer_flag;

    struct sockaddr_in me;
    struct sockaddr_in other;

    int socket_fd;
  	int inner_socket_fd;
    int SN;
    int RN;

		element_map bag;
	
//-----------//Funciones de utilidad privadas\\---------
		char* my_strncpy(char *dest, const char *src, int n);
    void make_pakage(char *data_block);
    void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, int my_port);
  	void insertar();
  	void leer();
    
//-----------------------------------------------------

public:

    tcpl(int port);
    ~tcpl();

    //Funciones de ligth_sender
    void send(char* sending_package, char* IP, char* send_to_port);
    void receive();
    void start_sending();
};

#endif // TCPL_H
