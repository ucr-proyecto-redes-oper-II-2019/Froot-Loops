#ifndef PITCHER_H
#define PITCHER_H

#include <stdio.h>
#include "list.h"
#include "auxiliar_functions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>


#define false 0
#define true 1
#define PACK_SIZE 516

void pitcher( char* my_port, char* destiny_ip, char* destiny_port, list_t* list, int* list_lock, int* all_data_read );
void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port);
int make_socket(struct sockaddr_in* source);

#endif
