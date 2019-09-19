#ifndef QUEUE_H
#define QUEUE_H

//la cola tiene que ser una estructura de tamano 11 que contenga paquetes de 516
typedef struct
{
  int rear, front;
  int size;
  char** recv_matrix;
}queue_t;

void queueInit(queue_t* queue);// initialize the queue

int enQueue(queue_t* queue, char* package);//add a 516B package

char* deQueue(queue_t* queue);// return the pointer to the package in the front

char* queueFirst(queue_t* queue);// returns first package in queue

char* queueLast(queue_t* queue);// returns last package in queue

void displayQueue(queue_t* queue);

void queueDestroy(queue_t* queue);

#endif //QUEUE_H
