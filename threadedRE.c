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
double totalCacheSize = 0; // keeps track of the data structure's size
double totalDuplicateBytes = 0; // keeps track of all duplicate bytes
struct Node* g_MyBigTable[30000]; // this is our hash table
int cacheHits = 0; //keeps track of cache hits

int count = 0; //keeps track of items in buffer
int complete = 0; //keeps track of whether producer has finished processing the file
int level = 2; // default is Level 2

/* Queue Implementation - Our Producer/Consumer Buffer */

typedef struct __node_t {
    struct PacketHolder p;
    struct __node_t *next;
} node_t;

typedef struct __queue_t {
    node_t *head;
    node_t *tail;
    pthread_mutex_t headLock;
    pthread_mutex_t tailLock;
} queue_t;

queue_t buffer;

void queue_init(queue_t *q) {
    node_t *tmp = malloc(sizeof(node_t));
    tmp->next = NULL;
    q->head = q->tail = tmp;
    pthread_mutex_init(&q->headLock, NULL);
    pthread_mutex_init(&q->tailLock, NULL);
}

void queue_push(queue_t *q, struct PacketHolder p) {
    node_t *tmp = malloc(sizeof(node_t));
    if (tmp != NULL) {
        tmp->p = p;
        tmp->next = NULL;
    }
    
    pthread_mutex_lock(&q->tailLock);
    q->tail->next = tmp;
    q->tail = tmp;
    pthread_mutex_unlock(&q->tailLock);
    
}

int queue_pop(queue_t *q, struct PacketHolder *p) {
    pthread_mutex_lock(&q->headLock);
    node_t *tmp = q->head;
    node_t *newHead = tmp->next;
    if (newHead == NULL) {
        pthread_mutex_unlock(&q->headLock);
        return -1; // queue was empty
    }
    *p = newHead->p;
    q->head = newHead;
    pthread_mutex_unlock(&q->headLock);
    free(tmp);
    
    return 0;
}

#define MAXSIZ 10000

/* Locks and Condition Variables */

pthread_mutex_t cache = PTHREAD_MUTEX_INITIALIZER; //Cache Lock
pthread_mutex_t countlock = PTHREAD_MUTEX_INITIALIZER; //Buffer Lock
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    FILE *fp = arg;
    uint32_t     nPacketLength;
    
    while(!feof(fp)) {
        complete = 0;
        // We are going to assume that fp is just after the global header

        /* Skip the ts_sec field */
        fseek(fp, 4, SEEK_CUR);
        if (feof(fp)) {
            break;
        }

        /* Skip the ts_usec field */
        fseek(fp, 4, SEEK_CUR);
        if (feof(fp)) {
            break;
        }

        /* Read in the incl_len field */
        fread(&nPacketLength, 4, 1, fp);
        if (feof(fp)) {
            break;
        }

        /* Skip the orig_len field */
        fseek(fp, 4, SEEK_CUR);
        if (feof(fp)) {
            break;
        }

        /* Check to see if packets are in range */
        if(nPacketLength < 128) { //packet is too small
            fseek(fp, nPacketLength, SEEK_CUR);
            if (feof(fp)) {
                break;
            }
        }
        else if (nPacketLength > 2400) { //packet is too big
            fseek(fp, nPacketLength, SEEK_CUR);
            if (feof(fp)) {
                break;
            }
        }
        else {
            // skip the first 52 bytes of data
            fseek(fp, 52, SEEK_CUR);
            if (feof(fp)) {
                break;
            }

            struct PacketHolder packetHolder;

            // read in the data directly into the packet holder
            size_t bytesRead = fread(packetHolder.byData, 1, nPacketLength - 52, fp); //this is the packet.
            packetHolder.bytes = bytesRead;
            if (level == 1){
                pthread_mutex_lock(&countlock);
                while (count == MAXSIZ) {
                    pthread_cond_wait(&empty, &countlock);
                }
                // update global byte count and push to buffer
                totalBytes += bytesRead;
                queue_push(&buffer, packetHolder);
                count++;
                pthread_cond_signal(&fill);
                pthread_mutex_unlock(&countlock);
            }
            // level = 2
            else {
                pthread_mutex_lock(&countlock);
                while (count == MAXSIZ) {
                    pthread_cond_wait(&empty, &countlock);
                }
                /* Below Section divides our packets into windows */
                int left = 0;
                for (int right = 63; right < nPacketLength-52; right++) {
                    char subArray[2400];
                    struct PacketHolder temp;
                    temp.firstIndex = left;
                    temp.bytes = 64;
                    for (int i = 0; i < 64; i++) {
                        subArray[i] = packetHolder.byData[left];
                        left++;
                    }
                    sprintf(temp.byData, "%s", subArray);
                    // update global byte count and push to buffer
                    queue_push(&buffer, temp);
                    totalBytes += 64;
                    left = right - 62;
                    count++;
                }
                pthread_cond_signal(&fill);
                pthread_mutex_unlock(&countlock);
            }
            
        }
        
    }
    complete = 1; //signifies our file has completed.
    return 0;
}

void *consumer(void* arg) {
    while(1) {
        pthread_mutex_lock(&countlock);
        while (count == 0) {
            if (complete == 1) {
                pthread_mutex_unlock(&countlock);
                return 0;
            }
            pthread_cond_wait(&fill, &countlock);
        } 
        struct PacketHolder packet;
        int retnval = queue_pop(&buffer, &packet);
        count--;
        if (complete == 1 && retnval == -1) {
            pthread_mutex_unlock(&countlock);
            return 0;
        }
        pthread_mutex_unlock(&countlock);

        uint32_t theHash = hashlittle(packet.byData, packet.bytes, 1); //hashes our payload
        uint32_t bucket = theHash % 30000;

        packet.nHash = theHash;

        double duplicateBytes = 0;
        pthread_mutex_lock(&cache);
        if (level == 1) {
            // check if the "bucket" contains an element
            if (g_MyBigTable[bucket]) {

                struct Node* head = g_MyBigTable[bucket]; //sets head of linked list equal to first item in bucket
                int matchFound = 0; // keeps track of whether a match is found in the bucket
            
                // check if there is perfect match with the first node
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
        }

        // level = 2
        else {
            // check if the "bucket" contains an element
            if (g_MyBigTable[bucket]) {
                struct Node* head = g_MyBigTable[bucket]; //sets head of linked list equal to first item in bucket
                int matchFound = 0; // keeps track of whether a match is found in the bucket
            
                // check if there is perfect match with the first node
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
                // (only if it's been 32 bytes since our last insertion and our data structure's size is less than 64 MB)
                if (matchFound == 0 && totalCacheSize < 64000000 - sizeof(struct Node) && packet.firstIndex % 32 == 0) {
                    // printf("MATCH FOUND AND SIZE IS STILL ACCEPTABLE AND WE CAN ADD TO CACHE\n");
                    struct Node *newNode = malloc(sizeof(struct Node));
                    totalCacheSize += sizeof(struct Node);
                    newNode->next = NULL;
                    newNode->p = packet;
                    g_MyBigTable[bucket]->next = newNode;
                    g_MyBigTable[bucket] = head; //resets bucket to point to the first item in the linked list. 
                } 

                // data structure has reached maximum size
                // Evict method
                
                else if (matchFound == 0 && packet.firstIndex % 32 == 0) {
                    // Free a node
                    struct Node *temp = g_MyBigTable[bucket];
                    g_MyBigTable[bucket] = g_MyBigTable[bucket]->next;
                    head = g_MyBigTable[bucket];
                    totalCacheSize -= sizeof(struct Node);
                    free(temp);

                    // Add packet to linked list in bucket
                    struct Node *newNode = malloc(sizeof(struct Node));
                    totalCacheSize += sizeof(struct Node);
                    newNode->next = NULL;
                    newNode->p = packet;
                    if (g_MyBigTable[bucket]) {
                        while(g_MyBigTable[bucket]->next != NULL) {
                            g_MyBigTable[bucket] = g_MyBigTable[bucket]->next;
                        }
                        g_MyBigTable[bucket]->next = newNode;
                        g_MyBigTable[bucket] = head; //resets bucket to point to the first item in the linked list. 
                    }
                    else {
                        g_MyBigTable[bucket] = newNode;
                    }
                    
                }    
            }
            // Bucket is empty: add packet to the bucket
            else {
                if (totalCacheSize < 64000000 - sizeof(struct Node) && packet.firstIndex % 32 == 0) {
                    struct Node *newNode = malloc(sizeof(struct Node));
                    totalCacheSize += sizeof(struct Node);
                    newNode->next = NULL;
                    newNode->p = packet;
                    g_MyBigTable[bucket] = newNode;
                }
                else if (packet.firstIndex % 32 == 0) {
                    // Free a node
                    int i = 0;
                    while (!g_MyBigTable[i]) {
                        i++;
                    }
                    struct Node *temp = g_MyBigTable[i];
                    g_MyBigTable[i] = g_MyBigTable[i]->next;
                    totalCacheSize -= sizeof(struct Node);
                    free(temp);

                    // Add node to bucket
                    struct Node *newNode = malloc(sizeof(struct Node));
                    totalCacheSize += sizeof(struct Node);
                    newNode->next = NULL;
                    newNode->p = packet;
                    g_MyBigTable[bucket] = newNode;
                }
            }
        }
        totalDuplicateBytes += duplicateBytes;
        pthread_mutex_unlock(&cache);
    }
        
    return 0;
}

/*-----------------------------------------------------*/

int main(int argc, char* argv[])
{
    int c;
    int threads = 500; //TODO: specify a default value for threads
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
                break;
            case 't':
                threads = atoi(optarg);
                files = files - 2;
                break;
		}
	}

    if (threads == 500) { //user didn't set value, give them defaults.
        if (level == 1) {
            threads = 5;
        }
        else if (level == 2) {
            threads = 2;
        }
    }

    if (level != 1 && level != 2) {
        printf("ERROR: Invalid level given.\n");
        exit(1);
    }

    // Check number of threads
    if (threads < min_threads || threads > 10) {
        printf("ERROR: Only 2 to 10 threads allowed.\n");
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
        queue_init(&buffer);
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
        pthread_t consID[threads];
        pthread_create(&prodID, &attr, producer, fp); //makes producer thread
        int i;
        for (i = 1; i < threads; i++) { //FOR loop to make consumer threads
            pthread_create(&consID[i], &attr, consumer, NULL);
        } 
        
        pthread_join(prodID, NULL); //waits for producer thread to finish.

        while (count > 0) {
            pthread_cond_signal(&fill);
        }
        pthread_cond_broadcast(&fill);

        for (i = 1; i < threads; i++) { //waits for consumer threads to finish.
            pthread_join(consID[i], NULL);
        }

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
    printf("%d hit(s).\n", cacheHits);

    double percentage = (totalDuplicateBytes / totalBytes) * 100;
    printf("%.2f%% redundancy detected.\n", percentage);
}
