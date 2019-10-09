all: sender.c list.c auxiliar_funtions.c file_reader.c packer.c
	gcc -Wall -Wextra sender.c list.c auxiliar_funtions.c file_reader.c packer.c -o out -fopenmp

.PHONY: clean
clean:
	rm -f out
