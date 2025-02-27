#include <stdio.h>
#include <pthread.h>

pthread_t threads[2];
// Thread-local indexed variable to store counter
__thread int counter = 0;


void* task(void * arg){
    for(int i=0; i<10000; i++){
        counter++;
        printf("Thread %ld, sharedCounter: %d\n", (long)arg, counter);
    }

    return NULL;

}



int main(){
    for (long i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, task, (void*)i);
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}