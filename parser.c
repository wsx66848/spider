#include "parser.h"

// #define PARSER_DEBUG

char* parse(PARSER_CTX *ctx, char *buffer, int len){
    char tmp[2];
    tmp[1] = 0;
    if(ctx->buf != buffer){
        ctx->pos = 0;
        ctx->buf = buffer;
    }
    while(ctx->pos < len ){
        char ch = buffer[ctx->pos++];
        if(!ch) return NULL;
#ifdef PARSER_DEBUG
        printf("%c %d\n", ch, ctx->status);
#endif
        switch(ctx->status){
        case START:
            if(ch == '<') ctx->status = LEFT;
            break;
        case LEFT:
            ctx->status = URLE;
            if(ch == 'a' || ch == 'A') ctx->status = TAGBEGIN;
            if(ch == ' ') ctx->status = START;
            break;
        case TAGBEGIN:
            ctx->status = URLE;
            if(ch == ' ') ctx->status = ATAG;
            break;
        case ATAG:
            switch (ch) {
            case '>':
                ctx->status = START;
                break;
            case 'h':
            case 'H':
                ctx->status = HREF;
                break;
            case ' ':
                break;
            default:
                ctx->status = OTAG;
                break;
            }
            break;
        case OTAG:
            if(ch == ' ') ctx->status = ATAG;
            break;
        case HREF:
            ctx->status = OTAG;
            if(ch == 'r' || ch == 'R') ctx->status = HREF1;
            break;
        case HREF1:
            ctx->status = OTAG;
            if(ch == 'e' || ch == 'E') ctx->status = HREF2;
            break;
        case HREF2:
            ctx->status = OTAG;
            if(ch == 'f' || ch == 'F') ctx->status = HREF3;
            break;
        case HREF3:
            ctx->status = OTAG;
            if(ch == ' ') ctx->status = HREF3;
            if(ch == '=') ctx->status = HREF4;
            break;
        case HREF4:
            ctx->status = OTAG;
            if(ch == ' ') ctx->status = HREF4;
            if(ch == '"'){
                ctx->status = URLB;
                ctx->url[0] = 0;
            }
            break;
        case URLB:
            if(ch == '"'){
                ctx->status = URLE;
                return ctx->url;
            }
            tmp[0] = ch;
            strcat_s(ctx->url, MAX_URL_LEN, tmp);
            break;
        case URLE:
            if(ch == '>') ctx->status = START;
            break;
        }
    }
    return NULL;
}

void url_join(char *dst, size_t len, const char *base, const char *ref){
    char *last;
    int flag = 0;
    // real path
    if(strncmp(ref, "//", 2) == 0){
        strcpy_s(dst, len, "http:");
        strcat_s(dst, len, ref);
        return;
    }
    // abs
    if(strncmp(ref, "/", 1) == 0){
        if(strncmp(base, "http:", 5) == 0){
            // abs
            strcpy_s(dst, len, base);
            if(strlen(base) < 7) return;
            char *begin = strstr(dst+7, "/");
            if(begin){
                *(begin) = 0;
            }
            strcat_s(dst, len, ref);
        }else{
            // relative
            strcpy_s(dst, len, ref);
        }
        return;
    }
    if(strncmp(base, "http:", 5) == 0){
        flag = 1;
    }
    strcpy_s(dst, len, base);
    if(strlen(ref) == 0) return;
    // resolve path
    last = strrchr(dst, '/');
    if(last){
        if(flag && last == dst + 6){
        }else{
            *last = 0;
        }
    }
    while(strlen(ref)){
        char *end = strstr(ref, "/");
        if(!end) break;
        if(strncmp(ref, ".", end-ref) == 0){
            ref = end + 1;
            continue;
        }else if(strncmp(ref, "..", end-ref) == 0){
            last = strrchr(dst, '/');
            if(flag && last == dst + 6){
                break;
            }
            if(!last) break;
            *last = 0;
            ref = end + 1;
        }else{
            break;
        }
    }
    strcat_s(dst, len, "/");
    strcat_s(dst, len, ref);
}
