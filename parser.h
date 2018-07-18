#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <stdlib.h>

// chrome
#define MAX_URL_LEN 8182

enum STATUS{
    START, LEFT, TAGBEGIN, ATAG, HREF, HREF1, HREF2, HREF3, HREF4, URLB, URLE, OTAG
};

typedef struct parser_ctx_s{
    enum STATUS status; // status
    char *buf; // html buffer
    char *url; // url pointer
    int pos;   // position
}PARSER_CTX;

char* parse(PARSER_CTX *ctx, char *buffer, int len);
void url_join(char *dst, size_t len, const char *base, const char *ref);

#endif
