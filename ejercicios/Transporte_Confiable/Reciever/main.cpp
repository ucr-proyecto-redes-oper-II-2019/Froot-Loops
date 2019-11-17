#include <cstring>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <omp.h>
#include <iostream>
#include <fstream>


#define SEND 0
#define PACK_THROUGHPUT 512
#define PACK_SIZE 516

union Data
{
  int seq_num;
  char str[4];
}data;

char* my_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
        dest[i] = src[i];

    return dest;
}


int main(int argc, char** argv)
{
	struct sockaddr_in me;
	struct sockaddr_in other;
	int pack_count_dummy = 0;
	char package[PACK_SIZE];
	std::ofstream file;
	
	bzero(&me, sizeof(me));
	bzero(&other, sizeof(other));
	
	socklen_t recv_size = sizeof(other);
   
	
	
	if(argc < 1)
	{
		std::cout << "No enough arguments, usage: ./sender PORT" << std::endl;
	}
	
	me.sin_family = AF_INET;
	me.sin_addr.s_addr = INADDR_ANY;
	me.sin_port = htons(atoi(argv[1]));
	
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	bind(socket_fd,(struct sockaddr*)&me,sizeof(me));
	
    bool flag = false;

	while(!flag)
    {
							//readB = recvfrom(sockfd, message, sizeof(message), MSG_DONTWAIT, (struct sockaddr*)&other, &len);
         ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &recv_size);
         
         if(bytes_received > 0)
         {
			 
			std::cout << "Primer byte " << (int)package[0] << std::endl;
			 
            my_strncpy(data.str, package+1, 3);
            std::cout << "Data seq: " << data.seq_num << std::endl;
            
            if(package[4] == '*')
				flag = true;
             
            if(data.seq_num == pack_count_dummy)
            {
				std::cout << "Entré al if " << std::endl;
				if(pack_count_dummy == 0)
				{
					char dummy_name[50];
					//my_strncpy( dummy_name, package+4, 50 );
					strcpy(dummy_name,package+4);
					
					file.open(dummy_name);
					std::cout << "Nombre Archivo: " << dummy_name << std::endl;
					file.write( package+54, 462 );
				}
				else
				{
					file.write( package+4, PACK_THROUGHPUT );
				}
				
				pack_count_dummy++;
				
				package[0] = 1;
				data.seq_num = pack_count_dummy;
				my_strncpy( package+1, data.str, 3 );         
				 
				int bytes_send = sendto(socket_fd,package,PACK_SIZE,0, (struct sockaddr*)&other, recv_size);
				std::cout << "Ack enviado con " << bytes_send << std::endl;
			}

         }

    }
    
    file.close();
    
    std::cout << "Terminé" << std::endl;
	
	return 0;
}
