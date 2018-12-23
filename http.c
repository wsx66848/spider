#include "http.h"

void http_ctx_free(struct http_context *http_ctx){
    evhttp_connection_free(http_ctx->con);
    evhttp_uri_free(http_ctx->uri);
    free(http_ctx->url);
    free(http_ctx->parser->url);
    free(http_ctx->parser);
    free(http_ctx);
}

void RemoteReadCallback(struct evhttp_request* remote_rsp, void* arg){
    struct http_context* http_ctx = (struct http_context*)arg;
    struct event_base *ev = http_ctx->ctx->ev;
    pthread_mutex_lock(&(http_ctx->ctx->mutex));
    http_ctx->ctx->requested--;
    http_ctx->ctx->visited++;
    pthread_mutex_unlock(&(http_ctx->ctx->mutex));
    http_ctx_free(http_ctx);
    event_base_loopcontinue(ev);
}

int async_http_request(SPIDER_CONTEXT* ctx, const char *url){
    if(!bloom_check(ctx->bf, url)){
        bloom_add(ctx->bf, url);
        pthread_mutex_lock(&(ctx->mutex));
        if(ctx->seq >= MAX_URL){
            pthread_mutex_unlock(&(ctx->mutex));
            return 0;
        }
        ctx->seq++;
        hash_table_insert(ctx->urls, url, ctx->seq);
        pthread_mutex_unlock(&(ctx->mutex));

        pthread_mutex_lock(&ctx->lg->mutex);
        fprintf(ctx->lg->fd, "%s %d\n", url, ctx->seq);
        pthread_mutex_unlock(&ctx->lg->mutex);
        return queue_try_enqueue(ctx->req, url);
    }
    return 0;
}

void add_edge(SPIDER_CONTEXT* ctx, const char *src, const char *dst){
    int in, out;
    pthread_mutex_lock(&(ctx->mutex));
    in = hash_table_lookup(ctx->urls, src);
    out = hash_table_lookup(ctx->urls, dst);
    if(in && out){
        arraylist_append(ctx->edges, in, out);
    }
    pthread_mutex_unlock(&(ctx->mutex));
}

void ReadChunkCallback(struct evhttp_request* remote_rsp, void* arg){
    struct http_context* http_ctx = (struct http_context*)arg;
    char buf[4096], *url;
    int code = evhttp_request_get_response_code(remote_rsp);
    if(code == HTTP_OK){
        struct evkeyvalq * headers = evhttp_request_get_input_headers(remote_rsp);
        char* ct = evhttp_find_header(headers, "Content-Type");
        if(ct == NULL || strstr(ct, "html") == NULL){
            //printf("[msg]unknown content type: %s %s\n", ct, http_ctx->url);
            evhttp_cancel_request(http_ctx->req);
            pthread_mutex_lock(&(http_ctx->ctx->mutex));
            http_ctx->ctx->requested--;
            http_ctx->ctx->visited++;
            pthread_mutex_unlock(&(http_ctx->ctx->mutex));
            http_ctx_free(http_ctx);
            return;
        }
        struct evbuffer* evbuf = evhttp_request_get_input_buffer(remote_rsp);
        int n = 0;
        while ((n = evbuffer_remove(evbuf, buf, 4096)) > 0){
            http_ctx->parser->pos = 0;
            while((url = parse(http_ctx->parser, buf, n))){
                if(!strlen(url)){
                     continue;
                }
                int len = strlen(url) + strlen(http_ctx->url) + 1 + 20;
                char *new_url = (char*)malloc(len);
                if(strncmp(url, "http:", 5)==0){
                    // link
                    strcpy_s(new_url, len, url);
                }else if(strncmp(url, "https:", 6) == 0){
                   free(new_url);
                   continue;
                }else if(strncmp(url, "javascript:", 11) == 0){
                   free(new_url);
                   continue;
                }else if(strncmp(url, "#", 1) == 0){
                   free(new_url);
                   continue;
                }else if(strpbrk(url, "' \t\n") != 0){
                   free(new_url);
                   continue;
                }else{
                    url_join(new_url, len, http_ctx->url, url);
                }
                if(!strlen(new_url)){
                    free(new_url);
                    continue;
                }
                struct evhttp_uri *uri = evhttp_uri_parse(new_url);
                if(uri){
                    evhttp_uri_free(uri);
                    add_edge(http_ctx->ctx, http_ctx->url , new_url);
                    if(!async_http_request(http_ctx->ctx, new_url)){
                        free(new_url);
                    }
                }else{
                    free(new_url);
                }
            }
        }
    }else{
       //printf("[msg]bad response code: %d %s\n", code, http_ctx->url);
    }
}


int http_request(SPIDER_CONTEXT* ctx, const char *url){
    struct http_context* http_ctx = (struct http_context*)malloc(sizeof(struct http_context));
    char *path = url;
    http_ctx->ctx = ctx;
    http_ctx->url = url;
    http_ctx->parser = (PARSER_CTX*)malloc(sizeof(PARSER_CTX));
    http_ctx->parser->status = START;
    http_ctx->parser->buf = NULL;
    http_ctx->parser->url = (char *)malloc(MAX_URL_LEN + 20);

    http_ctx->uri = evhttp_uri_parse(url);
    if (!http_ctx->uri){
        free(http_ctx->url);
        free(http_ctx->parser->url);
        free(http_ctx->parser);
        free(http_ctx);
        return 1;
    }


    http_ctx->req = evhttp_request_new(RemoteReadCallback, http_ctx);
    if (!http_ctx->req){
        evhttp_uri_free(http_ctx->uri);
        free(http_ctx->url);
        free(http_ctx->parser->url);
        free(http_ctx->parser);
        free(http_ctx);
        return 2;
    }

    evhttp_request_set_chunked_cb(http_ctx->req, ReadChunkCallback);

    const char* host = evhttp_uri_get_host(http_ctx->uri);
    if (!host){
        evhttp_request_free(http_ctx->req);
        evhttp_uri_free(http_ctx->uri);
        free(http_ctx->url);
        free(http_ctx->parser->url);
        free(http_ctx->parser);
        free(http_ctx);
        return 3;
    }

    int port = evhttp_uri_get_port(http_ctx->uri);
    if (port <= 0) {
        port = 80;
    }

    if(strncmp(url,"http://", 7) == 0){
        path = strstr(url+7,"/");
    }
    if(!path){
        path = "/";
    }

    http_ctx->con =  evhttp_connection_base_new(ctx->ev, ctx->evdns, host, port);
    if (!http_ctx->con){
        evhttp_request_free(http_ctx->req);
        evhttp_connection_free(http_ctx->con);
        evhttp_uri_free(http_ctx->uri);
        free(http_ctx->url);
        free(http_ctx->parser->url);
        free(http_ctx->parser);
        free(http_ctx);
        return 4;
    }

    struct evkeyvalq *headers = evhttp_request_get_output_headers(http_ctx->req);
    evhttp_add_header(headers, "Accept", "text/html,application/xhtml+xml,application/xml");
    evhttp_add_header(headers, "Accept-Language", "zh-CN,zh;q=0.9");
    evhttp_add_header(headers, "Cache-Control", "no-cache");
    evhttp_add_header(headers, "Connection" ,"keep-alive");
    evhttp_add_header(headers, "Host", host);
    evhttp_add_header(headers, "Pragma" ,"no-cache");
    evhttp_add_header(headers, "User-Agent" ,"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; QQDownload 732; .NET4.0C; .NET4.0E; LBBROWSER)");
    evhttp_connection_set_timeout(http_ctx->con, 30);

    int ret = evhttp_make_request(http_ctx->con, http_ctx->req, EVHTTP_REQ_GET, path);
    if(ret){
        evhttp_connection_free(http_ctx->con);
        evhttp_uri_free(http_ctx->uri);
        free(http_ctx->url);
        free(http_ctx->parser->url);
        free(http_ctx->parser);
        free(http_ctx);
    }
    return ret;
}
