#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct logger_s{
    FILE *fd;
    pthread_mutex_t mutex;
}LOGGER;

LOGGER* logger_init(const char * filename, const char * mode);
void logger_free(LOGGER *lg);

#endif
