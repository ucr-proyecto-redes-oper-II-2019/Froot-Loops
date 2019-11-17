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
#include <queue>

#define SEND 0
#define PACK_THROUGHPUT 512
#define PACK_SIZE 516
#define FILE_OPEN_ERROR -1
#define SOCK_FAILED_ERROR -2
#define BIND_FAILURE -3

#ifndef SENDER_H
#define SENDER_H

/// Avoids instances of a class to be copied
#define DISABLE_COPY_CLASS(ClassName) \
    ClassName(const ClassName& other) = delete; \
    ClassName(ClassName&& temp) = delete; \
    ClassName& operator=(const ClassName& other) = delete; \
    ClassName& operator=(ClassName&& temp) = delete;


class Sender
{
    DISABLE_COPY_CLASS(Sender)
    private:

        bool file_read_flag;
        bool setup_failure;

        char* my_port;
        char* destiny_ip;
        char* destiny_port;
        char* file_name;
        char* package;//buffer usado para enviar y recibir paquetes
        char* shared_buffer;

        char list_flag;
        char buffer_flag;

        std::ifstream file;

        struct sockaddr_in me;
        struct sockaddr_in other;

        std::list <char*> package_list;

        omp_lock_t writelock1;
        omp_lock_t writelock2;

        int socket_fd;
        int SN;
        int RN;

    public:

        Sender(char* my_port, char* destiny_ip, char* destiny_port, char* filename);
        ~Sender();

        //Funciones de sender
        void file_reader();
        void packer();
        void send_package_receive_ack();
        void start_sending();

        //Funciones de utilidad
        char* my_strncpy(char *dest, const char *src, int n);
        char* make_pakage(char *data_block);
        void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port);
        void flush(int ack_RN);

        //Getters y Setters
        bool get_setup_failure();
        int get_socket();
        char* get_read_data();

        //Pruebas de testing
        void even_dummier_sender();
        void dummy_sender();
        void reader_dummy();


};



#endif // SENDER_H
