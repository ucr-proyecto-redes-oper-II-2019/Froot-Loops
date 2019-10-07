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
#define true 1
#define false 0
#define PACKAGE_LIMIT 10

int refresh_rn( list_t* list, int current_rn)
{
    int recieved = current_rn;
    int stop = false;
    int max = PACKAGE_LIMIT;
    int moved = 0;
  
    while(!stop && (moved < max))
    {
      
      if( list->ack_array[current_rn % PACKAGE_LIMIT] == false )
      {
		printf("Revisando %d \n",list->ack_array[current_rn % PACKAGE_LIMIT]);
        stop = true;
      }
      current_rn++;

      moved++;
      
    }   
    return (current_rn  % PACKAGE_LIMIT);
}


int main(void)
{

	int current_ack = 0;
    list_t list;
    list_init(&list);
	
	list.ack_array[0] == true;
	list.ack_array[1] == true;
	list.ack_array[2] == true;
	
	list.ack_array[4] == true;
	list.ack_array[5] == true;
	list.ack_array[6] == true;
	
	
	current_ack = refresh_rn( &list, 3);
	
	printf("Quedo %d en ack \n", current_ack);
	
	destroy(&list);

	return 0;
}
