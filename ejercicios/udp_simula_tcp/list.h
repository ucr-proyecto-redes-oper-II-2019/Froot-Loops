#ifndef LIST_H
#define LIST_H
/*
Grupo: Froot Loops
Integrantes:
Daniel Barrantes
Antonio Alvarez
Steven Barahona 
*/
//la cola tiene que ser una estructura de tamano 11 que contenga paquetes de 516

#define PACKAGE_LIMIT 10
#define PACKAGE_SIZE 516
#define true 1
#define false 0
#define INSERT_FAILURE -1
#define INSERT_FAIL_REPEATED -2

typedef struct
{
  //rear is the end of the window (f)
  //front is the beggining of the window (i)
  int ack_array[10];
  int rear, front;
  int size;
  char** recv_matrix;
}list_t;

union Data
{
  int seq_num;
  char str[4];
}data;

void list_init(list_t* queue);// initialize the queue

int insert(list_t* queue, char* package);// add a 516B package

char* pop(list_t* queue);// return the pointer to the package in the front

char* first(list_t* queue);// returns first package in queue

char* last(list_t* queue);// returns last package in queue

int is_ready(list_t* queue_t);

void display_list(list_t* queue);

void destroy(list_t* queue);

int is_empty(list_t* queue);

char* my_strncpy(char *dest, const char *src, int n);

#endif //QUEUE_H
