all: threadedRE prodconAle

threadedRE: threadedRE.o
	gcc threadedRE.o -o threadedRE -lpthread

threadedRE.o: threadedRE.c
	gcc -Wall -g -c threadedRE.c -o threadedRE.o

prodconAle: prodconAle.o
	gcc prodconAle.o -o prodconAle -lpthread

prodconAle.o: prodconAle.c
	gcc -Wall -g -c prodconAle.c -o prodconAle.o

clean:
	rm -f threadedRE.o
	rm -f prodconAle.o
