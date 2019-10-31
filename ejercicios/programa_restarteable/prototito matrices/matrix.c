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
    double** matrix;

}matrix_info_t;

typedef struct
{
    matrix_info_t* A;
    matrix_info_t* B;
    matrix_info_t* I;
    matrix_info_t* Temp;
    int flag;
    int i, j, iters, totalprod;
    FILE* file;
    int jump_location;

}matrixes_status_t;

void handler(int senal);
void get_back_values();
void save_matrix_register( matrix_info_t* matrix );
void recover_matrix_register( matrix_info_t* matrix );
double** make_matrix();
void destroy_matrix(double** matrix);
void stop_simulation();


static matrixes_status_t* registro_matrices;

/* ----- Subrutinas y funciones de restarteo ------------*/

void handler(int senal)
{
    if (senal == SIGINT)
    {
        printf("\nSe recibió la señal correctamente\n");
        registro_matrices->flag = false;
        //Guarda el estado actual de los índices y las matrices de todo el programa (importa el orden)
    }
}

void stop_simulation()
{
    save_matrix_register( registro_matrices->A );
    save_matrix_register( registro_matrices->B );
    save_matrix_register( registro_matrices->I );
    save_matrix_register( registro_matrices->Temp );
    
    fwrite( &registro_matrices->jump_location,  sizeof(int), 1, registro_matrices->file );
    fwrite( &registro_matrices->i, sizeof(int), 1, registro_matrices->file );
    fwrite( &registro_matrices->j, sizeof(int), 1, registro_matrices->file );
    fwrite( &registro_matrices->iters, sizeof(int), 1, registro_matrices->file );
    fwrite( &registro_matrices->totalprod, sizeof(int), 1, registro_matrices->file );
       
    destroy_matrix(registro_matrices->A->matrix);
    destroy_matrix(registro_matrices->B->matrix);
    destroy_matrix(registro_matrices->I->matrix);
    destroy_matrix(registro_matrices->Temp->matrix);
    
    fclose(registro_matrices->file);  
}

//Recupera los datos de las matrices anteriores (si existen)
void get_back_values()
{
    registro_matrices->file = fopen( "values.dat","wx" );
    if( registro_matrices->file == NULL )
    {
		registro_matrices->file = fopen("values.dat","r+");
        recover_matrix_register( registro_matrices->A );
        recover_matrix_register( registro_matrices->B );
        recover_matrix_register( registro_matrices->I );
        recover_matrix_register( registro_matrices->Temp );
        
        fread( &registro_matrices->jump_location, sizeof(int), 1, registro_matrices->file );
        fread( &registro_matrices->i, sizeof(int), 1, registro_matrices->file );
        fread( &registro_matrices->j, sizeof(int), 1, registro_matrices->file );
        fread( &registro_matrices->iters, sizeof(int), 1, registro_matrices->file );
        fread( &registro_matrices->totalprod, sizeof(int), 1, registro_matrices->file );

        rewind( registro_matrices->file );
    }
    else
    {
        //Cuando el archivo aún no existe, todo empieza en 0
        registro_matrices->i = 0;
        registro_matrices->j = 0;
    }

}

//Guarda los estados de las matrices de la iteración actual
//NOTA: Es importante el orden de lectura (el mismo de guardado y cargado)
void save_matrix_register( matrix_info_t* matrix )	
{  
    //Escribe en el archivo de datos la matriz "n-ésima", por donde iba en la ejecución
    for(int row_index = 0; row_index < M_SIZE; ++row_index)
    {
        for(int col_index = 0; col_index < M_SIZE; ++col_index)
        {
            fwrite( &matrix->matrix[row_index][col_index],sizeof(double),1, registro_matrices->file );
        }
    }
}

//Recupera los valores de iteraciones anteriores para una matriz
//NOTA: Es importante el orden de lectura (el mismo de guardado y cargado)
void recover_matrix_register( matrix_info_t* matrix ) 
{
    //Cuando el archivo ya existe

    for(int row_index = 0; row_index < M_SIZE; ++row_index)
    {
        for(int col_index = 0; col_index < M_SIZE; ++col_index)
        {
            fread( &matrix->matrix[row_index][col_index],sizeof(double),1,registro_matrices->file );
        }
    }
}


//Reserva memoria para las distintas matrices
double** make_matrix()
{
    double** matrix = calloc(M_SIZE,sizeof(double*));
    for(int i = 0; i < M_SIZE; ++i)
    {
        matrix[i] = calloc(M_SIZE,sizeof(double));
    }

    return matrix;
}

//Libera memoria de las matrices alocadas previamente
void destroy_matrix(double** matrix)
{
    for(int i = 0; i < M_SIZE; ++i)
    {
        free(matrix[i]);
    }
    free(matrix);
}

/* ----- Subrutinas y funciones ----------- */
void mult(double** X, double** Y, double** Temp, int dim) {
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

void lea(FILE *fd, double** X, int dim) {
    int i, j;
    for (i=0; i<dim; i++) {
        for (j=0; j<dim; j++)
            fscanf(fd, "%lf", &X[i][j]);
    }
}
/* -----------------*/

void imprima(double** X, int dim) {
    int i, j;
    for (i=0; i<dim; i++) {
        for (j=0; j<dim; j++)
            printf("%12.2lf", X[i][j]);
        printf("\n");
    }
}
/* -----------------*/

void ident(double** X, int dim) {
    int i, j;
    for (i=0; i<dim; i++) {
        for (j=0; j<dim; j++)
            if (i == j) X[i][i] = 1.0;
            else X[i][j] = 0.0;
    }
}
/* -----------------*/

void scalar(double** X, int dim, double val) {
    int i, j;
    for (i=0; i<dim; i++) {
        for (j=0; j<dim; j++)
            X[i][j] *= val;
    }
}
/* -----------------*/

int verify(double** X, int dim) {
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
    int dim, n;
    double det, sdet, c;

    registro_matrices = ( matrixes_status_t* )calloc( 1, sizeof(matrixes_status_t) );
    registro_matrices->A = ( matrix_info_t* )calloc( 1, sizeof( matrix_info_t ) );
    registro_matrices->B = ( matrix_info_t* )calloc( 1, sizeof( matrix_info_t ) );
    registro_matrices->I = ( matrix_info_t* )calloc( 1, sizeof( matrix_info_t ) );
    registro_matrices->Temp = ( matrix_info_t* )calloc( 1, sizeof( matrix_info_t ) );
    
    registro_matrices->A->matrix = make_matrix();
    registro_matrices->B->matrix = make_matrix();
    registro_matrices->I->matrix = make_matrix();
    registro_matrices->Temp->matrix = make_matrix();
    
    registro_matrices->iters = 0;
    registro_matrices->totalprod = 0;
    registro_matrices->i = 0;
    registro_matrices->j = 0;
    registro_matrices->jump_location = 0;
  
    registro_matrices->flag = 1;
  
    //Asignamos el sighandler
	  if (signal(SIGINT, handler)== SIG_ERR)
	  {
		    printf("\nNo se puedo atrapar SIGINT\n");
	  }


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
    lea(fdata, registro_matrices->A->matrix, dim);
    lea(fdata, registro_matrices->B->matrix, dim);
    fclose(fdata);

    get_back_values();

    sdet = sqrt(det);
    c = 1.0/sdet;
    srand(time(0));
    printf("Leidos: dim=%d, det=%lf, sdet=%lf\n", dim, det, sdet);
    printf("\nMatriz A leida:\n");
    imprima(registro_matrices->A->matrix, dim);

    printf("\nMatriz B leida:\n");
    imprima(registro_matrices->B->matrix, dim);

    ident(registro_matrices->I->matrix, dim);

    //VAR COMPARTIDA DE LOCACIÓN: CURRENT_LOCATION (KEY)

    while (1) {
        fout  = fopen("trace.txt", "a");
        if( fout == NULL ) {
            perror("Error opening trace.txt: ");
            return(-1);
        }
        n = rand() % 6 + 1;
        
        if( registro_matrices->jump_location != 0 )
        {
            switch( registro_matrices->jump_location )
            {
                case 1:
                    registro_matrices->jump_location = 0;
                    goto jump1;
                    break;
                ;
                
                case 2:
                    registro_matrices->jump_location = 0;
                    goto jump2;
                    break;
                ;
                
                default:
					break;
            }
          
        }
        

jump1:
        for (registro_matrices->i = 0; registro_matrices->i<n; registro_matrices->i++) {
            mult(registro_matrices->I->matrix, registro_matrices->A->matrix, registro_matrices->Temp->matrix, dim);
            scalar(registro_matrices->I->matrix, dim, c);
            if( registro_matrices->flag == 0 )
            {
				printf("Terminando programa \n");
                registro_matrices->jump_location = 1;
                stop_simulation();
                fclose(fout);
                free(registro_matrices);
                return 0;

            }

        }
        ;

jump2:
        for (registro_matrices->j = 0; registro_matrices->j<n; registro_matrices->j++) {
            mult(registro_matrices->I->matrix, registro_matrices->B->matrix, registro_matrices->Temp->matrix, dim);
            scalar(registro_matrices->I->matrix, dim, c);
            if( registro_matrices->flag == 0 )
            {
				printf("Terminando programa \n");
                registro_matrices->jump_location = 2;
                stop_simulation();
                fclose(fout);
                free(registro_matrices);
                return 0;
            }
        }
        
        if (verify(registro_matrices->I->matrix, dim)) {
            registro_matrices->iters++;
            registro_matrices->totalprod += 2*n;
            fprintf(fout, "Iteracion %d verificada OK: productos = %d, \ttotalprod = %d\n", registro_matrices->iters, 2*n, registro_matrices->totalprod);
            printf("Completadas %d iteraciones\n", registro_matrices->iters);
        }
        else {
            printf("Iter %d presenta error. Se cancela el programa\n", registro_matrices->iters+1);
            fprintf(fout, "Iter %d presenta error. Se cancela el programa\n", registro_matrices->iters+1);
            exit(1);
        }


        fclose(fout);
        usleep(100000);
    }

    return(0);
}
