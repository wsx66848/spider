#include "log.h"

LOGGER* logger_init(const char * filename, const char * mode){
    LOGGER* lg = (LOGGER*)malloc(sizeof(LOGGER));
    lg->fd = fopen(filename, mode);
    lg->mutex = PTHREAD_MUTEX_INITIALIZER;
    return lg;
}

void logger_free(LOGGER *lg){
    fflush(lg->fd);
    fclose(lg->fd);
    free(lg);
}
