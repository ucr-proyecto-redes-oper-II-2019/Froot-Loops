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

#define SEND 0
#define PACK_THROUGHPUT 1024
#define PACK_SIZE 1029  //5 bytes de encabezado 1KB(1024bytes) de datos

union Data;

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

    char* my_port;
    char* destiny_ip;
    char* destiny_port;
    char* file_name;
    char* package;//buffer usado para enviar y recibir paquetes
    char* shared_buffer;

    char buffer_flag;

    std::ifstream file;

    struct sockaddr_in me;
    struct sockaddr_in other;

    int socket_fd;
    int SN;
    int RN;

public:

    tcpl(char *my_port, char *ip, char *other_port, char *file_name);
    ~tcpl();

    //Funciones de ligth_sender
    void send();
    void receive();

    //Funciones de utilidad
    char* my_strncpy(char *dest, const char *src, int n);
    char* make_pakage(char *data_block);
    void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port);
};

#endif // TCPL_H
