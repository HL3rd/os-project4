all: threadedRE noThreaded prodconHora level2 level22 level1

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

level2: level2.o
	gcc level2.o -o level2 -lpthread

level2.o: level2.c
	gcc -Wall -g -c level2.c -o level2.o

level22: level22.o
	gcc level22.o -o level22 -lpthread

level22.o: level22.c
	gcc -Wall -g -c level22.c -o level22.o

level1: level1.o
	gcc level1.o -o level1 -lpthread

level1.o: level1.c
	gcc -Wall -g -c level1.c -o level1.o

clean:
	rm -f threadedRE.o noThreaded.o prodconHora.o
	rm -f threadedRE noThreaded prodconHora
	rm -f level2.o level2
	rm -f level22.o level22
	rm -f level1.o level1
