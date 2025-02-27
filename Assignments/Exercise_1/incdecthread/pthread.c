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
//sched fifo
pthread_attr_t attribute[2];
struct sched_param priority_param[2];


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
    // sem_wait(&semaphore);
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
    // sem_post(&semaphore);
}


void *decThread(void *threadp)
{
    int i;
    struct sched_param schedule_param;
    int policy;

    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_getschedparam(pthread_self(), &policy, &schedule_param);
    printf("ThreadDec | priority = %d \n", schedule_param.sched_priority);

    // sem_wait(&semaphore);
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
    // sem_post(&semaphore);
}


void print_scheduler(void)
{
    int schedType;
    schedType = sched_getscheduler(getpid());
    switch (schedType)
    {
    case SCHED_FIFO:
        printf("Pthread Policy is SCHED_FIFO\n");
        break;
    case SCHED_OTHER:
        printf("Pthread Policy is SCHED_OTHER\n");
        break;
    case SCHED_RR:
        printf("Pthread Policy is SCHED_OTHER\n");
        break;
    default:
        printf("Pthread Policy is UNKNOWN\n");
    }
}

int main (int argc, char *argv[])
{
   int rc;
   int i=0;
    // sem_init(&semaphore, false, 1);

    pthread_attr_t attribute_flags_for_main; // for schedular type, priority
    struct sched_param main_priority_param;
    cpu_set_t cpuset;
    int target_cpu = 1; // core we want to run our process on

    CPU_ZERO(&cpuset);
    CPU_SET(target_cpu, &cpuset); 
    

    printf("Before adjustments to scheduling policy:\n");
    print_scheduler();

    pthread_attr_init(&attribute_flags_for_main);
    pthread_attr_init(&attribute[0]);
    pthread_attr_init(&attribute[1]);

   

    int rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    int rt_min_prio = sched_get_priority_min(SCHED_FIFO);

    main_priority_param.sched_priority = rt_max_prio;
    priority_param[0].sched_priority = rt_max_prio-1;
    priority_param[1].sched_priority = rt_max_prio-2;

    pthread_attr_setinheritsched(&attribute_flags_for_main, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute_flags_for_main, SCHED_FIFO);

    pthread_attr_setinheritsched(&attribute[0], PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute[0], SCHED_FIFO);
    pthread_attr_setschedparam(&attribute[0], &priority_param[0]);
    pthread_attr_setaffinity_np(&attribute[0], sizeof(cpu_set_t), &cpuset);

    pthread_attr_setinheritsched(&attribute[1], PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute[1], SCHED_FIFO);
    pthread_attr_setschedparam(&attribute[1], &priority_param[1]);
    pthread_attr_setaffinity_np(&attribute[1], sizeof(cpu_set_t), &cpuset);


    int status_setting_schedular = sched_setscheduler(getpid(), SCHED_FIFO, &main_priority_param);
    if (status_setting_schedular)
    {
        printf("ERROR; sched_setscheduler rc is %d\n", status_setting_schedular);
        perror(NULL);
        exit(-1);
    }

    printf("After adjustments to scheduling policy:\n");
    print_scheduler();


   threadParams[i].threadIdx=i;
   threadParams[i].name = "parth";

   pthread_create(&threads[i],   // pointer to thread descriptor
                  &attribute[i],     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   threadParams[i].threadIdx=i;
   threadParams[i].name = "thakkar";
   pthread_create(&threads[i], &attribute[i], decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
//    sem_destroy(&semaphore);
}
