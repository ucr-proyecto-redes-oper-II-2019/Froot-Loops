#ifndef NODO_VERDE_H
#define NODO_VERDE_H

#include <cstring>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <list>
#include <fstream>
#include <list>
#include <sstream>
#include <cstdlib>
#include <bits/stdc++.h>

#define GREEN_MESSAGE_SIZE 1015
#define REQUEST_POS 205
#define CONFIRM_POS 210
#define CONNECT 200
#define CONNECT_ACK 201
//Macros
#define SEND 0
#define PACK_THROUGHPUT 512
#define PACK_SIZE 516
#define FILE_OPEN_ERROR -1
#define SOCK_FAILED_ERROR -2
#define BIND_FAILURE -3
#define ERROR_FILE_NOT_FOUND -3
#define ORANGE_HEADER_SIZE 15
#define ORANGE_MESSAGE_SIZE 1015
#define REQUEST_NUM 4
#define BEGIN_CONFIRMATION_ANSWER 2
#define TASK_TO_REALIZE 1
#define PRIORITY_SIZE 2
#define ORANGE_NODES 3
#define REQUEST_POS 205
#define CONFIRM_POS 210
#define CONNECT 200
#define CONNECT_ACK 201

/// Avoids instances of a class to be copied
#define DISABLE_COPY_CLASS(ClassName) \
    ClassName(const ClassName& other) = delete; \
    ClassName(ClassName&& temp) = delete; \
    ClassName& operator=(const ClassName& other) = delete; \
    ClassName& operator=(ClassName&& temp) = delete;

using namespace std;

class Nodo_Verde
{
    DISABLE_COPY_CLASS(Nodo_Verde)

    private:
        char* orange_ip;
        char* orange_port;
        char* my_port;
        char* package;

        list <int> neighbours;

        struct sockaddr_in me;
        struct sockaddr_in other;

        bool setup_failure;

        int socket_fd;
    public:
        //-----------FUNCIONES DE NODO VERDE----------------------//
        Nodo_Verde(char* my_port, char* orange_ip, char* orange_port);
        ~Nodo_Verde();

        //-----------FUNCIONES DE UTILIDAD-----------------------//
        void net_setup(struct sockaddr_in* me, char* my_port);
        void send_instantiation_request();
        void make_package_n(short int inicio, int task, short int priority);
        char* my_strncpy(char *dest, const char *src, int n);




};

#endif // NODO_VERDE_H
