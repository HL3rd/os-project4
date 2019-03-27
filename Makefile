all: threadedRE

threadedRE: threadedRE.o
	gcc threadedRE.o -o threadedRE -lpthread

threadedRE.o: threadedRE.c
	gcc -Wall -g -c threadedRE.c -o threadedRE.o

prodcon: prodcon.o
	gcc prodcon.o -o prodcon -lpthread

prodcon.o: prodcon.c
	gcc -Wall -g -c prodcon.c -o prodcon.o

clean:
	rm -f threadedRE.o
	rm -f prodcon.o
