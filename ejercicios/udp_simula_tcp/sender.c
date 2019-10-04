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
#include "list.c"
#include <time.h>

#define false 0
#define true 1
#define BUFF_SIZE 300
#define PACK_SIZE 516 //pack_size total, incluyendo headers
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)

int main(int argc, char* argv[]) //argv[1] = mi_IP, argv[2] = my_port, argv[3] = file_path
{
  //programación preventiva, se asegura que se hayan ingresado los parámetros necesarios
  if(argc < 4)//validacion de parametros
  {
    printf("No enough arguments given\n Usage: <my_IP> <my_port> <file_path>\n");
    return EXIT_FAILURE;
  }

  char* data_block = calloc( 1, sizeof(char)* PACK_THROUGHPUT ); //buffer compartido de datos
  char* package = malloc( sizeof(char) * PACK_SIZE ); //buffer compartido del paquete
  int last_package_read = false; //bandera compartida que indica que el último paquete se leyó

  //-----------------------------inicio de la sección paralela----------------------------------//

  #pragma omp parallel num_threads(4) shared(data_block)
  {
    int my_thread_n = omp_get_thread_num(); //obtiene el identificador del hilo
    //--------------------------------- inicio hilo 0 -----------------------------------------//
    /*
    el hilo 0 se encarga de traer los bloques de datos y ponerlos en el data_block para
    que el hilo 1 lo empaquete y lo añada a la ventana del emisor
    */
    if (my_thread_n == 0)
    {
      //abre el archivo y chequea que se abriera de  manera correcta
      FILE* file = fopen(argv[3], "rb");
      if (file == NULL)
      {
        perror("Error opening file: ");
      }
      int all_data_read = false;//bandera que indica si toda la imagen fue leída
      char tmp[PACK_THROUGHPUT]; //buffer temporal para escribir en data_block
      /*
      este es el ciclo principal del hilo 0, es el que se encarga de leer toda la imagen,
      la condición de salida es que ya no se pueda leer nada de la imagen
      */
      while(!all_data_read)
      {
        //hay que escribir mientras data_block esté vacío
        while( check_emptyness(data_block,PACK_THROUGHPUT) )
        {
          if(fread(tmp, PACK_THROUGHPUT, 1, file) > 0) //si se logran leer 512B, escriba en data_block
          {
            //el acceso al data_block debe ser crítico
            #pragma omp critical (shared_buffer)
            {
              strncpy(data_block,tmp,PACK_THROUGHPUT);
            }
            printf("Sender: Pude escribir en data block\n");
          }
          else //si ya no se puede leer del archivo, todos los datos fueron leídos
          {
            all_data_read = true;
          }

        }

      }
      fclose(file); //ya se leyó todo el archivo en este punto
      last_package_read = true; //se prende la bandera compartida
    }
    //---------------------------------fin del hilo 0 -----------------------------------------//
  }

  //----------------------------------fin de la sección paralela ---------------------------------//

}





//----------------funciones(subrutinas) utilizadas en el programa del emisor--------------------//
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
