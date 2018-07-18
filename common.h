#ifndef COMMON_H
#define COMMON_H


#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <evhttp.h>
#include <event2/dns.h>

#include "arraylist.h"
#include "map.h"
#include "queue.h"
#include "bloom.h"
#include "log.h"

#define QUEUE_SIZE 204800
#define INIT_EDGE_SIZE 102400
#define MAX_URL 160000

typedef struct spider_context_s{
    struct event_base* ev;
    struct evdns_base* evdns;
    struct event tick_evt;
    HashTable* urls;
    ArrayList* edges;
    queue_t* req, resp;
    pthread_mutex_t mutex;
    unsigned int requested, seq, visited;
    BloomFilter *bf;
    LOGGER *lg;
}SPIDER_CONTEXT;

#endif
