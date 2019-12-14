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
#include <sstream>
#include <cstdlib>
#include <bits/stdc++.h>
#include <omp.h>

//Macros definidas para nodo naranja
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

class NODO_V; ///<Forward Declaration

class Nodo_naranja
{
    DISABLE_COPY_CLASS(Nodo_naranja)
    private:

        ///Estructuras de red
        struct sockaddr_in me;
        struct sockaddr_in other;

        bool setup_failure;

        int socket_fd;
        int contador_nodos_verdes;
        int contador_nodos_naranjas;
        short int my_priority;

        ///Datos utilizados para levantar el nodo naranja
        char* package;
        char* orange_pack;
        char* filename;
        char* orange_filename;
        char* my_ip;
        char* my_port;

        ///ifstreams para leer la configuración de los grafos desde los archivos
        std::ifstream file;
        std::ifstream orange_file;

        ///Estructuras de datos utilizadas para representar el grafo de los verdes y de los naranjas respectivamente
        std::map < NODO_V, std::list <int> > grafo_v;
        std::map <int, sockaddr_in> grafo_n;

        ///Herramientas utilizadas para manejar la sincronización del mapa compartido entre hilos
        omp_lock_t writelock;
        char map_flag;

    public:
        /**
         * @brief Nodo_naranja Constructor de nodo naranja.
         * @param my_ip IP En en la cual se levanta el nodo naranja.
         * @param my_port Puerto del cual va a escuchar el nodo naranja.
         * @param filename Archivo donde se encuentra el grafo de los verdes con el formato especificado.
         * @param orange_filename Archivo donde se encuentran las IP y puertos de los naranjas vecinos.
         */
        Nodo_naranja(char* my_ip, char* my_port, char* filename, char* orange_filename);
        ~Nodo_naranja();
        //Funciones del nodo naranja
        /**
         * @brief start_listening Hilo que se encarga de escuchar requests de verdes.
         */
        [[noreturn]] void start_listening();
        /**
         * @brief start_responding Hilo que se encarga de responder a otros naranjas.
         */
        [[noreturn]] void start_responding();
        /**
         * @brief run Función que crea y manda a correr los hilos del naranja.
         */
        void run();
        /**
         * @brief net_setup Función que se encarga de configurar las estructuras de red con los argumentos recibidos.
         * @param me Estructura sockaddr_in donde guarda los datos de mi IP y mi puerto.
         * @param my_port Puerto con el cual se desea levantar el verde.
         */
        void net_setup(struct sockaddr_in* me, char* my_port);
        /**
         * @brief send_confirmation_n Función que envia confirmación a los otros naranjas cuando se logra instanciar un nodo verde.
         * @param node_id ID del nodo que se logró instanciar correctamente.
         */
        void send_confirmation_n(short int node_id);
        /**
         * @brief make_package_n Prepara el paquete de envío a otros naranjas.
         * @param inicio Espacio del paquete donde va el inicio/respuesta del paquete.
         * @param task Espacio del paquete donde va el código de tarea.
         * @param priority Espacio del paquete donde va la prioridad.
         */
        void make_package_n(short int inicio, int task, short int priority);
        /**
         * @brief make_package_v Prepara el paquete de respuesta al verde.
         * @param task Espacio del paquete donde va el código de tarea.
         * @param nodo ID del nodo verde al cual le estoy respondiendo.
         */
        void make_package_v(int task, NODO_V nodo);

        ///Funciones de utilidad
        /**
         * @brief my_strncpy Versión modificada del strncpy de la stdlib.
         * @param dest Cadena de caracteres destino a copiar.
         * @param src Cadena de caracteres de origen desde donde copiar.
         * @param n Cantidad de bytes a copiar.
         * @return Hilera resultante despues del copiado.
         */
        char* my_strncpy(char *dest, const char *src, int n);
        /**
         * @brief get_num_nodos_verdes Getter simple de variable contador_nodos_verdes.
         */
        int get_num_nodos_verdes();
        /**
         * @brief get_num_nodos_naranjas Getter simple de variable contador_nodos_naranjas.
         */
        int get_num_nodos_naranjas();
        /**
         * @brief call_send_tcpl Recibe un paquete con la interfaz TCPL.
         * @param destiny Destinatario del mensaje.
         * @param send_package Cual paquete se envia.
         * @return Cantidad de bytes enviados exitósamente.
         */
        ssize_t call_send_tcpl(sockaddr_in destiny, char* send_package);
        /**
         * @brief call_recv_tcpl Recibe un paquete con la interfaz TCPL.
         * @param source Estructura donde se guarda quién me envió el mensaje.
         * @param recv_package Donde se quiere recibir el mensaje.
         * @return Cantidad de bytes recibidos exitósamente.
         */
        ssize_t call_recv_tcpl(sockaddr_in* source, char* recv_package);

        ///Manejo de archivos de nodos naranjas y verdes
        /**
         * @brief read_graph_from_csv Lee los nodos verdes según la topología del grafo.
         */
        void read_graph_from_csv();
        /**
         * @brief read_orange_neighbours_from_file Lee los vecinos naranjas según el archivo proporcionado.
         */
        void read_orange_neighbours_from_file();
        /**
         * @brief show_green_graph Función de testing para demostrar la lectura correcta del archivo de los verdes.
         */
        void show_green_graph();
        /**
         * @brief show_orange_graph Función de testing para demostrar la lectura correcta del archivo de los naranjas.
         */
        void show_orange_graph();



};

///<Clase utilizada para representar los nodos verdes del grafo.
class NODO_V
{
    public:
        int name;
        bool instantiated;
    private:
};

#endif // NODO_NARANJA_H
