#include "common.h"
#include "http.h"
#include "map.h"

SPIDER_CONTEXT* spider_init(){
#ifdef WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif
    SPIDER_CONTEXT* ctx = (SPIDER_CONTEXT*)malloc(sizeof(SPIDER_CONTEXT));
#ifdef WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
    signal(SIGPIPE, SIG_IGN);
#endif
    ctx->requested = 0;
    ctx->seq = 0;
    ctx->visited = 0;
    ctx->bf = bloom_init();
    ctx->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    ctx->urls = hash_table_new();
    ctx->edges = arraylist_new(INIT_EDGE_SIZE);
    ctx->req = queue_init(QUEUE_SIZE);
    ctx->ev = event_base_new();
    if (!ctx->ev)
    {
        fprintf(stderr, "initialize libevent failed!\n");
        return NULL;
    }

    ctx->evdns = evdns_base_new(ctx->ev, EVDNS_BASE_INITIALIZE_NAMESERVERS);
    if (!ctx->evdns)
    {
        fprintf(stderr, "initialize dns resolver failed!\n");
        return NULL;
    }
    return ctx;
}

void spider_free(SPIDER_CONTEXT* ctx){
    evdns_base_free(ctx->evdns, 0);
    event_base_free(ctx->ev);
    queue_free(ctx->req);
    arraylist_free(ctx->edges);
    hash_table_free(ctx->urls);
    bloom_free(ctx->bf);
    free(ctx);
#ifdef WIN32
    WSACleanup();
#endif
}

void spider_worker(SPIDER_CONTEXT* ctx){
    char* url;
    int ret;
    struct timespec timeout;
    timeout.tv_nsec = 0;
    timeout.tv_sec = 1;
    while(1){
        pthread_mutex_lock(&(ctx->mutex));
        unsigned int req = ctx->requested;
        pthread_mutex_unlock(&(ctx->mutex));
        if(req >= 100){
            usleep(1000);
            continue;
        }
        url = (char*)queue_dequeue(ctx->req, &timeout);
        if(!url) break;
        if(url == ETIMEDOUT){
            continue;
        }
        ret = http_request(ctx, url);
        if(!ret){
            pthread_mutex_lock(&(ctx->mutex));
            ctx->requested++;
            pthread_mutex_unlock(&(ctx->mutex));
        }
    }
}

void check_state(int sock, short event, SPIDER_CONTEXT *ctx)
{
    pthread_mutex_lock(&(ctx->mutex));
    unsigned int req = ctx->requested;
    unsigned int seq = ctx->seq;
    unsigned int visited = ctx->visited;
    pthread_mutex_unlock(&(ctx->mutex));
    int queued = queue_size(ctx->req);
    if(req+queued <= 0){
        event_base_loopbreak(ctx->ev);
    }
    fprintf(stderr, "[msg] total: %u, OK: %u, sending: %u, queued: %d\n",
           seq, visited, req, queued);
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0 };
    event_add(&ctx->tick_evt, &tv);
}

void ev_worker(SPIDER_CONTEXT* ctx){
    // tick
    evtimer_set(&ctx->tick_evt, check_state, ctx);
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0 };
    event_base_set(ctx->ev, &ctx->tick_evt);
    evtimer_add(&ctx->tick_evt, &tv);
    event_base_dispatch(ctx->ev);
    // notify to break
    queue_enqueue(ctx->req, NULL);
}

int main(int argc, char** argv){
    int ret;
    if(argc < 4){
        printf("usage: crawler ip port url.txt\n");
        return 0;
    }
    pthread_t t_spider, t_ev;
    SPIDER_CONTEXT* ctx = spider_init();
    if(!ctx){
        return -1;
    }
    ctx->lg = logger_init(argv[3], "w");
    char *init_url = (char*)malloc(256);
    strcpy(init_url, "http://");
    strcat_s(init_url, 256, argv[1]);
    if(strcmp(argv[2], "80")){
        strcat_s(init_url, 256, ":");
        strcat_s(init_url, 256, argv[2]);
    }
    strcat_s(init_url, 256, "/");
    printf("start: %s\n", init_url);
    async_http_request(ctx, init_url);

    ret = pthread_create(&t_ev, NULL, (void*)ev_worker, (void*)ctx);
    if (ret){
        printf("Create pthread error!/n");
        return 1;
    }
    ret = pthread_create(&t_spider, NULL, (void*)spider_worker, (void*)ctx);
    if (ret){
        printf("Create pthread error!/n");
        return 1;
    }

    pthread_join(t_ev, NULL);
    pthread_join(t_spider, NULL);

    printf("All done\n");
    // printf edges
    fprintf(ctx->lg->fd, "\n");
    for(int i=0; i<ctx->edges->length; i++){
        fprintf(ctx->lg->fd, "%d %d\n", ctx->edges->data[i].in, ctx->edges->data[i].out);
    }

    logger_free(ctx->lg);
    spider_free(ctx);
    return 0;
}

