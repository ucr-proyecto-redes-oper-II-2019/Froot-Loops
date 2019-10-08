#include <stdio.h>
#include <omp.h>
#include <string.h>
#include "suma.h"
#define VECTOR_SIZE 10

int min(int a, int b);
int calcular_inicio(int vector_size, int num_hilos, int mi_id, int inicio_datos);
int calcular_final(int vector_size, int num_hilos, int mi_id, int inicio_datos);
void rellenar_vector(int* vector, int vector_size);
int sumar_vector(int* vector, int vector_size);


int main(int argc, char** argv)
{
	int vector[VECTOR_SIZE];

	//Llenamos el vector para que tenga valores
	rellenar_vector(vector, VECTOR_SIZE);
	
	int sum = 0;
	sum = sumar_vector(vector,VECTOR_SIZE);
	printf("La suma es %d\n",sum);
	return 0;
}

int sumar_vector(int* vector, int vector_size)
{
	int sum = 0;
	#pragma omp parallel shared(sum)
	{
		int parcial = 0;
		int hilos_totales = omp_get_num_threads();
		int mi_id = omp_get_thread_num();
		
		int inicio_datos = calcular_inicio(VECTOR_SIZE,hilos_totales,mi_id,0);
		
		int mi_final = calcular_final(VECTOR_SIZE,hilos_totales,mi_id,0);
		
		printf("Mi id: %d, mi inicio: %d, mi final: %d\n",mi_id,inicio_datos,mi_final);
		
		
		for(int i = inicio_datos; i < mi_final; ++i)
		{
			parcial = suma(parcial,vector[i]);
		}
		
		#pragma omp critical
		{
			sum = suma(sum,parcial);
		}
	}
	
	return sum;
}

//Rellenar el vector por completo de forma tal que la suma sea la suma de Gauss
void rellenar_vector(int* vector, int vector_size)
{
	for(int i = 0; i < VECTOR_SIZE; ++i)
	{
		vector[i] = i;
	}
}

int calcular_inicio(int vector_size, int num_hilos, int mi_id, int inicio_datos)
{
    int equitativo = mi_id * ((vector_size - inicio_datos) / num_hilos);
    int sobrecarga = min((mi_id), (vector_size - inicio_datos) % num_hilos);
    
    return inicio_datos + equitativo + sobrecarga;
}


int calcular_final(int vector_size, int num_hilos, int mi_id, int inicio_datos)
{
    return calcular_inicio(vector_size, num_hilos, mi_id + 1, inicio_datos);
}

int min(int a, int b)
{
	if(a > b)
		return b;
	else
		return a;
}
