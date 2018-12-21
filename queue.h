#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#ifndef _QUEUE_H
#define _QUEUE_H


typedef struct queue
{
	void **buffer;
    int capacity;
	int size;
	int in;
	int out;
	pthread_mutex_t mutex;
	pthread_cond_t cond_full;
	pthread_cond_t cond_empty;
} queue_t;

queue_t *queue_init(int size);
void queue_free(queue_t *queue);
void queue_enqueue(queue_t *queue, void *value);
void *queue_dequeue(queue_t *queue, const struct timespec *timeout);
int queue_size(queue_t *queue);

#endif
