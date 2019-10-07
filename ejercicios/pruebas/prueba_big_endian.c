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
#include <time.h>

int main()
{
  union Data
  {
    int seq_num;
    char str[4];
  }data;
  //printf("Holi\n");
  //union Data dato;
  //printf("Holi2\n");
  data.seq_num = 0;

  data.seq_num = 65000;

  printf("El int en little endian es:\n");
  for (int i = 0; i < 4; i++)
  {

    printf("Char #%d es %hhu\n", i, data.str[i]);
  }

  int tmp = htonl(data.seq_num);
  data.seq_num = tmp;
  printf("El int en big endian es:\n");
  for (int i = 0; i < 4; i++)
  {

    printf("Char #%d es %hhu\n", i, data.str[i]);
  }
  
}
