all: threadedRE noThreaded prodconHora

threadedRE: threadedRE.o
	gcc threadedRE.o -o threadedRE -lpthread

threadedRE.o: threadedRE.c
	gcc -Wall -g -c threadedRE.c -o threadedRE.o

noThreaded: noThreaded.o
	gcc noThreaded.o -o noThreaded -lpthread

noThreaded.o: noThreaded.c
	gcc -Wall -g -c noThreaded.c -o noThreaded.o

prodconHora: prodconHora.o
	gcc prodconHora.o -o prodconHora -lpthread

prodconHora.o: prodconHora.c
	gcc -Wall -g -c prodconHora.c -o prodconHora.o

clean:
	rm -f threadedRE.o noThreaded.o prodconHora.o
	rm -f threadedRE noThreaded prodconHora
