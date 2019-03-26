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
struct Node* g_MyBigTable[30000]; // this is our hash table

double checkPacketsForDuplicates(struct PacketHolder packet) {

    uint32_t theHash = hashlittle(packet.byData, packet.bytes, 1); //hashes our payload
    uint32_t bucket = theHash % 30000;

    packet.nHash = theHash;

    double duplicateBytes = 0;

    // check if the "bucket" contains an element
    if (g_MyBigTable[bucket]) {

        struct Node* head = g_MyBigTable[bucket]; //sets head of linked list equal to first item in bucket
        int matchFound = 0; // keeps track of whether a match is found in the bucket
       
        // check if there is perfect match
        if (g_MyBigTable[bucket]->p.nHash == packet.nHash) {  
            duplicateBytes = packet.bytes;
            matchFound = 1;
        }
        while (g_MyBigTable[bucket]->next != NULL) { //read through linked list
            g_MyBigTable[bucket] = g_MyBigTable[bucket]->next;
            
            // match is found
            if (g_MyBigTable[bucket]->p.nHash == packet.nHash) { 
                duplicateBytes = packet.bytes;
                matchFound = 1;
                break; //we have found a match and can return
            }
        }

        // No match: add packet to the linked list
        if (matchFound == 0) {
            struct Node *newNode = malloc(sizeof(struct Node));
            newNode->next = NULL;
            newNode->p = packet;
            g_MyBigTable[bucket]->next = newNode;
            g_MyBigTable[bucket] = head; //resets bucket to point to the first item in the linked list. 
        }     
    }
    
    // Bucket is empty: add packet to the bucket
    else {
        struct Node *newNode = malloc(sizeof(struct Node));
        newNode->next = NULL;
        newNode->p = packet;
        g_MyBigTable[bucket] = newNode;
    }    

    return duplicateBytes;
}

void DumpAllPacketLengths (FILE *fp)
{
    // We are going to assume that fp is just after the global header 
    uint32_t     nPacketLength;

    while(!feof(fp)) {
        /* Skip the ts_sec field */
        fseek(fp, 4, SEEK_CUR);

        /* Skip the ts_usec field */
        fseek(fp, 4, SEEK_CUR);

        /* Read in the incl_len field */
        fread(&nPacketLength, 4, 1, fp);

        /* Skip the orig_len field */
        fseek(fp, 4, SEEK_CUR);

        /* Check to see if packets are in range */
        if(nPacketLength < 128) { //packet is too small
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else if (nPacketLength > 2400) { //packet is too big
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else {
            // skip the first 52 bytes of data
            fseek(fp, 52, SEEK_CUR);

            struct PacketHolder packetHolder;

            // read in the data directly into the packet holder
            size_t bytesRead = fread(packetHolder.byData, 1, nPacketLength - 52, fp); //this is the packet.
            packetHolder.bytes = bytesRead;

            // update global byte count
            totalBytes += bytesRead;

            size_t duplicateBytes = checkPacketsForDuplicates(packetHolder);
            totalDuplicateBytes += duplicateBytes;
        }
        // At this point, we have read the packet and are onto the next one 
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
