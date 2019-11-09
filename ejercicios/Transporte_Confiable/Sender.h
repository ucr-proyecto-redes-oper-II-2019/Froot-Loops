#include <list>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <omp.h>



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
  
    int socket;
	std::ifstream file;
    bool file_read;
    char buffer_flag;
    char list_flag;
    struct sockaddr_in me, other;
    char* read_data;
    std::list <char*> packages;
    omp_lock_t writelock1;
    omp_lock_t writelock2;
	
  public:

	/*Sender(int socket = 0)
	: socket{socket}{};*/
	//Sender(){};
    Sender(int socket = 0,std::ifstream file, char* read_data = new char[512])
	:socket{socket},
    file(filename),
    read_data{read_data}
    {
        omp_init_lock(&writelock1);
        omp_init_lock(&writelock2);
    }
	
	
	~Sender();

    void start_sending();

	int get_socket();
    char* get_read_data();
	

  
};



#endif // SENDER_H
