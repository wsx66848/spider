#ifndef __BLOOMFILTER_H__
#define __BLOOMFILTER_H__

#include<stdio.h>
#include<stdlib.h>

#define SETBIT(a, n) (a[n/(sizeof(unsigned int)*8)] |= (1<<(sizeof(unsigned int)*8-n%(sizeof(int)*8)-1)))
#define GETBIT(a, n) (a[n/(sizeof(unsigned int)*8)] & (1<<((sizeof(unsigned int)*8-n%(sizeof(int)*8)-1))))

#define BITSIZE 234881024
#define MAX_HASHNUM 11
#define HASHNUM 11

typedef struct {
    unsigned int size;
    unsigned int *filter;
    pthread_mutex_t mutex;
}BloomFilter;

BloomFilter* bloom_init();
void bloom_free(BloomFilter *bf);
void bloom_add(BloomFilter *bf, const char *str);
int bloom_check(BloomFilter *bf, const char *str);

#endif
