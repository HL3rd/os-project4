// Ale Lopez, Bailey Blum, and Horacio Lopez
// CSE34341 - Project 4
// Due: 3/28/2019

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <string.h>  
#include <getopt.h> 
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
    if (g_MyBigTable[bucket].bIsValid && memcmp(g_MyBigTable[bucket].byData, p.byData, p.bytes) == 0) {
        // printf("Cache Hit\n\n\n");
        duplicateBytes = p.bytes;
    // Evict and add new data
    } else {
        g_MyBigTable[bucket] = p;
        g_MyBigTable[bucket].bIsValid = 1;
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
            // printf("Skipping %d bytes ahead - packet is too small\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else if (nPacketLength > 2400) {
            // printf("Skipping %d bytes ahead - packet is too big\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else {
            // printf("Packet was a good size.\n");
            /* Might not be a bad idea to pay attention to this return value */
            
            // skip the first 52 bytes of data
            fseek(fp, 52, SEEK_CUR);

            struct PacketHolder packetHolder;

            // read in the data directly into the packet holder
            size_t bytesRead = fread(packetHolder.byData, 1, nPacketLength - 52, fp); //this is the packet.
            packetHolder.bytes = bytesRead;

            // update global counter
            totalBytes += bytesRead;

            size_t duplicateBytes = checkPacketsForDuplicates(packetHolder);
            totalDuplicateBytes += duplicateBytes;
        }
        // At this point, we have read the packet and are onto the next one 
        //totalCt += 1;
    }
}

int main(int argc, char* argv[])
{
    int c;
    int level = 2; // is no level if specified, we will run using Level 2
    int threads = 2; //TODO: specify a default value for threads
    int min_threads = 2; // minimum number of allowed threads
    int max_files = 10; // maximum number of processed files

    int option_index = 0;
    static struct option long_options[] = 
    {
        {"level",   required_argument, NULL,  'l'},
        {"thread",  required_argument, NULL,  't'},
        {NULL,      0,                 NULL,    0}
    };

    int files = argc - 1;

    // Parse command line arguments
    while ((c = getopt_long_only(argc, argv, ":l:t:", long_options, &option_index)) != -1) {
        switch(c) {
			case 'l':
                level = atoi(optarg);
                files = files - 2;
                printf("level: %d\n", level);
                break;
            case 't':
                threads = atoi(optarg);
                files = files - 2;
                printf("threads: %d\n", threads);
                break;
		}
	}

    // Check number of threads
    if (threads < min_threads) {
        printf("error: Minimum of two threads allowed.\n");
        exit(1);
    }
    
    // Save files to be processed in an array
    char *filenames[max_files];
    int i;
    for (i = 0; i < files; i++) {
        filenames[i] = argv[argc-files + i];
        printf("file %d: %s\n", i, filenames[i]);
    }

    for (i = 0; i < files; i++) {
        FILE *fp;
        fp = fopen(filenames[i], "r+");

        /* Display the Magic Number and skip over the rest */

        uint32_t   theMagicNum;

        if(fread(&theMagicNum, 4, 1, fp) < 4) {
        // There was an error
        }

        // printf("Magic Number was %X\n", theMagicNum);

        /* Jump through the rest (aka the other 20 bytes) to get past the global header */
        fseek(fp, 24, SEEK_CUR);

        DumpAllPacketLengths(fp);   
        
        fclose(fp);
    }

    printf("totalBytes: %f\n", totalBytes);
    printf("DuplicateBytes: %f\n", totalDuplicateBytes);

    double percentage = (totalDuplicateBytes / totalBytes) * 100;
    printf("result: %f\n", percentage);
}
