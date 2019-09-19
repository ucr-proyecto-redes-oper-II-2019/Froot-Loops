#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PACKAGE_LIMIT 11
#define PACKAGE_SIZE 516

void queueInit(queue_t* queue)
{
  queue->front = -1;
  queue->rear = -1;
  queue->size = PACKAGE_LIMIT;
  queue->recv_matrix = (char** )calloc(PACKAGE_LIMIT, sizeof(char* ));

  for(int index = 0; index < PACKAGE_LIMIT; index++)
  {
	queue->recv_matrix[index] =  (char *) calloc(PACKAGE_SIZE, sizeof(char));
  }
}

int enQueue(queue_t* queue, char* package)
{
  
  //check if queue is full
  if ((queue->front == 0 && queue->rear == queue->size-1) || (queue->rear == (queue->front-1)%(queue->size-1)))
  {
    printf("\nQueue is Full");
    return -1;
  }
  else if (queue->front == -1) //insert fisrt element
  {
    queue->front = 0;
    queue->rear = 0;
    
    //queue.recv_matrix[queue.rear] = package;
    strncpy( queue->recv_matrix[queue->rear], package, 516 );
    //printf("%s ", queue->recv_matrix[queue->rear]);
  }
  else if (queue->rear == queue->size-1 && queue->front != 0)
  {
    queue->rear = 0;
    
    //queue.recv_matrix[queue.rear] = package;
    strncpy( queue->recv_matrix[queue->rear], package, PACKAGE_SIZE );

  }else
  {
    queue->rear++;
    
    //queue.recv_matrix[queue.rear] = package;
    strncpy( queue->recv_matrix[queue->rear], package, PACKAGE_SIZE );
  }
  return EXIT_SUCCESS;
}

char* deQueue(queue_t* queue)
{
  if (queue->front == -1)
  {
    printf("\nQueue is Empty\n");
    return (char*)-1;
  }
  /*char* data = (char*)malloc(PACKAGE_SIZE*(sizeof(char)));
  strcpy( data, "izi");*/
  char* data = (char*)malloc(PACKAGE_SIZE*(sizeof(char)));
  strncpy( data, queue->recv_matrix[queue->front], PACKAGE_SIZE );
  queue->recv_matrix[queue->front] = 0;

  if (queue->front == queue->rear)
  {
    queue->front = -1;
    queue->rear = -1;
  }
  else if (queue->front == (queue->size)-1)
    queue->front = 0;
  else
    queue->front++;

  return data;
}


char* queueFirst(queue_t* queue)
{
  return queue->recv_matrix[queue->front];
}

char* queueLast(queue_t* queue)
{
  return queue->recv_matrix[queue->rear];
}

void displayQueue(queue_t* queue)
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

void queueDestroy(queue_t* queue)
{

  for(int index = 0; index < PACKAGE_LIMIT; ++index)
  {
    free(queue->recv_matrix[index]);
  }
  free(queue->recv_matrix);

}

int main()
{
  queue_t queue;
  queueInit(&queue);
  enQueue(&queue, "jajaja1");
  enQueue(&queue, "jajaja2");
  enQueue(&queue, "jajaja3");
  displayQueue(&queue);
  deQueue(&queue);
  displayQueue(&queue);
  queueDestroy(&queue);

  return 0;
}

