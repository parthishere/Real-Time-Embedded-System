#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>


pthread_mutex_t mutex;


typedef struct {
    int threadId;
} thread_params_t;

int parth = 0;

void * handle(void * args){
    thread_params_t *params = (thread_params_t *) args;
    printf("tim gendu hai %d pid: %d\n", params->threadId, getpid());

    for (int i=0; i<20000000; i++){
        pthread_mutex_lock(&mutex);
        parth++;
        // pthread_mutex_unlock(&mutex);
    }


    // pid_t pid = fork();

    // if(pid == -1){
    //     perror("print");
    //     exit(1);

    // }

    // if(pid == 0){
    //     printf("child inside thread\n");
    // }else{
    //     printf("main process iun the thread \n");
    //     wait(NULL);
    // }
}

int main(){
    

    int status = 0;

    pid_t pid = fork();
    pthread_t thread_arr[10];
    thread_params_t thread_params[10];
    if(pid == -1){
        exit(1);
    }

    

    if(pid != 0){
        printf("pid of child in wait %d\n", wait(&status));
        printf("status :%d \n", status);
        for(int i=5; i<7 ;i++){
            thread_params[i].threadId = i;
        }
        for(int i=5; i<7 ;i++){
            pthread_create(&thread_arr[i], NULL, handle, &(thread_params[i]));
        }
        for(int i=5; i<7 ;i++){
            pthread_join(thread_arr[i], NULL);
        }
        
    }else{
        pthread_mutex_init(&mutex, NULL);
        printf("will only be executed by child \n");
        for(int i=0; i<2 ;i++){
            thread_params[i].threadId = i;
        }
        for(int i=0; i<2 ;i++){
            pthread_create(&thread_arr[i], NULL, handle, (void *)&(thread_params[i]));
        }
        for(int i=0; i<2 ;i++){
            pthread_join(thread_arr[i], NULL);
        }
        pthread_mutex_destroy(&mutex);
    }
    printf("parth : %d \n", parth);
 
    
  


    return 0;
    

}