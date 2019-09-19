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
  printf("jejeps4");
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
    printf("jejeps");
    //queue.recv_matrix[queue.rear] = package;
    strncpy( queue->recv_matrix[queue->rear], package, 516 );
  }
  else if (queue->rear == queue->size-1 && queue->front != 0)
  {
    queue->rear = 0;
    printf("jejeps2");
    //queue.recv_matrix[queue.rear] = package;
    strncpy( queue->recv_matrix[queue->rear], package, 516 );

  }else
  {
    queue->rear++;
    printf("jejeps3");
    //queue.recv_matrix[queue.rear] = package;
    strncpy( queue->recv_matrix[queue->rear], package, 516 );
  }
}

char* deQueue(queue_t* queue)
{
  if (queue->front == -1)
  {
    printf("\nQueue is Empty");
    return (char*)-1;
  }

  char* data = queue->recv_matrix[queue->front];
  queue->recv_matrix[queue->front] = "vacio";

  if (queue->front == queue->rear)
  {
    queue->front = -1;
    queue->rear = -1;
  }
  else if (queue->front == queue->size-1)
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
    printf("\nQueue is Empty");
    return;
  }

  printf("\nElements in queue are: ");

  if (queue->rear >= queue->front)
  {
    for (int i = queue->front;  i < queue->rear; i++)
      printf("%s ", queue->recv_matrix[i]);
  }
  else
  {
    for (int i = queue->front;  i < queue->size; i++)
      printf("%s ", queue->recv_matrix[i]);

    for (int i = 0;  i < queue->rear; i++)
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
  enQueue(&queue, "j");
  displayQueue(&queue);
  queueDestroy(&queue);

  return 0;
}
