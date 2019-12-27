CC=gcc
all: asn3.o
	$(CC) -pthread -lm -o asn3.out asn3.c
clean:
		rm -f *.out assignment_3_output_file.txt
