all: restarteable

restarteable: 
	gcc main.c suma.c -o tcpl -fopenmp

.PHONY: clean
clean:
	rm -f tcpl
