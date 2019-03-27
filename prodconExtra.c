// TEST CODE
// want to test producer and consumer code

/* buffer.h */
typedef int buffer_item;
#define BUFFER_SIZE 5

/* main.c */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "functions.h"
#include "buffer.h"

#define RAND_DIVISOR 100000000
#define TRUE 1

// mutex lock
pthread_mutex_t mutex;
// semaphores
sem_t full, empty;
buffer_item buffer[BUFFER_SIZE];
// buffer counter
int counter;

pthread_t tid;       // thread ID
pthread_attr_t attr; // set of thread attributes

void *producer(void *param); // producer thread
void *consumer(void *param); // consumer thread

void initializeData() {

   pthread_mutex_init(&mutex, NULL);     // create the mutex lock
   sem_init(&full, 0, 0);                // create the full semaphore and initialize to 0
   sem_init(&empty, 0, BUFFER_SIZE);     // create empty semaphore and initialize to 0

   pthread_attr_init(&attr);    // get default attributes

   counter = 0;
}

void *producer(void *param) {
   buffer_item item;

   while(TRUE) {
      // sleep for random period of time
      int rNum = rand() / RAND_DIVISOR;
      sleep(rNum);

      // generate random number
      item = rand();

      // acquire the empty lock
      sem_wait(&empty);
      // acquire the mutex lock
      pthread_mutex_lock(&mutex);

      if(insert_item(item)) {
         fprintf(stderr, " Producer report error condition\n");
      }
      else {
         printf("producer produced %d\n", item);
      }
      // release mutex lock and signal that the buffer is full
      pthread_mutex_unlock(&mutex);
      sem_post(&full);
   }
}

void *consumer(void *param) {
    buffer_item item;

    while(TRUE) {
      int rNum = rand() / RAND_DIVISOR;
      sleep(rNum);

      // acquire the full lock and the mutex lock
      sem_wait(&full);
      pthread_mutex_lock(&mutex);

      if(remove_item(&item)) {
          fprintf(stderr, "Consumer report error condition\n");
      }
      else {
          printf("consumer consumed %d\n", item);
      }

      // release the mutex lock and signal that the buffer is now empty
      pthread_mutex_unlock(&mutex);
      sem_post(&empty);
   }
}

/** Functions to insert and remove items from the buffer **/

int insert_item(buffer_item item) {
   // When the buffer is not full add the item and increment the counter
   if(counter < BUFFER_SIZE) {
      buffer[counter] = item;
      counter++;
      return 0;
   }
   else {
      // buffer is full
      return -1;
   }
}


int remove_item(buffer_item *item) {
   // When the buffer is not empty remove the item and decrement the counter
   if(counter > 0) {
      *item = buffer[(counter-1)];
      counter--;
      return 0;
   }
   else {
      // buffer is empty
      return -1;
   }
}

/**********************************************/
int main(int argc, char *argv[]) {
   int i;

   // TODO: Delete the following args
   // verify correct number of args is passed in
   if(argc != 4) {
      fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
   }

   // TODO: Delete these args
   int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
   int numProd = atoi(argv[2]); /* Number of producer threads */
   int numCons = atoi(argv[3]); /* Number of consumer threads */

   initializeData();

   // create producer threads
   for(i = 0; i < numProd; i++) {
      pthread_create(&tid,&attr,producer,NULL);
    }

   // create consumer threads
   for(i = 0; i < numCons; i++) {
      pthread_create(&tid,&attr,consumer,NULL);
   }

   sleep(mainSleepTime);

   printf("Exit the program\n");
   exit(0);
}
