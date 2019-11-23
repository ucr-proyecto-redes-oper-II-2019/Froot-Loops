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
    char mi_paquete[50];
    std::ofstream file;

    bzero(&me, sizeof(me));
    bzero(&other, sizeof(other));

    socklen_t recv_size = sizeof(other);



    if(argc < 2)
    {
        std::cout << "No enough arguments, usage: ./sender PORT" << std::endl;
        return 0;
    }

    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = htons(atoi(argv[1]));

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    bind(socket_fd,(struct sockaddr*)&me,sizeof(me));

    bool flag = false;

    while(!flag)
    {
        ssize_t bytes_received = recvfrom(socket_fd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &recv_size);

        if(bytes_received > 0)
        {


            my_strncpy(data.str, package+1, 3);

            char seq[sizeof(int)] = {0, 0, 0, 0};
            memcpy(seq+1, data.str  /*buffer que recibimos con el numero de sequencia de 3 bytes*/, 3);
            int result = ntohl( *((int*) seq) );
            //std::cout << "Paquete recibido #: " << result << std::endl;

            if(result == pack_count_dummy)
            {

                if(package[4] == '*')
                {
                    flag = true;
                }

                if(pack_count_dummy == 0)
                {
                    char dummy_name[50];
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

                int network_sequence = htonl( data.seq_num );
                my_strncpy( package+1, ((char*) &network_sequence+1), 3 );

                int bytes_send = sendto(socket_fd,package,PACK_SIZE,0, (struct sockaddr*)&other, recv_size);
                //std::cout << "Ack enviado con #: " << network_sequence << " y # bytes " << bytes_send << std::endl;
            }
        }
        //usleep(500000);
    }
    
    for(int i = 0; i < 5; ++i)
    {
        usleep(5000);
        //sleep(1);
        sendto(socket_fd,package,PACK_SIZE,0, (struct sockaddr*)&other, recv_size);
    }
    
    file.close();
    
    std::cout << "Terminé" << std::endl;

    return 0;
}
