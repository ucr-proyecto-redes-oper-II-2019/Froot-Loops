/*
Grupo: Froot Loops
Integrantes:
Daniel Barrantes
Antonio Alvarez
Steven Barahona

Programa encargado de enviar un jpg por paquetes de 516B
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

#define false 0
#define true 1
#define PACK_SIZE 516 //pack_size total, incluyendo headers
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)

int check_emptyness(char* str, int size);
void flush( list_t* list, int my_RN, int ack_RN );
int refresh_rn( list_t* list, int current_rn);

int main(int argc, char* argv[]) //argv[1] = <my_port>, argv[2] = <destiny_ip>, argv[3] = <destiny_port> argv[4] = file_path
{
  //programación preventiva, se asegura que se hayan ingresado los parámetros necesarios
  if(argc < 4)//validacion de parametros
  {
    printf("No enough arguments given\n Usage: <my_port>, argv[2] = <destiny_ip>, argv[3] = <destiny_port> argv[4] = file_path\n");
    return EXIT_FAILURE;
  }

  list_t list;// la lista que servirá como ventana del emisor
  list_init(&list); // se inicializa sa lista
  char* data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos
  int last_package_read = false; //bandera compartida que indica que el último paquete se leyó
  int SN, RN;
  int all_data_read = false;
  int wait_flag =false;
  int give_up_flag = false;

  //-----------------------------inicio de la sección paralela----------------------------------//

  #pragma omp parallel num_threads(4) shared(data_block, last_package_read, all_data_read, list, wait_flag)
  {
    int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
    //--------------------------------- inicio hilo 0 ------------------------------------------//
    /*
    el hilo 0 se encarga de traer los bloques de datos y ponerlos en el data_block para
    que el hilo 1 lo empaquete y lo añada a la ventana del emisor
    */
    if (my_thread_n == 0)
    {
      //abre el archivo y chequea que se abriera de  manera correcta
      FILE* file = fopen(argv[4], "rb");
      if (file == NULL)
      {
        perror("Sender[0]: Error opening file: ");
      }
      int all_data_read = false;//bandera que indica si toda la imagen fue leída
      char tmp[PACK_THROUGHPUT]; //buffer temporal para escribir en data_block
      /*
      este es el ciclo principal del hilo 0, es el que se encarga de leer toda la imagen,
      la condición de salida es que ya no se pueda leer nada de la imagen
      */
      while(!all_data_read)
      {
        //hay que escribir siempre que data_block esté vacío
        while( check_emptyness(data_block,PACK_THROUGHPUT) )
        {
          if(fread(tmp, PACK_THROUGHPUT, 1, file) > 0) //si se logran leer 512B, escriba en data_block
          {
            //el acceso al data_block debe ser crítico
            #pragma omp critical (data_block) //el nombre la sección crítica es MUY importante
            {
              my_strncpy(data_block,tmp,PACK_THROUGHPUT);
            }
            printf("Sender[0]: Pude escribir en data block\n");
          }
          else //si ya no se puede leer del archivo, todos los datos fueron leídos
          {
            all_data_read = true;
          }

        }
        usleep(200000); //se le da un chance al hilo 1 para que consuma el data_block

      }
      fclose(file); //ya se leyó todo el archivo en este punto
      last_package_read = true; //se prende la bandera compartida
    }
    //------------------------------------fin del hilo 0 -----------------------------------------//

    //-----------------------------------inicio del hilo 1 ---------------------------------------//
    /*
    el hilo 1 se encarga de empaquetar (poner el número de secuencia correcto en big endian)
    y además colocar el paquete en la ventana para que el hilo 2 pueda enviarlo por capa de transporte
    */
    else if (my_thread_n == 1)
    {
      char* package = calloc( 1, sizeof(char)* PACK_SIZE );// se crea el contenedor del paquete de 516B
      union Data data;
      data.seq_num = 0;
      SN = 0;

      while(!all_data_read) //hasta que el hilo 0 ya no pueda leer más (se acabó el archivo)
      {
        //esta sección critica bloquea la escritura en el data_block que se realiza en el hilo 0
        #pragma omp critical (data_block)
        {
          //verificamos que el bloque no esté vació (por ende podemos leer de él y limpiarlo)
          if( !( check_emptyness(data_block, PACK_THROUGHPUT) ) )
          {
            //-----------------empaquetamiento------------------//
            package[0] = 0; //se especifica que es un envío
            my_strncpy( package+4, data_block, PACK_THROUGHPUT ); //se copian los 512 leídos del archivo por el hilo 0
            data.seq_num = SN;//htonl(SN);//se guarda el SN en BIG ENDIAN en el data.seq_num
            my_strncpy( package+1, data.str, 3 );//se guardan los primeros 3B del seq_num en el paquete
            //-----------------empaquetamiento------------------//

            //intenta de meter el paquete hasta que obtenga error code = success
            printf("Sender[1]: El sq intentando de insertar es %d\n",data.seq_num);
            int error_code = insert( &list, package);
            while(error_code == INSERT_FAILURE || error_code == INSERT_FAIL_REPEATED)
            {
              error_code = insert(&list, package);
              usleep(800000);
              printf("Sender[1]: Intentando de insertar %d\n",data.seq_num);
            }
            SN++;
            bzero(data_block, PACK_THROUGHPUT);
          }

          //luego de escribir en el data_block, se pregunta si el hilo 0 ya leyó todo el archivo
          if (all_data_read) //si ya se leyó todo el archivo, se añade el paquete final a la lista
          {
            package[0] = 0;
            package[4] = '*';
            int error_code = insert(&list, package);
            while(error_code == INSERT_FAILURE) //asegurarse que el paquete sea metido a la estructura de datos
            {
                usleep(3000000);
                error_code = insert(&list, package);
            }
            last_package_read = true; 

          }

        }
        usleep(200000);//usleep para darle chance al hilo 0 de que escriba en el data_block

      }
      free(package);

    }
    //--------------------------------------fin del hilo 1 ---------------------------------------//
    
  	//------------------------------------inicio del hilo 2---------------------------------------//
    /*
     el hilo 2 se encarga de enviar los paquetes por la red y recibe los ack's 
     */
    else if (my_thread_n == 2)
    {

      char* package = calloc( 1, sizeof(char)* PACK_SIZE );// se crea el contenedor del paquete de 516B
      union Data data;
      data.seq_num = 0;
      //---------------definición de direcciones-----------//
      struct sockaddr_in source, dest;
      bzero(&source, sizeof(source));
      bzero(&dest, sizeof(dest));

      source.sin_family = AF_INET;
      source.sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
      source.sin_port = htons(atoi(argv[1]));//Mi puerto donde estoy escuchando

      dest.sin_family = AF_INET;
      dest.sin_addr.s_addr = inet_addr(argv[2]); //IP destino se especifica en el 2do parametro de linea de comando
			dest.sin_port = htons(atoi(argv[3])); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando
      
      //-----------------creación del socket-----------------//
      int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

      //Se enlaza el socket sockfd a la estructura de datos source
      int bind_return = bind(sockfd, (struct sockaddr *)&source, sizeof(source));
      
      unsigned int len = sizeof(dest);
      
      while( !(last_package_read) && !(all_data_read) && !(is_empty(&list)) )
      {
        
        #pragma omp critical (data_block)
        {
          //se intentan de enviar todos los paquetes actualmente en la lista
          for(int index = 0; index < list.size - 1; ++index) //intenta de enviar todos los paquetes actualmente en la lista sin eliminarlos de la misma
          {
            if(list.ack_array[index] == true) //si existe logicamente en la lista se envia
            {
              my_strncpy(package, list.recv_matrix[index], PACK_SIZE);
              
              my_strncpy(data.str,list.recv_matrix[index] + 1 ,3);
              printf("Sender[2] enviando package #%d por red\n",data.seq_num);
              usleep(300000);
              
              sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&dest, len);
            }
          }
          
          //se recupera un paquete y se extrae el ACK          
          int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&dest, &len);
          if( check > 0 && package[0])
          {
            my_strncpy(data.str, package+1, 3);
            
            printf("Sender[2] recibi ack #%d de receptor\n",data.seq_num);
            flush(&list, RN, data.seq_num); 
            //flush rn's menores que el ack más reciente, para enviar desde el ack recibido en adelante
            RN = data.seq_num;
            printf("Sender[2] despues de flush, RN: %d \n", RN);
            
          }
          
        }
        
      }
      
      //-----------------------------------periodo de rendirse---------------------------------------//
      wait_flag = false;
      printf("Sender: About to give up in 60s \n");
      
      //empaquetamos el último paquete
      package[4] = '*';     
      data.seq_num = RN;      
      my_strncpy( package+1, data.str, 3 );
      
      while(!give_up_flag)
      {
        sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&dest, len);
        usleep(500000);
                   
        int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&dest, &len);
        if(check > 0)
        {
          if(package[4] == '*')
          {
            give_up_flag = true;
          }
        }
                    
      }//after 60s, give up :c
      free(package);
      close(sockfd); 
    }
    //-------------------------------------fin del hilo 2-----------------------------------------//
    //-------------------------------------inicio del hilo 3-----------------------------------------//
    else if(my_thread_n == 3)
    {
      wait_flag = true;
      while(wait_flag)
      {
        sleep(1);
      }
      sleep(60);
      give_up_flag = true;
    }
    //-------------------------------------inicio del hilo 3-----------------------------------------//
  }
  //----------------------------------fin de la sección paralela ---------------------------------//
  return 0;
}





//----------------funciones(subrutinas) utilizadas en el programa del emisor----------------------//
/*
check_emptyness es una función utilizada para ver si el data_block está
vacío y por ende disponible para ser escrito
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

void flush( list_t* list, int my_RN, int ack_RN )
{
    while ( my_RN < ack_RN)
    {
        pop( list );
        my_RN++;
    }
}

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
