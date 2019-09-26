#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <omp.h>
#include "queue.c"
#include <time.h>
typedef struct timespec walltime_t;

#define false 0
#define true 1
#define BUFF_SIZE 300
#define PACK_SIZE 516 //pack_size total, incluyendo headers
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)

int my_tcp_send(char* data_block, FILE* file);
int check_emptyness(char* str, int size);
void flush( list_t* list, int my_RN, int ack_RN );


/*
walltime_t start;
clock_gettime(CLOCK_MONOTONIC, &start);

//Cosas

walltime_t finish;
clock_gettime(CLOCK_MONOTONIC, &finish);
double elapsed = (finish.tv_sec - start.tv_sec);
elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
printf("Time elapsed %.9lfs \n",elapsed);
*/

int main(int argc, char* argv[])//agrg[1] = my_IP, argv[2] = my_port, argv[3] = receptor_ip, argv[4] = receptor_port, argv[5] = file_path
{
    struct sockaddr_in father, child;
    bzero(&father, sizeof(father));
    bzero(&child, sizeof(child));

    father.sin_family = AF_INET;
    father.sin_addr.s_addr = inet_addr(argv[1]);
    father.sin_port = htons(atoi(argv[2]));

    child.sin_family = AF_INET;
    child.sin_addr.s_addr = inet_addr(argv[3]); //IP destino se especifica en el 2 parametro de linea de comando
    child.sin_port = htons(atoi(argv[4]));



    pid_t cpid;
    cpid = fork();

    if(cpid == -1)
    {
        perror("Fork failed to create child lul \n");
        exit(EXIT_FAILURE);
    }

    if(cpid != 0)//father process(sender)
    {

        char * data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos
        char * package = malloc( sizeof(char) * PACK_SIZE ); //buffer compartido del paquete
        list_t list;
        list_init(&list);
        int SN = 0;
        int RN = 0;
        int last_package_read = false;
        int end_flag = false;
        int continue_flag = 0;
        int give_up_flag = 0;
        int wait_flag = true;
        union Data data;

        int len = sizeof(child);

        // create datagram socket
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        // bind created socket to my addres
        bind(sockfd, (struct sockaddr *)&father, sizeof(father));

		#pragma omp parallel num_threads(4) shared(data_block, list)
        {
            int my_thread_n = omp_get_thread_num(); //get my thread ID
            //char data_read[PACK_THROUGHPUT];

            if(my_thread_n == 0)
            {
                FILE * file;
                file = fopen(argv[5], "rb");
                int write_flag = false;


                end_flag = false;
                char tmp[PACK_THROUGHPUT];
                while(!end_flag)
                {
                    write_flag = false;

                    if( fread(tmp, PACK_THROUGHPUT, 1, file) > 0)
                    {
                        while(!write_flag)
                        {
                            if(check_emptyness(data_block,PACK_THROUGHPUT))
                            {
								#pragma omp critical (shared_buffer)
                                {
                                    strncpy(data_block,tmp,PACK_THROUGHPUT);
                                }
                                write_flag = true;
                            }
                        }
                    }
                    else
                    {
                        end_flag = true;
                    }
                }

                fclose(file);
                last_package_read = true;

            }
            else if(my_thread_n == 1)
            {

                while(!last_package_read)
                {

					#pragma omp critical (list_queuer)
                    {
                        if( !(check_emptyness(data_block, PACK_THROUGHPUT)) )
                        {

                            strncpy(package+4, data_block, PACK_THROUGHPUT );
                            bzero(data_block, PACK_THROUGHPUT);

                            //empaquetamiento
                            package[0] = 0;//Indica que es un SN

                            data.seq_num = SN;
                            strncpy( package+3, data.str, 3 );

                            while(insert(&list, package) == -1)//asegurarse que el paquete sea metido a la estructura de datos
                            {
                                usleep(5000000);
                            }
                            SN++;
                        }

                        if(end_flag)
                        {
                            package[0] = 0;
                            package[4] = '*';
                            while(insert(&list, package) == -1)//asegurarse que el paquete sea metido a la estructura de datos
                            {
                                usleep(5000000);
                            }
                            //insert(&list, package);
                            last_package_read = true;
                        }

                    }
                }


            }
            else if(my_thread_n == 2)
            {
                //intentar de enviar los paquetes disponibles
                int end_flag = 0;

                while(!end_flag && !last_package_read && ( list.front != -1 ) )
                {

					#pragma omp critical (sender_reciever)
                    {
                        for(int index = list.front; index < list.rear; ++index) //envía todos los paquetes actualmente en la lista sin eliminarlos de la misma
                        {
                            strncpy(package, list.recv_matrix[index], PACK_SIZE);
                            sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&child, len);
                        }

                        int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&child, &len); //recivir un ack
                        if(check)
                        {
                            strncpy(data.str, package+3, 3);
                            flush(&list, RN, data.seq_num); //flush rn's menores que el ack más reciente
                            RN = data.seq_num;
                        }
                    }
                }
                wait_flag = false;

                printf("Sender: About to give up in 60s \n");
                package[4] = '*';
                while(!give_up_flag)
                {
                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&child, len);
                    usleep(500000);
                }//after 60s, give up :c

            }
            else if( my_thread_n == 3)
            {
                while(wait_flag)
                {
                    sleep(1);
                }
                sleep(60);
                give_up_flag = true;

            }
        }
        //end of parallel region

        close(sockfd);
        destroy(&list);
        free(data_block);
        free(package);

    }
    else//Child process start(reciever)
    {

        char * data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos
        char * package = malloc( sizeof(char) * PACK_SIZE ); //buffer compartido del paquete
        list_t list;
        list_init(&list);
        int RN = 0; //ack que estoy enviando
        int last_package_read = false;
        union Data data;
        int continue_flag = 0;
        int give_up_flag = 0;


        int len = sizeof(father);
        // create datagram socket
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        // bind created socket to my address
        bind(sockfd, (struct sockaddr *)&child, sizeof(child));

		#pragma omp parallel num_threads(4) shared(data_block, list)
        {
            int my_thread_n = omp_get_thread_num();

            if(my_thread_n == 0)
            {
                while(!last_package_read)
                {
                    int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&father, &len);

                    if(check > 0)
                    {
						#pragma omp critical (insert_packages)
                        {
                            insert(&list, package);
                        }

                        strncpy( data.str, package+3, 3 );
                        RN = data.seq_num+1;
                    }

                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&father, len);
                }

                give_up_flag = false;

                printf("Reciever: About to give up in 60s \n");
                package[4] = '*';
                while(!give_up_flag)
                {
                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&child, len);
                    usleep(500000);
                }//after 60s, i give up :c

                //giveup time (intentar de mandar el ack cierto tiempo)
            }
            else if(my_thread_n == 1)
            {

                while(!last_package_read)
                {
                    if(list.front != -1 && (check_emptyness(data_block,  PACK_THROUGHPUT))) //verifica que el buffer también este vacío
                    {
                        char* tmp = pop(&list);
						#pragma omp critical (check_buffer)
                        {
                            strncpy( data_block , tmp+4, PACK_THROUGHPUT ); //si la lisa no está vacia, entonces desempaqueta el de menor rn
                        }
                        if( data_block[0] = '*')
                        {
                            last_package_read = true;
                        }

                    }

                }
            }
            else if(my_thread_n == 2)
            {
                FILE* cpy_file;
                cpy_file = fopen("larry.jpg", "wr");

                while( !last_package_read && !check_emptyness(data_block, PACK_THROUGHPUT) )
                {
                    if(!check_emptyness(data_block, PACK_THROUGHPUT))
                    {
                        fwrite(data_block, PACK_THROUGHPUT,1 , cpy_file);
                    }
					#pragma omp critical (clean_buffer)
                    {
                        bzero(data_block, sizeof(char));
                    }
                }

                fclose(cpy_file);

            }
            else if(my_thread_n == 3)
            {

                while(!give_up_flag)
                {
                    sleep(1);
                }
                sleep(60);
                give_up_flag = true;
            }

            close(sockfd);
            destroy(&list);
            free(data_block);
            free(package);
        }
    }

    wait(NULL);
    return 0;
}
/*
int my_tcp_recieve(char* data_block, FILE* file)
{
    if(check_emptyness(data_block,PACK_THROUGHPUT) && !file_end)
    {
        fwrite(data_block,PACK_THROUGHPUT,1,file);
    }
}

int my_tcp_send(char* data_block, FILE* file)
{
    int file_end = 0;

    if(check_emptyness(data_block,PACK_THROUGHPUT) && !file_end)
    {
        file_end = fread(data_block, PACK_THROUGHPUT, 1, file);
    }

    return file_end;

}
*/
int check_emptyness(char* str, int size)
{
    for( int index = 0; index < size; ++index)
    {
        if( str[index] != 0)
            return false;
    }
    return true;
}

//Función flush bota de la lista los packages con secuencia menor al ack recibido
void flush( list_t* list, int my_RN, int ack_RN )
{
    while ( my_RN < ack_RN )
    {
        pop( list );
        my_RN++;
    }
    //jajasalu2
}
