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
#include <sstream>
#include <cstdlib>
#include <bits/stdc++.h>

//Macros definidas para el nodo verde
#define ORANGE_HEADER_SIZE 15
#define GREEN_MESSAGE_SIZE 1015
#define REQUEST_NUM 4
#define BEGIN_CONFIRMATION_ANSWER 2
#define TASK_TO_REALIZE 1
#define PRIORITY_SIZE 2
#define ORANGE_NODES 3
#define REQUEST_POS 205
#define CONFIRM_POS 210
#define CONNECT 200
#define CONNECT_ACK 201
#define BLUE_REQ 600
#define BLUE_ANS 601

/// Avoids instances of a class to be copied
#define DISABLE_COPY_CLASS(ClassName) \
    ClassName(const ClassName& other) = delete; \
    ClassName(ClassName&& temp) = delete; \
    ClassName& operator=(const ClassName& other) = delete; \
    ClassName& operator=(ClassName&& temp) = delete;

class Nodo_Verde
{
    DISABLE_COPY_CLASS(Nodo_Verde)

    private:
        ///Variables usadas para comunicarse con el naranja
        char* orange_ip;
        char* orange_port;
        char* my_port;
        char* package;

        std::list<int> neighbours; ///<Estructura de datos donde se guardan sus vecinos

        //Estructuras de red
        struct sockaddr_in me;
        struct sockaddr_in other;

        bool setup_failure;
        int socket_fd;

    public:
        /**
         * @brief Nodo_Verde Constructor de nodo verde.
         * @param my_port Puerto del cual va a escuchar el nodo verde.
         * @param orange_ip IP donde se envía la solicitud de instanciación.
         * @param orange_port Puerto donde se envía la solicitud de instanciación.
         */
        [[noreturn]] Nodo_Verde(char* my_port, char* orange_ip, char* orange_port);
        ~Nodo_Verde();
        /**
         * @brief net_setup Función que se encarga de configurar las estructuras de red con los argumentos recibidos.
         * @param me Estructura sockaddr_in donde guarda los datos de mi IP y mi puerto.
         * @param my_port Puerto con el cual se desea levantar el verde.
         */
        void net_setup(struct sockaddr_in* me, char* my_port);
        /**
         * @brief send_instantiation_request Función que se encarga de enviar solicitud de instanciación al naranja.
         */
        [[noreturn]] void send_instantiation_request();
        /**
         * @brief make_package_n Prepara el paquete de envío al naranja.
         * @param inicio Espacio del paquete donde va el inicio/respuesta del paquete.
         * @param task Espacio del paquete donde va el código de tarea.
         * @param priority Espacio del paquete donde va la prioridad.
         */
        void make_package_n(short int inicio, int task, short int priority);
        /**
         * @brief make_package_a Prepara el paquete de envío al azul..
         * @param inicio Espacio del paquete donde va el inicio/respuesta del paquete.
         * @param task Espacio del paquete donde va el código de tarea.
         * @param priority Espacio del paquete donde va la prioridad.
         */
        void make_package_a(short int inicio, int task, short int priority);
        /**
         * @brief call_send_tcpl Envía un paquete con la interfaz TCPL.
         * @return Cantidad de bytes enviados exitósamente.
         */
        ssize_t call_send_tcpl();
        /**
         * @brief call_recv_tcpl Recibe un paquete con la interfaz TCPL.
         * @return Cantidad de bytes recibidos exitósamente.
         */
        ssize_t call_recv_tcpl();
        /**
         * @brief my_strncpy Versión modificada del strncpy de la stdlib.
         * @param dest Cadena de caracteres destino a copiar.
         * @param src Cadena de caracteres de origen desde donde copiar.
         * @param n Cantidad de bytes a copiar.
         * @return Hilera resultante despues del copiado.
         */
        char* my_strncpy(char *dest, const char *src, int n);

};

#endif // NODO_VERDE_H
