// Ale Lopez, Bailey Blum, and Horacio Lopez
// CSE34341 - Project 4
// Due: 3/28/2019

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

void DumpAllPacketLengths (FILE *fp)
{
    /* We are going to assume that fp is just after the global header */
    uint32_t     nPacketLength;
    char         theData[2000];
 
    while(!feof(fp)) {
        /* Skip the ts_sec field */
        fseek(fp, 4, SEEK_CUR);   
 
        /* Skip the ts_usec field */
        fseek(fp, 4, SEEK_CUR);
 
        /* Read in the incl_len field */
        fread(&nPacketLength, 4, 1, fp);
 
        /* Skip the orig_len field */
        fseek(fp, 4, SEEK_CUR);
 
        /* Let’s do a sanity check before reading */
        if(nPacketLength < 2000) {
            printf("Packet length was %d\n", nPacketLength);
 
            /* Might not be a bad idea to pay attention to this return value */
            fread(theData, 1, nPacketLength, fp);
        }
        else {
            printf("Skipping %d bytes ahead - packet is too big\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
 
        /* At this point, we have read the packet and are onto the next one */
    }
}

int main(int argc, char* argv[])
{
    FILE *fp;
    fp = fopen("Dataset-Small.pcap", "r+");

    /* Display the Magic Number and skip over the rest */
 
    uint32_t   theMagicNum;
 
    if(fread(&theMagicNum, 4, 1, fp) < 4) {
      // There was an error
    }
 
    printf("Magic Number was %X\n", theMagicNum);
 
    /* Jump through the rest (aka the other 20 bytes) to get past the global header */
    fseek(fp, 24, SEEK_CUR);

    DumpAllPacketLengths(fp);


}
