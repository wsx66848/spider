#ifndef LIBHTTP_H
#define LIBHTTP_H

#include "common.h"
#include "parser.h"

struct http_context {
    SPIDER_CONTEXT* ctx;
    struct evhttp_uri *uri;
    char *url;
    struct evhttp_connection *con;
    struct evhttp_request *req;
    PARSER_CTX *parser;
};

int http_request(SPIDER_CONTEXT* ctx, const char *url);
int async_http_request(SPIDER_CONTEXT* ctx, const char *url);

#endif
