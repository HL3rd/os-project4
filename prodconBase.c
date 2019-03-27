#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <string.h>  
#include <unistd.h>
#include "functions.h"

/* Producer/Consumer code implementation */
/* Producer/Consumer code found in OS textbook, chapter 30 page 12. */
/* Based on section 29.3 in chapter 29, it seems like just using this is fine */

#define MAX 4096//TODO: define MAX
int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int loops; // TODO: set value of loops, should be number of files.

void put(int value) {
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % MAX;
    count++;
}

int get() {
    int tmp = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % MAX;
    count--;
    return tmp;
}
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

//TODO: We are supposed to essentially put all of our code into the producer and consumer
//threads. Producers read in packets and consumers check if they are in our
//data structure. We need to make sure we lock certain operations, such as 
//doing things with our data structure.

void *producer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == MAX) {
            pthread_cond_wait(&empty, &mutex);
        }
        //TODO: read in input here
        put(i);
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&fill, &mutex);
        }
        int tmp = get();
        //TODO: tmp contains our packet. Perform duplicate check here.
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char* argv[]) {
    return 0;
}