#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>

sem_t semaphore;

#define COUNT  1000

typedef struct
{
    char * name;
    int threadIdx;
} threadParams_t;

// struct sched 
// POSIX thread declarations and scheduling attributes
//
pthread_t threads[2];
threadParams_t threadParams[2];


// Unsafe global
int gsum=0;

void *incThread(void *threadp)
{
    int i;
    struct sched_param schedule_param;
    int policy;

    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_getschedparam(pthread_self(), &policy, &schedule_param);
    printf("ThreadInc | priority = %d \n", schedule_param.sched_priority);
    sem_wait(&semaphore);
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
    sem_post(&semaphore);
}


void *decThread(void *threadp)
{
    int i;
    struct sched_param schedule_param;
    int policy;

    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_getschedparam(pthread_self(), &policy, &schedule_param);
    printf("ThreadDec | priority = %d \n", schedule_param.sched_priority);

    sem_wait(&semaphore);
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
    sem_post(&semaphore);
}


int main (int argc, char *argv[])
{
   int rc;
   int i=0;
    sem_init(&semaphore, false, 1);


   threadParams[i].threadIdx=i;
   threadParams[i].name = "parth";

   pthread_create(&threads[i],   // pointer to thread descriptor
                  NULL,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   threadParams[i].threadIdx=i;
   threadParams[i].name = "thakkar";
   pthread_create(&threads[i], NULL, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
   sem_destroy(&semaphore);
}
