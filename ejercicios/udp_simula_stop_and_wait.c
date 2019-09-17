#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define PORT 5000
#define MAXLINE 1000
#define false 0
#define true 1
#define BUFF_SIZE 300
#define PACK_SIZE 516 //pack_size total, incluyendo headers
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)
#define PACKAGE_LIMIT 10 //package batch limit

// Driver code
int main(int argc, char* argv[]) //agrg[1] = my_IP, argv[2] = my_port, argv[3] = receptor_ip, argv[4] = receptor_port, argv[5] = file_path
{
	
	if(argc < 6)
	{
		printf("Not enough arguments given\n Usage: ./client <my_IP> <my_port> <receptor_id> <receptor_port> <file_path> \n");
		return 0;
	}
	
	FILE* file; 
	
	file = fopen(argv[5], "rb");
	/*
	char* ip;
	char* send;
	char* pch;
	char* other_port;
	*/
	char** recv_matrix;
	pid_t cpid; //create child id
	int pipefd[2];
	int batch_counter;
  int total_package_counter = 0;
  int total_packages_sent = 0;
	int ack_chars[PACKAGE_LIMIT];	
	bzero(ack_chars,PACKAGE_LIMIT);
	char package[PACK_SIZE];
	int sockfd, sockfd2, n, len, current_batch_size;
	struct sockaddr_in me, other;
	int end_flag = 0;
	
	// clear servaddr
	// Limpia el registro
	bzero(&me, sizeof(me));

	me.sin_addr.s_addr = inet_addr(argv[1]);
	me.sin_port = htons(PORT);
	me.sin_family = AF_INET;
	
	other.sin_family = AF_INET;
	
	// create datagram socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	// bind created socket to my addres
	bind(sockfd, (struct sockaddr *)&me, sizeof(me));

	int readB = 0;
	len = sizeof(other);
	
	if(pipe(pipefd) == -1)
	{
		perror("Could not create pipe\n");
		exit(EXIT_FAILURE);
	}
	
  current_batch_size = PACKAGE_LIMIT;
    
	cpid = fork();
	
	if(cpid == -1)
	{
		perror("Fork failed to create child lul \n");
		exit(EXIT_FAILURE);
	}
	
	
	//parent (emisor)
	if(cpid != 0)
	{
      
    struct stat buffer;
    int status;
    status = stat("path to file", &buffer);
    if(status == 0) {
      printf("File read succesfully: %zu \n", buffer.st_size);
         
      total_package_counter = buffer.st_size / PACK_THROUGHPUT;
    }

		
		other.sin_addr.s_addr = inet_addr(argv[3]); //IP destino se especifica en el 2 parametro de linea de comando
		other.sin_port = htons(atoi(argv[4]));
		
		while(!end_flag)
		{
			package[0] = 0;
			
			fseek( file, batch_counter*10*512, SEEK_SET);
			
			int confirmed = 0;//cantidad de paquetes ya recividos(confirmados)
			
			for(int index = 0; index < current_batch_size; ++index)
			{
								
				if( ack_chars[index] == 0 )//if ack recv hasn't arrived
				{
					char* tmp;
					tmp = (char*)(&index);
					package[1] = tmp[1];
					package[2] = tmp[2];
					package[3] = tmp[3];
						
					if(fread(package+4, 512, 1, file) < 1)
					{
						end_flag = 1;
					}
					
					sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len);
					
					
				}else{//if ack confirmation was recieved
					
					fseek( file, batch_counter*index*512, SEEK_SET);
					++confirmed;	
				}	
        
        recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len);
        if(package[0] == 1) 
        {
        		int seq_num;
         		char tmp[4];
            tmp[0] = 0;
        		tmp[1] = package[1];
          	tmp[2] = package[2];
          	tmp[3] = package[3];
          	seq_num = (int)tmp;
          	
          	ack_chars[seq_num] = 1;
        }
        
        
			}			

			if(confirmed == current_batch_size)
			{
				batch_counter++;
        bzero(ack_chars, sizeof(int));
        total_packages_sent += current_batch_size;
			}
     
      
      if( total_packages_sent == (total_package_counter - total_package_counter % 10 )  )//cuando solo faltan total_packages %10 por enviar,se envian los restantes de forma separada
      {
        current_batch_size = total_package_counter % 10;
      }
      
      if( (current_batch_size != 10) && (total_packages_sent == total_package_counter))
      {
        end_flag = true;
        package[0] = 0;
        package[PACK_SIZE-1] = '*'; //PREGUNTAR
        sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len);
      }

			usleep(500000);
		}
		
    
		printf("Salí del ciclo\n");
	}
  
	//child (reciever)
	if(cpid == 0)
	{
    
    other.sin_addr.s_addr = inet_addr(argv[3]); //IP destino se especifica en el 2 parametro de linea de comando
		other.sin_port = htons(atoi(argv[4]));
				
		recv_matrix = (char** )calloc(PACKAGE_LIMIT, sizeof(char* ));
    

    while(!end_flag)
		{
      
        for(int index = 0; index < 10; ++index)
        {
          	int check_recv = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len);
            if(check_recv > 0)
            {
              	if(package[0] == 0)
                {
                  	int seq_num;
                    char tmp[4];
                  	tmp[0] = 0;
                    tmp[1] = package[1];
                    tmp[2] = package[2];
                    tmp[3] = package[3];
                    seq_num = (int)tmp;

                    ack_chars[seq_num] = 1;
                  
                  	strncpy(recv_matrix[seq_num],package+4,PACK_THROUGHPUT);
                  	
                }
            }
          		
        }
      
    }
    
		/*
		other.sin_addr.s_addr = inet_addr(ip); //IP destino se especifica en el 2 parametro de linea de comando
		other.sin_port = htons(atoi(other_port));
				
		recv_matrix = (char** )calloc(PACKAGE_LIMIT, sizeof(char* ));
		while(!end_flag)
		{
			
			recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len);
			
			int package_position = package[3]; //order of the package in the 10 package batch
			
			strncpy( recv_matrix[package_position], package+4, 512 );
			
			ack_chars[package_position] = 1;	
 
			sendto(sockfd, send, MAXLINE, 0, (struct sockaddr*)&other, len);
			
			
				
			usleep(300000);
		}*/
	}

	close(sockfd);

}
