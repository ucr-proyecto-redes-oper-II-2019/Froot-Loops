#include <list>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <omp.h>
#include "my_functions.h"



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
  
    char* my_port;
    char* ip;
    char* other_port;
    char* file_name;
	std::ifstream file;
    bool file_read;
    char buffer_flag;
    char list_flag;
    struct sockaddr_in me;
    struct sockaddr_in other;
    char* read_data;
    std::list <char*> packages;
    omp_lock_t writelock1;
    omp_lock_t writelock2;
    int socket_fd;
	
  public:

    Sender(char* my_port,char* ip = nullptr, char* other_port = nullptr, char*file_name = nullptr ,char* filename = nullptr,
           bool file_read = false, char buffer_flag = '\0',char list_flag ='\0',
           char* read_data = new char[512],int socket_fd = 0)
    :my_port{my_port},
    ip{ip},
    other_port{other_port},
    file_name{file_name},
    file{filename, std::ios::in},
    file_read{file_read},
    buffer_flag{buffer_flag},
    list_flag{list_flag},
    read_data{read_data},
    socket_fd{socket_fd}
    {
        net_setup(&me,&other,my_port,ip,other_port);
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        omp_init_lock(&writelock1);
        omp_init_lock(&writelock2);
    }
	
	
	~Sender();

    void start_sending();

	int get_socket();
    char* get_read_data();
    void file_reader();
	

  
};



#endif // SENDER_H
