all: pcap

pcap: pcapHash.o
	gcc pcapHash.o -o pcap -lpthread

pcapHash.o: pcapHash.c
	gcc -Wall -g -c pcapHash.c -o pcapHash.o

clean:
	rm -f pcapHash.o pcapHash
