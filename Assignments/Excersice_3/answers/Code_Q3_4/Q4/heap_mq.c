 /*
  * Author: Sam Siewart for heap_mq.c code in Exercise3/Posix_MQ_loop
  * Modified by: Shashank and Parth
  * References:  
  * 1. Sam Siewert - 10/14/97 heap_mq.c - vxWorks code
  * 2. heap_mq.c code in Exercise3/Posix_MQ_loop used as the base
  */       

#define _GNU_SOURCE 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>

// On Linux the file systems slash is needed
#define SNDRCV_MQ "/send_receive_mq"

#define ERROR (-1)

#define NUM_CPUS (1)

pthread_t th_receive, th_send; // create threads
pthread_attr_t attr_receive, attr_send;
struct sched_param param_receive, param_send;

static char imagebuff[4096];
struct mq_attr mq_attr;
mqd_t mymq;

/* receives pointer to heap, reads it, and deallocate heap memory */
void *receiver(void *arg)
{
  void *buffptr; 
  char buffer[sizeof(void *)+sizeof(int)];
  int prio;
  int nbytes;
  int id;

  cpu_set_t cpuset;  
  CPU_ZERO(&cpuset);
 
  printf("receiver - thread entry\n");

  /* read oldest, highest priority msg from the message queue until empty */
  while(1)
  {
    printf("receiver - awaiting message\n");

    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == ERROR)
    {
      perror("mq_receive");
    }
    else
    {
      memcpy(&buffptr, &buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      printf("receiver - ptr msg 0x%p received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);
      printf("receiver - Contents of ptr = \n%s\n", (char *)buffptr);
      
      free(buffptr);
      printf("receiver - heap space memory freed\n");
    }
  } 
}

/*send pointer to heap which points to the data in imagebuff*/
void *sender(void *arg)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio;
  int nbytes;
  int id = 999;

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset); 

  printf("sender - thread entry\n");

  while(1)
  {
    buffptr = (void *)malloc(sizeof(imagebuff));
    strcpy(buffptr, imagebuff);
    printf("sender - Message to send = %s\n", (char *)buffptr);
    printf("sender - Sending message of size=%d\n", sizeof(buffptr));

    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

      if((nbytes = mq_send(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), 30)) == ERROR)
      {
        perror("mq_send");
      }
      else
      {
        printf("sender - message ptr 0x%p successfully sent\n", buffptr);
      }
  }
  
}

/*Fills imagebuff with ASCII data */
void fillbuffer(void)
{
  int i, j;
  char pixel = 'A';

  for(i=0;i<4096;i+=64) 
  {
    pixel = 'A';
    for(j=i;j<i+64;j++) 
    {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';
}


void main(void)
{
  int i=0, rc=0;

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  for(i=0; i < NUM_CPUS; i++)
      CPU_SET(i, &cpuset);

  fillbuffer();
  
  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 10;
  mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);

  mq_attr.mq_flags = 0;

  mq_unlink(SNDRCV_MQ);  //Unlink if the previous message queue exists

  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, S_IRWXU, &mq_attr);
  if(mymq == (mqd_t)ERROR)
  {
    perror("mq_open");
  }
    
  int rt_max_prio, rt_min_prio;
  rt_max_prio = sched_get_priority_max(SCHED_FIFO);
  rt_min_prio = sched_get_priority_min(SCHED_FIFO);
  
  //creating prioritized thread

  //initialize  with default atrribute
  rc = pthread_attr_init(&attr_receive);
  //specific scheduling for Receiving
  rc = pthread_attr_setinheritsched(&attr_receive, PTHREAD_EXPLICIT_SCHED);
  rc = pthread_attr_setschedpolicy(&attr_receive, SCHED_FIFO); 
  rc=pthread_attr_setaffinity_np(&attr_receive, sizeof(cpu_set_t), &cpuset); 
  param_receive.sched_priority = rt_min_prio;
  pthread_attr_setschedparam(&attr_receive, &param_receive);
  
  //initialize  with default atrribute
  rc = pthread_attr_init(&attr_send);
  //specific scheduling for Sending
  rc = pthread_attr_setinheritsched(&attr_send, PTHREAD_EXPLICIT_SCHED);
  rc = pthread_attr_setschedpolicy(&attr_send, SCHED_FIFO); 
  rc=pthread_attr_setaffinity_np(&attr_send, sizeof(cpu_set_t), &cpuset);  //SC Added
  param_send.sched_priority = rt_max_prio;
  pthread_attr_setschedparam(&attr_send, &param_send);
  
  if((rc=pthread_create(&th_send, &attr_send, sender, NULL)) == 0)
  {
    printf("\n\rSender Thread Created with rc=%d\n\r", rc);
  }
  else 
  {
    perror("\n\rFailed to Make Sender Thread\n\r");
    printf("rc=%d\n", rc);
  }

  if((rc=pthread_create(&th_receive, &attr_receive, receiver, NULL)) == 0)
  {
    printf("\n\r Receiver Thread Created with rc=%d\n\r", rc);
  }
  else
  {
    perror("\n\r Failed Making Reciever Thread\n\r"); 
    printf("rc=%d\n", rc);
  }

  printf("pthread join send\n");  
  pthread_join(th_send, NULL);

  printf("pthread join receive\n");  
  pthread_join(th_receive, NULL);
}
