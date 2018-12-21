#include "queue.h"
#include "stdlib.h"

queue_t *queue_init(int size){
    void **buf = (void **)malloc(sizeof(void*) * size);
    queue_t *queue = (queue_t*)malloc(sizeof(queue_t));
    queue->buffer = buf;
    queue->capacity = size;
    queue->size = 0;
    queue->in = 0;
    queue->out = 0;
    queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->cond_empty = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    queue->cond_full = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    return queue;
}

void queue_free(queue_t *queue){
    free(queue->buffer);
    free(queue);
}

void queue_enqueue(queue_t *queue, void *value){
	pthread_mutex_lock(&(queue->mutex));
    while (queue->size == queue->capacity){
        pthread_cond_wait(&(queue->cond_full), &(queue->mutex));
    }
	queue->buffer[queue->in] = value;
	++ queue->size;
	++ queue->in;
	queue->in %= queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));
	pthread_cond_broadcast(&(queue->cond_empty));
}

void *queue_dequeue(queue_t *queue, const struct timespec *timeout){
    int ret = 0;
    struct timespec abstimeout;
    struct timeval now;
    if (timeout) {
        gettimeofday(&now, NULL);
        abstimeout.tv_sec = now.tv_sec + timeout->tv_sec;
        abstimeout.tv_nsec = (now.tv_usec * 1000) + timeout->tv_nsec;
        if (abstimeout.tv_nsec >= 1000000000) {
            abstimeout.tv_sec++;
            abstimeout.tv_nsec -= 1000000000;
        }
    }
	pthread_mutex_lock(&(queue->mutex));
    /* Will wait until awakened by a signal or broadcast */
    while (queue->size ==0 && ret != ETIMEDOUT) {  //Need to loop to handle spurious wakeups
        if (timeout) {
            ret = pthread_cond_timedwait(&queue->cond_empty, &queue->mutex, &abstimeout);
        } else {
            pthread_cond_wait(&queue->cond_empty, &queue->mutex);
        }
    }
    if (ret == ETIMEDOUT) {
        pthread_mutex_unlock(&queue->mutex);
        return ret;
    }
	void *value = queue->buffer[queue->out];
	-- queue->size;
	++ queue->out;
	queue->out %= queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));
	pthread_cond_broadcast(&(queue->cond_full));
	return value;
}

int queue_size(queue_t *queue){
	pthread_mutex_lock(&(queue->mutex));
	int size = queue->size;
	pthread_mutex_unlock(&(queue->mutex));
	return size;
}

int queue_try_enqueue(queue_t *queue, void *value){
    pthread_mutex_lock(&(queue->mutex));
    if (queue->size == queue->capacity){
        pthread_mutex_unlock(&(queue->mutex));
        return 0;
    }
    queue->buffer[queue->in] = value;
    ++ queue->size;
    ++ queue->in;
    queue->in %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_empty));
    return 1;
}
