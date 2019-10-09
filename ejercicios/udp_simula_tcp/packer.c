#include "packer.h"
//Este es el empaquetador manejado por el hilo 1
//Verifica si recibe una nueva sección de datos de la capa superior
//empaqueta el dato y busca insertarlo a la lista de envíos
void packer(char* data_block, int* all_data_read, int* block_is_free, list_t* list)
{
	char* package = calloc( 1, sizeof(char)* PACK_SIZE );// se crea el contenedor del paquete de 516B
	union Data data;
	data.seq_num = 0;
	int SN = 0;

	//hasta que el hilo 0 ya no pueda leer más (se acabó el archivo)
	// y me aseguro de que no quedan datos en el bloque para que yo procese
	while(!*all_data_read || !*block_is_free) 
	{
		printf("Entré a empaquetar\n");
		//verificamos que el bloque no esté lógicamente vacío (por ende podemos leer de él)
		//Si está vacío debemos esperar, pero si se acaban los datos no tiene sentido esperar
		while(!*all_data_read && *block_is_free)
			;
		
		//-----------------empaquetamiento------------------//
		package[0] = 0; //se especifica que es un envío
		
		if (all_data_read) //si ya se leyó todo el archivo, se añade el paquete final a la lista
			package[4] = '*';
		else		
			my_strncpy( package+4, data_block, PACK_THROUGHPUT ); //se copian los 512 leídos del archivo por el hilo 0
			
		data.seq_num = SN;//htonl(SN);//se guarda el SN en BIG ENDIAN en el data.seq_num
		my_strncpy( package+1, data.str, 3 );//se guardan los primeros 3B del seq_num en el paquete
		//-----------------empaquetamiento------------------//
		
		//Una vez que empaqueté puedo liberar al hilo para que siga su labor
		//Ya no dependo de los datos del data_block
		*block_is_free = true;
		
		//intenta de meter el paquete hasta que obtenga error code = success
		printf("Sender[1]: El sq intentando de insertar es %d\n",data.seq_num);
		
		//Si falla al insertar el paquete debo esperar porque la lista está llena
		//En algún punto el hilo 2 liberá espacio de la 
		while(insert( list, package) == -1)
			;
		
		printf("Sender[1]: El sq que logré insertar es %d\n",data.seq_num);
		
		SN++;
		
		//Por aquí debo liberar al hilo 2 para que continúe su labor ya que el paquete
		//fue insertado

		//usleep(200000);//usleep para darle chance al hilo 0 de que escriba en el data_block
		// Ya no hace falta lo controla el candado
		usleep(200000);

	}
	free(package);
}
