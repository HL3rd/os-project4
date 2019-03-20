all: pcapHash

pcapHash: pcapHash.o
	gcc pcapHash.o -o pcapHash -lpthread

pcapHash.o: pcapHash.c
	gcc -Wall -g -c pcapHash.c -o pcapHash.o

clean:
	rm -f pcapHash.o pcapHash
