all: sender.c list.c auxiliar_functions.c file_reader.c packer.c pitcher.c
	gcc -Wall -Wextra sender.c list.c auxiliar_functions.c file_reader.c packer.c pitcher.c -o out -fopenmp

.PHONY: clean
clean:
	rm -f out
