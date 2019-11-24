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


struct NODO
{
    int name;
    bool instantiated;

}nodo_t;



#define SEND 0
#define PACK_THROUGHPUT 512
#define PACK_SIZE 516
#define FILE_OPEN_ERROR -1
#define SOCK_FAILED_ERROR -2
#define BIND_FAILURE -3
#define ERROR_FILE_NOT_FOUND -3

/// Avoids instances of a class to be copied
#define DISABLE_COPY_CLASS(ClassName) \
    ClassName(const ClassName& other) = delete; \
    ClassName(ClassName&& temp) = delete; \
    ClassName& operator=(const ClassName& other) = delete; \
    ClassName& operator=(ClassName&& temp) = delete;


class Nodo_naranja
{
    DISABLE_COPY_CLASS(Nodo_naranja)
    private:
        struct sockaddr_in me;
        struct sockaddr_in other;

        int socket_fd;
        bool setup_failure;
        char* package;
        int contador_nodos_verdes;
        char* filename;
        std::ifstream file;

        std::map < NODO, std::list <int> > grafo;
        //std::map<NODO,std::list<int>> grafo;

        //std::map<int, std::list<NODO>> grafo;



    public:
        Nodo_naranja(char* my_port,char* filename);
        void start_listening();
        void net_setup(struct sockaddr_in* me, char* my_port);

        //Function to read csv
        void read_graph_from_csv();
        int get_num_nodos_verdes();
        void show_map();
};




#endif // NODO_NARANJA_H
