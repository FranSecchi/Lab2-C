#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

typedef struct {
    uint32_t * data_ptr;
    pthread_mutex_t * mutex1;
    pthread_mutex_t * mutex2;
} thread_data_t;


void* bounce(void* arg, int thread_id) {
    thread_data_t* data = (thread_data_t*) arg;
    uint32_t* count = data->data_ptr;
    pthread_mutex_t* mutex1 = data->mutex1;
    pthread_mutex_t* mutex2 = data->mutex2;
    
    uint32_t bounce_count = *count;
    printf("thread%d begins, %d.\n", thread_id, bounce_count);
    while (bounce_count > 0) {
        pthread_mutex_lock(mutex1);
        if (*count == 0) {
            pthread_mutex_unlock(mutex2);
            break;
        }
        (*count)--;
        printf("thread%d bounce %d.\n", thread_id, *count);
        pthread_mutex_unlock(mutex2);
    }
    printf("thread%d ends.\n", thread_id);
    pthread_mutex_unlock(mutex1);
    pthread_exit(NULL);
}
void * thread_func1(void * arg) {
    thread_data_t * data = (thread_data_t *) arg;
    uint32_t * val = data->data_ptr;
    pthread_mutex_t * mutex1 = data->mutex1;
    pthread_mutex_t * mutex2 = data->mutex2;
    bounce(data, 1);
    pthread_exit(NULL);
}

void * thread_func2(void * arg) {
    thread_data_t * data = (thread_data_t *) arg;
    uint32_t * val = data->data_ptr;
    pthread_mutex_t * mutex1 = data->mutex1;
    pthread_mutex_t * mutex2 = data->mutex2;
    bounce(data, 2);
    pthread_exit(NULL);
}
int main() {
    uint32_t data = rand()%20 + 11;
    pthread_mutex_t mutex1, mutex2;
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_t thread1, thread2;
    thread_data_t data1 = {&data, &mutex1, &mutex2};
    thread_data_t data2 = {&data, &mutex2, &mutex1};
    printf("main: bouncing for %d times.\n", data);
    pthread_create(&thread1, NULL, thread_func1, &data1);
    pthread_create(&thread2, NULL, thread_func2, &data2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    exit(0);
}
