#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define true 1
#define false 0
#define M_SIZE 5

typedef struct
{
	int i;
	int j;
	int** matrix;
	FILE* file;
	int flag;
}info_t;

static info_t* my_info;

void print_matrix(int** matrix, int size);
int** make_matrix(int size);
void destroy_matrix(int** matrix, int size);
void fill_matrix(int** matrix, int size);
void get_back_values();


void manejador(int senal)
{
  if (senal == SIGINT)
  {
	  printf("\nSe recibió la señal correctamente\n");
	  my_info->flag = false;
	  
	  fwrite(&my_info->i,sizeof(int),1,my_info->file);
	  fwrite(&my_info->j,sizeof(int),1,my_info->file);
	  
	  //Escribe en el archivo de datos la matrix por donde iba
	  for(int k = 0; k < M_SIZE; ++k)
	  {
		for(int t = 0; t < M_SIZE; ++t)
		{
			fwrite(&my_info->matrix[k][t],sizeof(int),1,my_info->file);
		}
		  
	  }
	  
  }  
}



int main(void)
{
	
	srand(time(0)); 
	my_info = (info_t*)calloc(1,sizeof(info_t));
	my_info->flag = true;
	my_info->matrix = make_matrix(M_SIZE);

	//Asignamos el sighandler
	if (signal(SIGINT, manejador)== SIG_ERR)
	{
		printf("\nNo se puedo atrapar SIGINT\n");
	}

	my_info->i = 0;
	my_info->j = 0;
	
	
	get_back_values();
	print_matrix(my_info->matrix,M_SIZE);
	
	
	printf("\n-------------------------\n");
	
	
	fill_matrix(my_info->matrix,M_SIZE);
	print_matrix(my_info->matrix,M_SIZE);
	
	fclose(my_info->file);
	destroy_matrix(my_info->matrix,M_SIZE);
	free(my_info);
	
}

void get_back_values()
{
	
	my_info->file = fopen("values.dat","wx");
	if(my_info->file == NULL)
	{
		my_info->file = fopen("values.dat","r+");	
		fread(&my_info->i,sizeof(int),1,my_info->file);
		fread(&my_info->j,sizeof(int),1,my_info->file);
		for(int k = 0; k < M_SIZE; ++k)
		{
			for(int t = 0; t < M_SIZE; ++t)
			{
				fread(&my_info->matrix[k][t],sizeof(int),1,my_info->file);
			}
		  
		}
		rewind(my_info->file);
		
	}
	else
	{
		my_info->i = 0;
		my_info->j = 0;
	}
	
}

void destroy_matrix(int** matrix, int size)
{
	for(int i = 0; i < size; ++i)
	{
		free(matrix[i]);
	}
	free(matrix);
}

int** make_matrix(int size)
{
	int** matrix = calloc(size,sizeof(int*));
	for(int i = 0; i < size; ++i)
	{
		matrix[i] = calloc(size,sizeof(int));
	}
	
	return matrix;
}

void fill_matrix(int** matrix,int size)
{
	int num = 0;
	int upper = 99;
	int lower = 0;
	

	for(;my_info->i < size && my_info->flag; ++my_info->i)
	{
		for(; my_info->j < size && my_info->flag; ++my_info->j)
		{
			num = (rand() % (upper - lower + 1)) + lower;
			matrix[my_info->i][my_info->j] = num;
			usleep(100000);
		}
		my_info->j = 0;
	}
}

void print_matrix(int** matrix, int size)
{
	for(int i = 0; i < size; ++i)
	{
		for(int j = 0; j < size; ++j)
		{
			printf("%-2d ",matrix[i][j]);
		}
		putchar('\n');
	}
}
