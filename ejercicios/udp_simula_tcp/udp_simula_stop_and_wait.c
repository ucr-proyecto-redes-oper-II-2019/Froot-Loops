/*
Grupo: Froot Loops
Integrantes:
Daniel Barrantes
Antonio Alvarez
Steven Barahona
*/
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
#include "list.h"
#include <time.h>
typedef struct timespec walltime_t;

#define false 0
#define true 1
#define PACKAGE_LIMIT 10
#define BUFF_SIZE 300
#define PACK_SIZE 516 //pack_size total, incluyendo headers
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)

int my_tcp_send(char* data_block, FILE* file);
int check_emptyness(char* str, int size);
void flush( list_t* list, int my_RN, int ack_RN );
int refresh_rn( list_t* list, int current_rn);


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

    if(argc < 7)//validacion de parametros
    {
        printf("No enough arguments given\n Usage: <my_IP> <my_port> <receptor_ip> <receptor_port> <file_path> <Role>\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in me, other;
    bzero(&me, sizeof(me));
    bzero(&other, sizeof(other));

    me.sin_family = AF_INET;
    me.sin_addr.s_addr = inet_addr(argv[1]);
    me.sin_port = htons(atoi(argv[2]));

    other.sin_family = AF_INET;
    other.sin_addr.s_addr = inet_addr(argv[3]); //IP destino se especifica en el 2 parametro de linea de comando
    other.sin_port = htons(atoi(argv[4]));

    pid_t cpid;
    cpid = fork();

    if(cpid == -1)
    {
        perror("Fork failed to create child lul \n");
        exit(EXIT_FAILURE);
    }

    if(cpid != 0 && atoi(argv[6]) == 0)//father process(sender)
    {
        printf("Holi soy sender\n");

        unsigned int len = sizeof(other);
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        bind(sockfd, (struct sockaddr *)&me, sizeof(me));

        char * data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos
        //char * package = malloc( sizeof(char) * PACK_SIZE ); //buffer compartido del paquete
        list_t list;
        list_init(&list);
        int SN = 0;
        int RN = 0;
        int last_package_read = false;
        int end_flag = false;
        int give_up_flag = 0;
        int wait_flag = true;

        #pragma omp parallel num_threads(4) shared(data_block, list, end_flag, last_package_read)
        {
            int my_thread_n = omp_get_thread_num(); //get my thread ID
            //char data_read[PACK_THROUGHPUT];

            if(my_thread_n == 0)
            {
                FILE * file;
                file = fopen(argv[5], "rb");
                end_flag = false;
                char tmp[PACK_THROUGHPUT];
                while(!end_flag)
                {

                    if(check_emptyness(data_block,PACK_THROUGHPUT) )
                    {
                        printf("Sender [0]: Había cambo en buffer compartido\n");
                        if(fread(tmp, PACK_THROUGHPUT, 1, file) > 0)
                        {

                            #pragma omp critical (shared_buffer)
                            {
                                my_strncpy(data_block,tmp,PACK_THROUGHPUT);
                            }

                            printf("Sender [0]: Pude escribir en buffer compartido\n");
                        }
                        else
                        {
                            end_flag = true;
                        }

                    }
                    usleep(200000);

                }

                fclose(file);
                sleep(1);
                last_package_read = true;

                printf("Sender: Thread 0 terminó de leer el archivo\n");

            }
            else if(my_thread_n == 1)
            {
                char * package = malloc( sizeof(char) * PACK_SIZE );
                union Data data;
                data.seq_num = 0;

                #pragma omp critical (data_block)
                {
                    while(!last_package_read)
                    {
                        #pragma omp critical (list)
                        {
                            if( !(check_emptyness(data_block, PACK_THROUGHPUT)) ) //si hay algo en el buffer compartido, lo empaqueto y libero el buffer
                            {
                                //empaquetamiento
                                package[0] = 0;//Indica que es un SN
                                strncpy(package+4, data_block, PACK_THROUGHPUT );
                                data.seq_num = SN;
                                my_strncpy( package+1, data.str, 3 );

                                printf("Sender[1]: El sq intentando de insertar es %d\n",data.seq_num);

                                int error_code = insert(&list, package);

                                while(error_code == -1 || error_code == -2)
                                {
                                    error_code = insert(&list, package);
                                    usleep(800000);
                                    printf("Sender[1]: Intentando de insertar %d\n",data.seq_num);
                                }

                                my_strncpy( data.str, list.recv_matrix[list.rear]+1, 3 );
                                SN++;
                                bzero(data_block, PACK_THROUGHPUT); //limpia el buffer para que thread 0 pueda volver a leer

                            }

                            if(end_flag) // si ya no hay mas por leer del archivo, se empaqueta el ultimo con el *
                            {
                                package[0] = 0;
                                package[4] = '*';
                                int error_code = insert(&list, package);
                                while(error_code == -1) //asegurarse que el paquete sea metido a la estructura de datos
                                {
                                    usleep(3000000);
                                    error_code = insert(&list, package);
                                }
                                last_package_read = true; // despues de agregar el paquete con * a la lista, termina
                            }

                        }
                    }
                }
                free(package);

            }
            else if(my_thread_n == 2)
            {
                //intentar de enviar los paquetes disponibles
                char * package = malloc( sizeof(char) * PACK_SIZE );
                union Data data;
                data.seq_num = 0;
                sleep(1);//Da tiempo al hilo 1 de poner al menos 1 elemento en la lista.

                while( !end_flag && !last_package_read /*&& !(is_empty(&list))*/ )
                {
					printf("jaja\n");
                    #pragma omp critical (sender_reciever)
                    {
                        for(int index = 0; index < list.size - 1; ++index) //intenta de enviar todos los paquetes actualmente en la lista sin eliminarlos de la misma
                        {
                            if(list.ack_array[index] == true) //si existe logicamente en la lista se envia
                            {
                                my_strncpy(package, list.recv_matrix[index], PACK_SIZE);
                                my_strncpy(data.str,list.recv_matrix[index] + 1 ,3);

                                printf("Sender[2] enviando package #%d por red\n",data.seq_num);
                                sleep(300000);
                                sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len);
                            }
                        }

                        int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len); // intenta recivir un paquete
                        if(check > 0 && package[0] == 1) //si el "kind" del package es un ack
                        {
							
                            my_strncpy(data.str, package+1, 3);
							printf("Sender[2] recibi ack #%d de receptor\n",data.seq_num);
							
                            printf("Sender[2] haciendo flush, antes en RN: %d hasta %d\n", RN,data.seq_num);
                            flush(&list, RN, data.seq_num); //flush rn's menores que el ack más reciente
                            RN = data.seq_num;

                            printf("Sender[2] despues de flush, RN: %d \n", RN);

                        }
                    }
                }
                
                //inicia el periodo de "rendirse"
                wait_flag = false;
                printf("Sender: About to give up in 60s \n");
                package[4] = '*';
                while(!give_up_flag)
                {
                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len);
                    usleep(500000);
                    
                    int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len);
                    if(check > 0)
                    {
                        if(package[4] == '*')
                        {
                            give_up_flag = true;
                        }
                    }
                    
                }//after 60s, give up :c
                free(package);
            }
            else if( my_thread_n == 3)
            {
                wait_flag = true;
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
        //free(str);

    }
    else if(atoi(argv[6]) == 1 && cpid == 0)//Child process start(reciever)
    {
        printf("Soi reciever jejeps\n");
        unsigned int len = sizeof(other);
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        bind(sockfd, (struct sockaddr *)&me, sizeof(me));

        char * data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos

        list_t list;
        list_init(&list);
        int RN = 0; //ack que estoy enviando
        int last_package_read = false;
        //int wait_flag = true;

        int continue_flag = false;
        int give_up_flag = false;


        #pragma omp parallel num_threads(4) shared(data_block, list, last_package_read)
        {
            int my_thread_n = omp_get_thread_num();

            if(my_thread_n == 0)
            {
                char * package = malloc( sizeof(char) * PACK_SIZE ); //buffer compartido del paquete
                union Data data;
                data.seq_num = 0;

                while(!last_package_read)
                {
                    int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len);

                    if(check > 0) //si capturo un paquete
                    {
                        #pragma omp critical (insert_packages)
                        {
                            my_strncpy(data.str,package+1,3); //recupero su seq_num
                            printf("Reciever[0] intentando de meter %d a la ventana\n",data.seq_num);

                            int error_code = insert(&list, package);

                            if(error_code == 0)//si se logró insertar ese paquete
                            {
                                printf("Reciever[0] logro meter paquete: %d\n", data.seq_num);
                                if(data.seq_num == RN)
                                {
                                    printf("Me llego el RN %d y estaba esperando el %d\n", data.seq_num, RN);
                                    RN = refresh_rn( &list, RN); //busca cual es el siguiente ack a pedir(basado en los continuos que ya tiene)

                                }
                            }
                        }
                    }
                    package[0] = 1;
                    data.seq_num = RN;
                    my_strncpy( package + 1, data.str, 3 );
                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len); //pido el ack correspondiente

                }

                continue_flag = true;
                printf("Reciever: About to give up in 60s \n");
                package[4] = '*';
                
                //giveup time (intentar de mandar el ack cierto tiempo)
                while(!give_up_flag)
                {
                    sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&other, len);
                    usleep(500000);
                    
                    int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&other, &len);
                    if(check > 0)
                    {
                        if(package[4] == '*')
                        {
                            give_up_flag = true;
                        }
                    }
                }//after 60s, i give up :c
                
                printf("Salí del juail\n");
                free(package);
            }
            else if(my_thread_n == 1)
            {
                #pragma omp critical (check_buffer)
                {
                    while(!last_package_read)
                    {
                        if(!is_empty(&list) && (check_emptyness(data_block,  PACK_THROUGHPUT))) //verifica que el buffer también este vacío
                        {


                            char* tmp = pop(&list);
                            my_strncpy( data.str, tmp+1, 3);
                            my_strncpy( data_block , tmp+4, PACK_THROUGHPUT ); //si la lisa no está vacia, entonces desempaqueta el de menor rn y lo mete al buffer compartido
                            printf("Reciever[1]: pude insertar en buffer compartido el paquete %d \n", data.seq_num);

                            if( data_block[0] == '*')
                            {
                                last_package_read = true;
                            }

                        }

                    }
                }

            }
            else if(my_thread_n == 2)
            {
                FILE* cpy_file;
                cpy_file = fopen(argv[5], "wr");

                #pragma omp critical (clean_buffer)
                {
                    while( !last_package_read && !check_emptyness(data_block, PACK_THROUGHPUT) )
                    {

                        printf("Reciever[2]: Pude leer del buffer compartido, escribiendo en archivo... \n");
                        if(!check_emptyness(data_block, PACK_THROUGHPUT))
                        {
                            fwrite(data_block, PACK_THROUGHPUT,1 , cpy_file);
                        }

                        bzero(data_block, sizeof(char));

                    }
                }

                fclose(cpy_file);

            }
            else if(my_thread_n == 3)
            {

                while(!continue_flag)
                {
                    sleep(1);
                }
                sleep(60);
                give_up_flag = true;
            }

            close(sockfd);
            destroy(&list);
            //free(data_block);
            //free(package);
        }
    }

    wait(NULL);
    return 0;
}

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
    while ( my_RN < ack_RN)
    {
        pop( list );
        my_RN++;
    }

}

//Funcion refresh_rn verifica segun el contenido de la lista cual ack debe pedir el receptor despues de recibir el que estaba esperando
int refresh_rn( list_t* list, int current_rn)
{
    int stop = false;
    int max = PACKAGE_LIMIT;
    int moved = 0;

    while(!stop && (moved < max))
    {
        current_rn++;
        if( list->ack_array[current_rn % PACKAGE_LIMIT] == false )
        {
            printf("Detuvo en RN %d, en la posición %d de la lista  \n", current_rn ,(current_rn % PACKAGE_LIMIT));
            stop = true;
        }
        moved++;
    }
    
    return (current_rn);
}
