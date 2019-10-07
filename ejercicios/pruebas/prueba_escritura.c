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
#define PACK_SIZE 516 //pack_size total, incluyendo headers
#define PACK_THROUGHPUT 512 //cantidad de datos de todo el paquete (throughput)


int main(int argc, char** argv)// argv[1] = puerto por el que recibo
{
  struct sockaddr_in source, dest;
  bzero(&source, sizeof(source));
  bzero(&dest, sizeof(dest));

  source.sin_family = AF_INET;
  source.sin_addr.s_addr = INADDR_ANY; //Espero paquetes desde cualquier dirección
  source.sin_port = htons(atoi(argv[1]));//hacia este puerto mío

  char* package = calloc( 1, sizeof(char)* PACK_SIZE );

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bind(sockfd, (struct sockaddr *)&source, sizeof(source));
  unsigned int len = sizeof(dest);
  union Data data;
  data.seq_num = 0;

  int pack_counter = 0;

  int finished = 0;
  FILE* file = fopen(argv[2], "wb");
  if (file == NULL){
    perror("Error opening file: ");
  }

  while(!finished)
  {
     int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&dest, &len);
     printf("Intentando de recibir un paquete\n");

     if(check > 0)
     {

        if(package[0] == 0)
        {
           my_strncpy( data.str , package+1 , 3 );

           printf("Recibi el paquete %d\n", data.seq_num );

		   if(data.seq_num == pack_counter)
		   {
         ++pack_counter;
         data.seq_num = pack_counter;
         printf("Me llego el que esperaba :D RN++ \n");
			   fwrite(package, PACK_THROUGHPUT, 1, file);
		   }

           if(package[4] == '*')
           {
             finished = true;
           }


        }


     }

	package[0] = 1;
	data.seq_num = pack_counter;
	my_strncpy(package+1, data.str , 3 );
	int sendtonum = sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&dest, len);
	printf("Enviando ack #%d, %d/516 enviados en sendto\n", pack_counter, sendtonum );

	usleep(300000);

  }

  int sendtonum = sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&dest, len);
  free(package);
  fclose(file);
  return 0;
}
