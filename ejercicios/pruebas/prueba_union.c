#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"

int main()
{
	
	list_t list;
	char tmp[4];
	int a = 0;
	union Data data;
	
	//list_init(&list);
	
	data.seq_num = 15;
	strncpy(tmp+1,data.str,3);
	
	//bzero(data.str,4);
	
	//strncpy(data.str,tmp+1,3);
	
	//a = data.seq_num;
	
	printf("%d\n",data.seq_num);
	
	/*char* str = malloc(516 *sizeof(char));
	data.seq_num = 3;
	printf("Soy data seq %d\n",data.seq_num);
	strncpy(str+1,data.str,3);
		
	insert(&list,str);*/
	
	/*
	for(int i = 0; i < 11; ++i)
	{
		char* str = malloc(516 *sizeof(char));
		data.seq_num = i;
		printf("Soy data seq %d\n",data.seq_num);
		strncpy(str+1,data.str,3);
		
		insert(&list,str);
		//pop(&list);
	}
	
	for(int i = 0; i < 11; ++i)
	{
		pop(&list);
	}
	
	
	/*for(int i = 10; i < 11; ++i)
	{
		char* str = malloc(516 *sizeof(char));
		data.seq_num = i;
		printf("Soy data seq %d\n",data.seq_num);
		strncpy(str+1,data.str,3);
		
		insert(&list,str);
		//pop(&list);
	}*/
	
/*
	char* str = malloc(516 *sizeof(char));
	data.seq_num = 10;
	printf("Soy data seq %d\n",data.seq_num);
	strncpy(str+1,data.str,3);
		
	insert(&list,str);
	
	pop(&list);
	
	char* k = malloc(516 *sizeof(char));
	data.seq_num = 11;
	printf("Soy data seq %d\n",data.seq_num);
	strncpy(k+1,data.str,3);
		
	insert(&list,k);
	
	destroy(&list);*/
	

	return 0;
}
