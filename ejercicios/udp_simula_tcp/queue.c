#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PACKAGE_LIMIT 10
#define PACKAGE_SIZE 516
#define true 1
#define false 0


void list_init(list_t* queue)// initialize the list
{
  queue->front = -1;
  queue->rear = -1;
  queue->size = PACKAGE_LIMIT;
  queue->recv_matrix = (char** )calloc(PACKAGE_LIMIT, sizeof(char* ));

  bzero(queue->ack_array,10);

  for(int index = 0; index < PACKAGE_LIMIT; index++)
  {
	queue->recv_matrix[index] =  (char *) calloc(PACKAGE_SIZE, sizeof(char));
  }

}

int insert(list_t* queue, char* package)//insert package to its respective place in the list
{
  //this is used to control the package header (first 4B)
  union Data data;
  strncpy(data.str, package+1, 3);// we only care about the last 3B becuase it refers to the sequence number of the package

  //check if list is full
  if ((queue->front == 0 && queue->rear == queue->size-1) || (queue->rear == (queue->front-1)%(queue->size-1)))
  {
    printf("\nList is Full\n");
    return -1;
  }
  else
  {
    //if its the first element to be inserted
    if (queue->front == -1)
    {
      queue->front = 0;
      queue->rear = 0;

      strncpy( queue->recv_matrix[queue->rear], package, PACKAGE_SIZE );//add it to the list
      printf("Estoy agregando el paquete %d\n", data.seq_num);
      queue->ack_array[queue->rear] = true;
      return EXIT_SUCCESS;
    }
    //if the list is full on the end but there's space at the begginig of the list
    else if (queue->rear == queue->size-1 && queue->front == 0)
    {
      //if the package's seq_num does not overlap the list's size and if the package has not yet been received, handle it
      if ((data.seq_num < (queue->recv_matrix[queue->front][1] + PACKAGE_SIZE)) && (strncmp(queue->recv_matrix[queue->rear], package, PACKAGE_SIZE ) != 0))
      {
        queue->rear = 0;

        strncpy( queue->recv_matrix[queue->rear], package, PACKAGE_SIZE );//add it to the list
        printf("Estoy agregando el paquete %d\n", data.seq_num);
        queue->ack_array[queue->rear] = true;
      }
      return EXIT_SUCCESS;
    }
    //otherwise just insert the package on the corresponding index (position)
    else
    {
      //calculate index in which the package shall be placed
      int package_index = data.seq_num % ((queue->front - queue->rear) + 1);

      //if the package has not yet been received, add it to the list and if the package's seq_num does not overlap the list's size, then handle it
      if ((strncmp(queue->recv_matrix[package_index], package, PACKAGE_SIZE ) != 0) && (data.seq_num < (queue->recv_matrix[queue->front][1] + PACKAGE_SIZE)))
      {
        strncpy( queue->recv_matrix[package_index], package, PACKAGE_SIZE );//add it to the list
        printf("Estoy agregando el paquete %d\n", data.seq_num);
        queue->ack_array[queue->rear] = true;
        queue->rear++;//advance the window
      }
      return EXIT_SUCCESS;
    }
  }
}

char* pop(list_t* queue)
{
  //check if list is empty
  if (queue->front == -1)
  {
    printf("\nList is Empty\n");
    return (char*)-1;
  }
  /*char* data = (char*)malloc(PACKAGE_SIZE*(sizeof(char)));
  strcpy( data, "izi");*/

  //variable to hold data to be shown when user pops the first element of the list
  char* data = (char*)malloc(PACKAGE_SIZE*(sizeof(char)));
  strncpy( data, queue->recv_matrix[queue->front], PACKAGE_SIZE );

  //stablish and asign a neutral value to put in the list when the element is popped
  queue->recv_matrix[queue->front] = 0;

  //handle the list's front pointer depending on the situation
  if (queue->front == queue->rear)// if list is just 1 element big
  {
    queue->ack_array[queue->front] = false;
    queue->front = -1;
    queue->rear = -1;
  }
  else if (queue->front == (queue->size)-1)//this is what it makes the list circular, if front is at the end of list
  //it resets and goes back to the beggining
  {
    queue->ack_array[queue->front] = false;
    queue->front = 0;
  }
  else
  {
    queue->ack_array[queue->front] = false;
    queue->front++; //window just shrinks
  }

  return data;
}


char* first(list_t* queue)
{
  return queue->recv_matrix[queue->front];
}

char* last(list_t* queue)
{
  return queue->recv_matrix[queue->rear];
}

int is_ready(list_t* queue)
{
  int is_ready = false;
  int list_size = (queue->front - queue->rear);
  int ready_count = 0;

  //check every slot of ack array in the window range to see if every slot is true
  for (int index = queue->front; index <= list_size; index++)
  {
    if (queue->ack_array[index] == true)
      ready_count++;
  }
  if(ready_count == (list_size+1))
    is_ready = true;

  return is_ready;
}

void display_list(list_t* queue)
{

  if (queue->front == -1)
  {
    printf("\nQueue is Empty\n");
    return;
  }

  printf("\nElements in queue are: ");

  if (queue->rear >= queue->front)
  {
	//printf("llega aca");
    for (int i = queue->front;  i <= queue->rear; i++)
    printf("%s ", queue->recv_matrix[i]);
  }
  else
  {
    for (int i = queue->front;  i < queue->size; i++)
      printf("%s ", queue->recv_matrix[i]);

    for (int i = 0;  i <= queue->rear; i++)
      printf("%s ", queue->recv_matrix[i]);
  }

}

void destroy(list_t* queue)
{

  for(int index = 0; index < PACKAGE_LIMIT; ++index)
  {
    free(queue->recv_matrix[index]);
  }
  free(queue->recv_matrix);
}
