all: udp_simula_stop_and_wait.c
	gcc -Wall -Wextra udp_simula_stop_and_wait.c list.c -o out -fopenmp

.PHONY: clean
clean:
	rm -f out
