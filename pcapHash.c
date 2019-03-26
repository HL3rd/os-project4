// Ale Lopez, Bailey Blum, and Horacio Lopez
// CSE34341 - Project 4
// Due: 3/28/2019

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "functions.h"

double totalBytes = 0; // keeps track of all bytes
double totalDuplicateBytes = 0; // keeps track of all duplicate bytes
struct PacketHolder g_MyBigTable[30000];

double checkPacketsForDuplicates(struct PacketHolder p) {

    uint32_t theHash = hashlittle(p.byData, p.bytes, 1);
    uint32_t bucket = theHash % 30000;

    p.nHash = theHash;

    double duplicateBytes = 0;

    // Our data matches that in the table
    if (g_MyBigTable[bucket].bIsValid && memcmp(g_MyBigTable[bucket].byData, p.byData, p.bytes)) {
        printf("Cache Hit\n\n\n");
        duplicateBytes = p.bytes;
    // Evict and add new data
    } else {
        g_MyBigTable[bucket] = p;
    }

    return duplicateBytes;
}

void DumpAllPacketLengths (FILE *fp)
{
    // We are going to assume that fp is just after the global header 
    uint32_t     nPacketLength;
    // unsigned char         theData[2400];
    // unsigned char         theHashData[2348];
    // uint32_t     choppedPacketLength;
    

    while(!feof(fp)) {
        /* Skip the ts_sec field */
        fseek(fp, 4, SEEK_CUR);

        /* Skip the ts_usec field */
        fseek(fp, 4, SEEK_CUR);

        /* Read in the incl_len field */
        fread(&nPacketLength, 4, 1, fp);
        //printf("%d", nPacketLength);

        /* Skip the orig_len field */
        fseek(fp, 4, SEEK_CUR);

        /* Let’s do a sanity check before reading */
        if(nPacketLength < 128) {
            printf("Skipping %d bytes ahead - packet is too small\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else if (nPacketLength > 2400) {
            printf("Skipping %d bytes ahead - packet is too big\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else {
            //printf("HEY\n\n\n");
            /* Might not be a bad idea to pay attention to this return value */
            
            // skip the first 52 bytes of data
            fseek(fp, 52, SEEK_CUR);

            struct PacketHolder packetHolder;

            // read in the data directly into the packet holder
            size_t bytesRead = fread(packetHolder.byData, nPacketLength - 52, 1, fp); //this is the packet.
            packetHolder.bytes = bytesRead;
            packetHolder.bIsValid = 1;

            // update global counter
            totalBytes += bytesRead;

        /* Bailey's code
            // printf("Packet length was %d\n", nPacketLength);
            printf("Orignal Packet----------\n");
            for (int i = 0; i < nPacketLength; i++) {
                printf("%hhx", theData[i]);
            }
            printf("\n");
            // This is the value we will want to eliminate the first 52 bytes of to then be hashed.
            int j = 0;
            choppedPacketLength = nPacketLength - 52;
            for (int i = 52; i < nPacketLength; i++) {
                theHashData[j] = theData[i];
                j++;
            }
            printf("Chopped packet length was %d\n", choppedPacketLength);
            printf("Chopped Packet----------\n");
            for (int i = 0; i < choppedPacketLength; i++) {
                printf("%hhx", theHashData[i]);
            }
            printf("\n");
        */

            size_t duplicateBytes = checkPacketsForDuplicates(packetHolder);
            totalDuplicateBytes += duplicateBytes;
        }
        // At this point, we have read the packet and are onto the next one 
        //totalCt += 1;
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

    printf("totalBytes: %f\n", totalBytes);
    printf("DuplicateBytes: %f\n", totalDuplicateBytes);

    double percentage = (double)totalDuplicateBytes / (double)totalBytes * 100;
    printf("result: %f\n", percentage);
}
