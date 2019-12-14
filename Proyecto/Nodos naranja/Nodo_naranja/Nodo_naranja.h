#ifndef NODO_NARANJA_H
#define NODO_NARANJA_H

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
#include <omp.h>

//Macros
#define SEND 0
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
#define CONFIRM_POS_ACK 211
#define CONNECT 200
#define CONNECT_ACK 201
#define REQUEST_POS_ACK 206

/// Avoids instances of a class to be copied
#define DISABLE_COPY_CLASS(ClassName) \
    ClassName(const ClassName& other) = delete; \
    ClassName(ClassName&& temp) = delete; \
    ClassName& operator=(const ClassName& other) = delete; \
    ClassName& operator=(ClassName&& temp) = delete;

class NODO_V;
class Nodo_naranja
{
    DISABLE_COPY_CLASS(Nodo_naranja)
    private:

        struct sockaddr_in me;
        struct sockaddr_in other;

        bool setup_failure;

        int socket_fd;
        int contador_nodos_verdes;
        int contador_nodos_naranjas;
        short int my_priority;

        char* package;
        char* orange_pack;
        char* filename;
        char* orange_filename;
        char* my_ip;
        char* my_port;

        std::ifstream file;
        std::ifstream orange_file;

        std::map < NODO_V, std::list <int> > grafo_v;
        std::map <int, sockaddr_in> grafo_n;

        omp_lock_t writelock;
        char map_flag;

    public:
        Nodo_naranja(char* my_ip, char* my_port, char* filename, char* orange_filename);
        ~Nodo_naranja();

        //Funciones del nodo naranja
        [[noreturn]] void start_listening();
        [[noreturn]] void start_responding();
        void run();
        void net_setup(struct sockaddr_in* me, char* my_port);

        void send_confirmation_n(short int node_id);
        void make_package_n(short int inicio, int task, short int priority);
        void make_package_v(int task, NODO_V nodo);

        //Funciones de utilidad
        char* my_strncpy(char *dest, const char *src, int n);
        int get_num_nodos_verdes();
        int get_num_nodos_naranjas();
        ssize_t call_send_tcpl(sockaddr_in destiny, char* send_package);
        ssize_t call_recv_tcpl(sockaddr_in* source, char* recv_package);

        //Manejo de nodos naranjas y verdes
        void read_graph_from_csv();
        void read_orange_neighbours_from_file();
        void show_green_graph();
        void show_orange_graph();



};

class NODO_V
{
    public:
        int name;
        bool instantiated;

    private:


};

#endif // NODO_NARANJA_H
