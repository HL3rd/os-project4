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
#include <unistd.h>
#include "functions.h"

double totalBytes = 0; // keeps track of all bytes
double totalDuplicateBytes = 0; // keeps track of all duplicate bytes
struct Node* g_MyBigTable[30000]; // this is our hash table
int cacheHits = 0;

#define MAXSIZ 10000000//TODO: define MAX
struct PacketHolder buffer[MAXSIZ];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int complete = 0;

void put(struct PacketHolder packet) {
    buffer[fill_ptr] = packet;
    fill_ptr = (fill_ptr + 1) % MAXSIZ;
    count++;
}

struct PacketHolder get() {
    struct PacketHolder tmp = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % MAXSIZ;
    count--;
    return tmp;
}
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

//TODO: we now have to keep track of the size of our data structure ourselves to
//make sure it doesn't go above 64MB. If it does, then we have to start
//evicting items from our cache using a FIFO method. Our linked lists are going to increase the size
//of the structure.

void *producer(void *arg) {
    FILE *fp = arg;
    uint32_t     nPacketLength;
    printf("inside producer\n");
    while(!feof(fp)) {
        complete = 0;
        printf("producer acquired the lock\n");
        while (count == MAXSIZ) {
            pthread_cond_wait(&empty, &mutex);
        }
        printf("producer received signal empty\n");
        // We are going to assume that fp is just after the global header

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
            size_t bytesRead = fread(packetHolder.byData, 1, 64, fp); //this is the packet.
            packetHolder.bytes = bytesRead;

            // update global byte count
            totalBytes += bytesRead;
            put(packetHolder);
            printf("put packet in\n");
            pthread_cond_signal(&fill);
            printf("producer signaled fill\n");
        }
        printf("producer releasing lock\n");
        pthread_mutex_unlock(&mutex);
    }
    complete = 1;
    while (count > 0) {
        pthread_cond_signal(&fill);
    }
    pthread_cond_broadcast(&fill);
    return 0;
}

void *consumer(void* arg) {
    printf("inside consumer\n");
    printf("complete = %d\n", complete);
    while(1) {

        printf("inside complete loop in consumer\n");
        printf("count= %d\n", count);
        pthread_mutex_lock(&mutex);
        printf("consumer acquired the lock\n");

        if (complete == 1 & count == 0) {
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        while (count == 0) {
            pthread_cond_wait(&fill, &mutex);
        }
        printf("consumer received signal fill\n");
        struct PacketHolder packet = get();
        printf("got package\n");
        uint32_t theHash = hashlittle(packet.byData, packet.bytes, 1); //hashes our payload
        uint32_t bucket = theHash % 30000;

        packet.nHash = theHash;

        double duplicateBytes = 0;

        // check if the "bucket" contains an element
        if (g_MyBigTable[bucket]) {

            struct Node* head = g_MyBigTable[bucket]; //sets head of linked list equal to first item in bucket
            int matchFound = 0; // keeps track of whether a match is found in the bucket

            // check if there is perfect match
            if (g_MyBigTable[bucket]->p.nHash == packet.nHash && memcmp(g_MyBigTable[bucket]->p.byData, packet.byData, packet.bytes) == 0) {
                duplicateBytes = packet.bytes;
                matchFound = 1;
                cacheHits += 1;
            }
            while (g_MyBigTable[bucket]->next != NULL) { //read through linked list
                g_MyBigTable[bucket] = g_MyBigTable[bucket]->next;

                // match is found
                if (g_MyBigTable[bucket]->p.nHash == packet.nHash && memcmp(g_MyBigTable[bucket]->p.byData, packet.byData, packet.bytes) == 0) {
                    duplicateBytes = packet.bytes;
                    matchFound = 1;
                    cacheHits += 1;
                    break; //we have found a match and can return
                }
            }

            // No match: add packet to the linked list
            if (matchFound == 0) {
                // here we want to check if totalDataByteCount < 64
                struct Node *newNode = malloc(sizeof(struct Node));
                // totalDataByteCount += sizeofStructNode
                // totalDataByteCount should be global
                newNode->next = NULL;
                newNode->p = packet;
                g_MyBigTable[bucket]->next = newNode;
                g_MyBigTable[bucket] = head; //resets bucket to point to the first item in the linked list.
                // otherwise, if not < 64, then we
                // g_MyBigTable[bucket] = g_MyBigTable[bucket]->next, THEN do the insertion
            }
        }

        // Bucket is empty: add packet to the bucket
        else {
            struct Node *newNode = malloc(sizeof(struct Node));
            newNode->next = NULL;
            newNode->p = packet;
            g_MyBigTable[bucket] = newNode;
        }

        totalDuplicateBytes += duplicateBytes;

        pthread_cond_signal(&empty);
        printf("consumer signaled empty\n");
        printf("consumer releasing lock\n");
        pthread_mutex_unlock(&mutex);
    }
    printf("consumer is leaving\n");
    return 0;
}

/*-----------------------------------------------------*/

int main(int argc, char* argv[])
{
    int c; //TODO: Change default level back to 2
    int level = 1; // is no level if specified, we will run using Level 2
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
    }

    //for loop for reading through each file
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

        //Thread Management
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_t prodID;
        pthread_t consID[threads - 1];
        pthread_create(&prodID, &attr, producer, fp); //makes producer thread
        for (int i = 0; i < threads - 1; i++) { //FOR loop to make consumer threads
            pthread_create(&consID[i], &attr, consumer, NULL);
        }
        pthread_join(prodID, NULL); //waits for producer thread to finish.
        for (int i = 0; i < threads - 1; i++) { //waits for consumer threads to finish.
            pthread_join(consID[i], NULL);
        }

        //TODO: Move levels to producer and consumer functions
        // if (level == 1) {
        //     DumpAllPacketLengths(fp); //still need threading for this section
        // }
        // else if (level == 2) {

        // }


        fclose(fp);
    }


    //Output results

    printf("Welcome to Project 4 - ThreadedRE by Dos Lopezes y un Gringo\n");
    printf("Now operating in Level %d mode: ", level);
    if (level == 1) {
        printf("Full Payload Detection.\n");
    }
    else {
        printf("Sub-Payload Matching.\n");
    }
    printf("Threads Allowed: %d\n", threads);
    printf("Allocating %d thread to file I/O, %d thread(s) to checking for redundancy.\n", 1, threads-1);
    printf("Results:\n");
    printf("%.2f MB processed.\n", (totalBytes/1000000));
    //printf("DuplicateBytes: %f\n", totalDuplicateBytes);
    printf("%d hits.\n", cacheHits);

    double percentage = (totalDuplicateBytes / totalBytes) * 100;
    printf("%.2f%% redundancy detected.\n", percentage);
}
