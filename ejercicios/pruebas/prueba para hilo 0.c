while(all_data_read == false)
{
	while(is_block_free == true && all_data_read == false)
		;
	usleep(500);
	is_block_free = true;
}

//Esto funciona como consumidor dummy para el thread 0 colóquese en vez
// del thread 1 para probar su funcionalidad
