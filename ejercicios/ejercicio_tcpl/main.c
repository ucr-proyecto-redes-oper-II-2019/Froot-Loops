#include <stdio.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include "suma.h"
//#define VECTOR_SIZE 10


int min(int a, int b);
/**
 * @brief calcular_inicio Dado un thread n, calcula del vector cual indice
 * inicial le corresponde.
 * @param vector_size Total vector size.
 * @param num_hilos Numero de hilos creados/indicados en el programa.
 * @param mi_id Identificador para cada hilo.
 * @param inicio_datos Desde cual indice en adelante se reparten los datos
 * @return Valor de indice inicial.
 */
int calcular_inicio(int vector_size, int num_hilos, int mi_id, int inicio_datos);
/**
 * @brief calcular_final Dado un thread n, calcula del vector cual indice
 * final le corresponde.
 * @param vector_size Total vector size.
 * @param num_hilos Numero de hilos creados/indicados en el programa.
 * @param mi_id Identificador para cada hilo.
 * @param inicio_datos Desde cual indice en adelante se reparten los datos
 * @return Valor de indice final.
 */
int calcular_final(int vector_size, int num_hilos, int mi_id, int inicio_datos);
/**
 * @brief rellenar_vector Rellena el vector con la suma de Gauss del tamaño del vector,
 * cada indice con su numero respectivo.
 * @param vector Vector sobre el cual se trabaja.
 * @param vector_size Tamaño del vector.
 */
void rellenar_vector(int* vector, int vector_size);
/**
 * @brief sumar_vector Ejecuta concurrente la suma de cada hilo.
 * @param vector Vector sobre el cual se trabaja.
 * @param vector_size Tamaño del vector.
 * @return Suma parcial de los indices correspondientes al thread.
 */
int sumar_vector(int* vector, int vector_size);


int main(int argc, char** argv)
{
	
	int VECTOR_SIZE = 10;
	
	//Si el usuario indica un tamaño de vector, se usa ese, de lo contrario se usa el defecto (10)
	if(argc >= 2)
	{
		//Usage: ./tcpl [vector_size]
		VECTOR_SIZE = atoi(argv[1]);
	}
	
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
	int hilos_potenciales = omp_get_max_threads(); 
	
	//Se analiza cuantos threads son necesarios crear
	int hilos_a_crear = min( hilos_potenciales, vector_size);
	
	#pragma omp parallel shared(sum) num_threads(hilos_a_crear)
	{
		int parcial = 0;
		int hilos_totales = hilos_a_crear;
		int mi_id = omp_get_thread_num();
		
		int inicio_datos = calcular_inicio(vector_size,hilos_totales,mi_id,0);
		
		int mi_final = calcular_final(vector_size,hilos_totales,mi_id,0);
		
		printf("Mi id: %d, mi inicio: %d, mi final: %d\n",mi_id,inicio_datos,mi_final);
		
		//Realiza la suma parcial del thread.
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
	for(int i = 0; i < vector_size; ++i)
	{
		vector[i] = i;
	}
}

//Calcula los indices del vector a los cuales cada vector especifico va a atenter
int calcular_inicio(int vector_size, int num_hilos, int mi_id, int inicio_datos)
{
    int equitativo = mi_id * ((vector_size - inicio_datos) / num_hilos);
    int sobrecarga = min((mi_id), (vector_size - inicio_datos) % num_hilos);
    
    return inicio_datos + equitativo + sobrecarga;
}

//Calcula el indice final para cada hilo individual
int calcular_final(int vector_size, int num_hilos, int mi_id, int inicio_datos)
{
    return calcular_inicio(vector_size, num_hilos, mi_id + 1, inicio_datos);
}


//Funcion generica de minimo
int min(int a, int b)
{
	if(a > b)
		return b;
	else
		return a;
}
