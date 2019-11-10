void net_setup(struct sockaddr_in* source, struct sockaddr_in* dest, char* my_port, char* destiny_ip, char* destiny_port)
{
  bzero(source, sizeof(*source)); //se limpian ambos registros de antemano
  bzero(dest, sizeof(*dest));

  source->sin_family = AF_INET;
  source->sin_addr.s_addr = INADDR_ANY; //se pueden recibir paquetes de cualquier fuente
  source->sin_port = htons(atoi(my_port));//Mi puerto donde estoy escuchando

  dest->sin_family = AF_INET;
  dest->sin_addr.s_addr = inet_addr(destiny_ip); //IP destino se especifica en el 2do parametro de linea de comando
  dest->sin_port = htons(atoi(destiny_port)); //el puerto a donde envío se especifica en el 3er parámetro de la línea de comando
}
