#include <stdio.h>
#include <pthread.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t threads[2];

int sharedCounter = 0;

void* task(void * arg){
    for(int i=0; i<10000; i++){
        pthread_mutex_lock(&mutex);
        sharedCounter++;
        printf("Thread %ld, sharedCounter: %d\n", (long)arg, sharedCounter);
        // Unlock the mutex after updating
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


int main(){
    pthread_mutex_init(&mutex, NULL);

    for (long i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, task, (void*)i);
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}