all: threadedRE noThreaded

threadedRE: threadedRE.o
	gcc threadedRE.o -o threadedRE -lpthread

threadedRE.o: threadedRE.c
	gcc -Wall -g -c threadedRE.c -o threadedRE.o

noThreaded: noThreaded.o
	gcc noThreaded.o -o noThreaded -lpthread

noThreaded.o: noThreaded.c
	gcc -Wall -g -c noThreaded.c -o noThreaded.o

clean:
	rm -f threadedRE.o
	rm -f noThreaded.o
