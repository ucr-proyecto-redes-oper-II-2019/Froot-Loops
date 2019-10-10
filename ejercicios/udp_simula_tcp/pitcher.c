#include "pitcher.h"

void pitcher( char* my_port, char* destiny_ip, char* destiny_port, list_t* list, int* list_lock, int* all_data_read )
{
   
  //usleep(300000);
  char* package = calloc( 1, sizeof(char)* PACK_SIZE );// se crea el contenedor del paquete de 516B
  union Data data;
  data.seq_num = 0;
  int RN = 0;
  //---------------definición de direcciones-----------//
  struct sockaddr_in source;
  struct sockaddr_in dest;
  unsigned int len = sizeof(dest);
  printf("Antes de net_setup \n");
  net_setup(&source, &dest, my_port, destiny_ip, destiny_port);
  printf("Despues de net_setup \n");
  //-----------------creación del socket-----------------//
  int sockfd = make_socket(&source);
  printf("Despues de socket \n");

  while( !(*all_data_read) || !(is_empty(list)) )
  {
    //se intentan de enviar todos los paquetes actualmente en la lista
    for(int index = 0; index < list->size; ++index) //intenta de enviar todos los paquetes actualmente en la lista sin eliminarlos de la misma
    {
      if(list->ack_array[index] == true) //si existe logicamente en la lista se envia
      {
        my_strncpy(package, list->recv_matrix[index], PACK_SIZE);
        my_strncpy(data.str,list->recv_matrix[index] + 1 ,3);
        printf("Sender[2] enviando package #%d por red\n",data.seq_num);
        usleep(100000);
        sendto(sockfd, package, PACK_SIZE, 0, (struct sockaddr*)&dest, len);
      }
    }
        

      //se recupera un paquete y se extrae el ACK
    int check = recvfrom(sockfd, package, PACK_SIZE, MSG_DONTWAIT, (struct sockaddr*)&dest, &len);
    if( check > 0 && package[0] == 1 )
    {
       my_strncpy(data.str, package+1, 3); //----------------------------------------be careful
       while(!*list_lock)
         ;//-----------
             
       printf("Sender[2] recibi ack #%d de receptor\n",data.seq_num);
       flush(list, RN, data.seq_num);


       //flush rn's menores que el ack más reciente, para enviar desde el ack recibido en adelante
       RN = data.seq_num;
       printf("Sender[2] despues de flush, RN: %d \n", RN);
    }
    *list_lock  = false;
    
  }

  free(package);
  close(sockfd);
  //-----------------------------------periodo de rendirse---------------------------------------//
}

//se encarga de llenar los registros con los datos de red recibidos
void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port)
{  		
  bzero(source, sizeof(*source)); //se limpian ambos registros de antemano
  bzero(dest, sizeof(*dest));

  source->sin_family = AF_INET;
  source->sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
  source->sin_port = htons(atoi(my_port));//Mi puerto donde estoy escuchando

  dest->sin_family = AF_INET;
  dest->sin_addr.s_addr = inet_addr(destiny_ip); //IP destino se especifica en el 2do parametro de linea de comando
  dest->sin_port = htons(atoi(destiny_port)); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando  
}

int make_socket(struct sockaddr_in* source)
{
  int tmp_sock = socket(AF_INET, SOCK_DGRAM, 0);

  //Se enlaza el socket sockfd a la estructura de datos source
  int bind_return = bind(tmp_sock, (struct sockaddr *)&source, sizeof(source));

  if(bind_return < 0)
  {
		printf("Socket creation failed\n");
  }
  return tmp_sock;
}
