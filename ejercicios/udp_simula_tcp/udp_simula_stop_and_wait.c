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
#include "queue.h"

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
    
    pid_t cpid; //create child id
    
    char package[PACK_SIZE];
    int sockfd, sockfd2, n, len, current_batch_size;
    int RN = 0;
    int SN = 0;
    struct sockaddr_in me, other;
    int end_flag = 0;
    
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
    len = sizeof(other);
    
    cpid = fork();
    if(cpid == -1)
    {
        perror("Fork failed to create child lul \n");
        exit(EXIT_FAILURE);
    }
    
    
    //parent (emisor)
    if(cpid != 0)
    {
        
        other.sin_addr.s_addr = inet_addr(argv[3]); //IP destino se especifica en el 2 parametro de linea de comando
        other.sin_port = htons(atoi(argv[4]));
        
        //queue_t q;
        //q.init();
        
        while(!end_flag)
        {
            package[0] = 0; //indicar que es un SN
           
            fseek( file, SN*10*512, SEEK_SET);
            
            for(int index = 0; index < 10; ++index)
            {
                package[0] = 0;
                package[3] = index;
                
                if(fread( package+4, 512, 1, file) > 0)
                {
                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len);
                }
                else
                {
                    end_flag = 1;
                }
            }      
            
            usleep(800000); //time-out simulation
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
                
				//manejo de cola
				if( SN < seq_num )
				{
					SN = seq_num;
				}
            }
            


            usleep(400000);

            
        }
        printf("Salí del ciclo\n");
    }
    
    //child (reciever)
    if(cpid == 0)
    {
        
        FILE* copy_file;
        copy_file = fopen("payaso.jpg" , "wr" );
        
        other.sin_addr.s_addr = inet_addr(argv[3]); //IP destino se especifica en el 2 parametro de linea de comando
        other.sin_port = htons(atoi(argv[4]));
        
        queue_t q;
        queueInit(&q);
        
        while(!end_flag)
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
                    
                    //RN es el número de secuencia que el receptor está esperando
                    //SN es el número de secuencia que el emisor está enviando actualmente
                    if( seq_num <= RN + PACKAGE_LIMIT-1 )
                    {
                        enQueue(&q ,package);
                    }	
                    
                    //strncpy(recv_matrix[seq_num],package+4,PACK_THROUGHPUT);//Dentro de la cola (hacer copia de los 512 bytes)
                    
                    strncpy(package, queueFirst(&q), 516);
                    if( seq_num == RN )
                    {
                        fwrite( queueFirst(&q)+4, 1 , PACK_THROUGHPUT , copy_file );
                        deQueue(&q);
                        ++RN;
                    }
                    //q.first()->[0] = 1;
                    package[0] = 1;//decir que es un RN
                    sendto( sockfd, package ,PACK_SIZE, 0, (struct sockaddr*)&other, len );
                }
            } 	
            
            if(  queueFirst(&q)[516] == '*' ) //el último byte del jpg nunca puede ser '*', es el token de finalizar comunicación
            {
                end_flag = true;
            }
            
            usleep(400000);
            
        }
        
        
        
    }
    
    close(sockfd);
    
    return 0;
    
}
