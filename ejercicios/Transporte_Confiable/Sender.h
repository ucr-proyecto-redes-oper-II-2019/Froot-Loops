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
#include <cstring>
#include <stdlib.h>


union Data
{
  int seq_num;
  char str[4];
}data;

#define SEND 0
#define PACK_THROUGHPUT 512



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
    char* package; //buffer usado para enviar y recibir paquetes
    std::list <char*> packages;
    omp_lock_t writelock1;
    omp_lock_t writelock2;
    int socket_fd;
    int SN;
    int RN;
	
  public:

    Sender(char* my_port,char* ip = nullptr, char* other_port = nullptr, char*file_name = nullptr ,char* filename = nullptr,
           bool file_read = false, char buffer_flag = 'L',char list_flag ='I',
           char* read_data = new char[512],char* package = new char[512],int socket_fd = 0, int SN = 0, int RN = 0)
    :my_port{my_port},
    ip{ip},
    other_port{other_port},
    file_name{file_name},
    file{filename, std::ios::in},
    file_read{file_read},
    buffer_flag{buffer_flag},
    list_flag{list_flag},
    read_data{read_data},
    package{package},
    socket_fd{socket_fd},
    SN{SN},
    RN{RN}
    {
        net_setup(&me,&other,my_port,ip,other_port);
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        omp_init_lock(&writelock1);
        omp_init_lock(&writelock2);
    }
	
	
	~Sender();
	
	void packer();

    char* my_strncpy(char *dest, const char *src, int n);

    void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port);
    char* make_pakage(char *data_block);


    void start_sending();

	int get_socket();
    char* get_read_data();
    void file_reader();
	
	void send_package_receive_ack(struct sockaddr_in* source, struct sockaddr_in* dest);
    void flush(int my_RN, int ack_RN);
};



#endif // SENDER_H
