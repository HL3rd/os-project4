all: pcap threadedRE

pcap: pcapHash.o
	gcc pcapHash.o -o pcap -lpthread

pcapHash.o: pcapHash.c
	gcc -Wall -g -c pcapHash.c -o pcapHash.o

threadedRE: threadedRE.o
	gcc threadedRE.o -o threadedRE -lpthread

threadedRE.o: threadedRE.c
	gcc -Wall -g -c threadedRE.c -o threadedRE.o

clean:
	rm -f pcapHash.o pcap
	rm -f threadedRE.o threadedRE
