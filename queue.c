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

#define MAX 4096 //TODO: define MAX

int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int loops = 2; // TODO: set value of loops

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

cond_t empty, fill;
mutex_t mutex;

void* producer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == MAX) {
            pthread_cond_wait(&empty, &mutex);
        }
        put(i);
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
    }
}

void* consumer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&fill, &mutex);
        }
        int tmp = get();
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char* argv[]) {
    return 0;
}