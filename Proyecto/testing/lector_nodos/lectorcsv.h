#ifndef LECTORCSV_H
#define LECTORCSV_H

#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <cstring>
#include <list>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include <algorithm>
#include <iterator>
#include <vector>



#define ERROR_FILE_NOT_FOUND -3

typedef struct 
{
    int name;
    bool instantiated;

}NODO_V;

class lectorCSV
{

    private:
        std::ifstream file;
        std::ifstream orange_file;
        
        std::map < NODO_V, std::list <int> > grafo_v;
        std::map <int, sockaddr_in> grafo_n;
        
        char* filename;
		char* orange_filename;
        
        int contador_nodos_verdes;
		int contador_nodos_naranjas;
        


    public:

        lectorCSV(char* filename, char* orange_filename);
        ~lectorCSV();
        void read_graph_from_csv();
        void show_map();

		        
        int get_num_nodos_verdes();
        int get_num_nodos_naranjas();
        
		void read_orange_neighbours_from_file();
		void show_orange_neighbours();

};

#endif // LECTORCSV_H
