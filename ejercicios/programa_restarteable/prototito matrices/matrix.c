#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define true 1
#define false 0
#define M_SIZE 15

typedef struct
{
	int i;
	int j;
	int** matrix;

}matrix_info_t;

typedef struct
{
	matrix_info_t* A;
	matrix_info_t* B;
	matrix_info_t* I;
	matrix_info_t* Temp;
	int flag;
	FILE* file;
	int jump_location;
	
}matrixes_status_t;

void handler(int senal);
void get_back_values();
void save_matrix_register( matrix_info_t* matrix );
void recover_matrix_register( matrix_info_t* matrix );

static matrixes_status_t* registro_matrices;

/* ----- Subrutinas y funciones de restarteo ------------*/

void handler(int senal)
{
  if (senal == SIGINT)
  {
	  printf("\nSe recibió la señal correctamente\n");
	  save_matrix_register( registro_matrices->A );
	  save_matrix_register( registro_matrices->B );
	  save_matrix_register( registro_matrices->I );
	  save_matrix_register( registro_matrices->Temp );
	  registro_matrices->flag = true;
	  fwrite( &registro_matrices->jump_location,  sizeof(int), 1, registro_matrices->file );
	  
	  //Guarda el estado actual de los índices y las matrices de todo el programa (importa el orden)
  }  
}

//Recupera los datos de las matrices anteriores (si existen)
void get_back_values()
{
	registro_matrices->file = fopen( "values.dat","wx" );
	if( registro_matrices->file == NULL )
	{
		
		recover_matrix_register( registro_matrices->A );
		recover_matrix_register( registro_matrices->B );
		recover_matrix_register( registro_matrices->I );
		recover_matrix_register( registro_matrices->Temp );
		fread( &registro_matrices->jump_location, sizeof(int), 1, registro_matrices->file );
		
		rewind( registro_matrices->file );
	}

}

//Guarda los estados de las matrices de la iteración actual
//NOTA: Es importante el orden de lectura (el mismo de guardado y cargado)
void save_matrix_register( matrix_info_t* matrix )	
{  
	  fwrite( &matrix->i,sizeof(int),1,registro_matrices->file );
	  fwrite( &matrix->j,sizeof(int),1,registro_matrices->file );
	  
	  //Escribe en el archivo de datos la matriz "n-ésima", por donde iba en la ejecución
	  for(int row_index = 0; row_index < M_SIZE; ++row_index)
	  {
		for(int col_index = 0; col_index < M_SIZE; ++col_index)
		{
			fwrite( &registro_matrices->matrix[row_index][col_index],sizeof(double),1, registro_matrices->file );
		}	  
	  }
}

//Recupera los valores de iteraciones anteriores para una matriz
//NOTA: Es importante el orden de lectura (el mismo de guardado y cargado)
void recover_matrix_register( matrix_info_t* matrix ) 
{
	//Cuando el archivo ya existe
	registro_matrices->file = fopen( "values.dat","r+" );	
	fread( &matrix->i,sizeof(int),1,matrix->file );
	fread( &matrix->j,sizeof(int),1,matrix->file );
	
	for(int row_index = 0; row_index < M_SIZE; ++row_index)
	{
		for(int col_index = 0; col_index < M_SIZE; ++col_index)
		{
			fread( &matrix->matrix[row_index][col_index],sizeof(double),1,registro_matrices->file );
		}
	}
	else
	{
		//Cuando el archivo aún no existe
		matrix->i = 0;
		matrix->j = 0;
	}	
}

/* ----- Subrutinas y funciones ----------- */
void mult(double X[15][15], double Y[15][15], double Temp[15][15], int dim) {
	int i, j, k;
	for (i=0; i<dim; i++)
	  for (j=0; j<dim; j++) {
		Temp[i][j] = 0;
		for (k=0; k<dim; k++)
		  Temp[i][j] += X[i][k]*Y[k][j];
	  }
	for (i=0; i<dim; i++)
	  for (j=0; j<dim; j++) {
        X[i][j] = Temp[i][j];
	}	  
}
/* -----------------*/

void lea(FILE *fd, double X[15][15], int dim) {
	int i, j;
	for (i=0; i<dim; i++) {
	  for (j=0; j<dim; j++) 
         fscanf(fd, "%lf", &X[i][j]);
	 }
}
/* -----------------*/

void imprima(double X[15][15], int dim) {
	int i, j;
	for (i=0; i<dim; i++) {
	  for (j=0; j<dim; j++) 
         printf("%12.2lf", X[i][j]);
      printf("\n");
	 }
}
/* -----------------*/

void ident(double X[15][15], int dim) {
	int i, j;
	for (i=0; i<dim; i++) {
	  for (j=0; j<dim; j++) 
        if (i == j) X[i][i] = 1.0;
        else X[i][j] = 0.0;
	 }
}
/* -----------------*/

void scalar(double X[15][15], int dim, double val) {
	int i, j;
	for (i=0; i<dim; i++) {
	  for (j=0; j<dim; j++) 
        X[i][j] *= val;
	 }
}
/* -----------------*/

int verify(double X[15][15], int dim) {
	int i, j;
	for (i=0; i<dim; i++) {
	  for (j=0; j<dim; j++) 
        if (i == j) {
			if (X[i][i] != 1.0)
			   return(0);
		}
		else {
		  if (X[i][j] != 0.0)
		     return(0);
		 }
	 }
	 return(1);
}


/* --------------------------MAIN---------------------------------*/
int main(void) {
/* ----- Variables -----*/
  FILE *fdata, *fout;
  int dim, n, i, j, iters, totalprod;
  double A[15][15], B[15][15], I[15][15], Temp[15][15], det, sdet, c;
  
  registro_matrices = ( matrixes_status_t* )calloc( 1, sizeof(info_t) );
  registro_matrices->jump_location = 0;
  registro_matrices->A = &A; //CHECK
  registro_matrices->A = &B; //CHECK
  registro_matrices->A = &I; //CHECK
  registro_matrices->A = &Temp; //CHECK
  get_back_values();

/* --- Instrucciones ---*/
  fdata = fopen("matrices.dat", "r");
  if( fdata == NULL ) {
      perror("Error opening matrices.dat: ");
      return(-1);
   }

  fout  = fopen("trace.txt", "w");
  if( fout == NULL ) {
      perror("Error opening trace.txt: ");
      return(-1);
   }
  fclose(fout);

  fscanf(fdata, "%d %lf", &dim, &det);  
  lea(fdata, A, dim);
  lea(fdata, B, dim);
  fclose(fdata);

  sdet = sqrt(det);
  c = 1.0/sdet;
  srand(time(0));
  printf("Leidos: dim=%d, det=%lf, sdet=%lf\n", dim, det, sdet);
  printf("\nMatriz A leida:\n");
  imprima(A, dim);

  printf("\nMatriz B leida:\n");
  imprima(B, dim);
  
  ident(I, dim);
  
  iters = 0;
  totalprod = 0;
  
  //VAR COMPARTIDA DE LOCACIÓN: CURRENT_LOCATION (KEY)
  
  while (1) {
	fout  = fopen("trace.txt", "a");
    if( fout == NULL ) {
       perror("Error opening trace.txt: ");
       return(-1);
    }
    //KEY WORD 1
    n = rand() % 6 + 1;
   //KEY WORD 2 (Con índices asignados)
	for (i=0; i<n; i++) {
	   mult(I, A, Temp, dim);
	   scalar(I, dim, c);
    }
    //KEY WORD 3 (Con índices asignados)
	for (i=0; i<n; i++) {
	   mult(I, B, Temp, dim);
	   scalar(I, dim, c);
    }
    //KEY WORD 3 (Con índices asignados)
    if (verify(I, dim)) {
		iters++;
		totalprod += 2*n;
		fprintf(fout, "Iteracion %d verificada OK: productos = %d, \ttotalprod = %d\n", iters, 2*n, totalprod);
		printf("Completadas %d iteraciones\n", iters);
	}
	else {
	    printf("Iter %d presenta error. Se cancela el programa\n", iters+1);
	    fprintf(fout, "Iter %d presenta error. Se cancela el programa\n", iters+1);
	    exit(1);
	}
	

    fclose(fout);
	usleep(100000);
  }
  
  return(0);
}
