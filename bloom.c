#include <limits.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "bloom.h"
#include "hash.h"

phashfunc func[MAX_HASHNUM];

BloomFilter* bloom_init(){
    BloomFilter *p = (BloomFilter *)malloc(sizeof(BloomFilter));
	p->size = 0;
	p->filter = (unsigned int *) malloc(BITSIZE/8);
    p->mutex = PTHREAD_MUTEX_INITIALIZER;
    memset(p->filter, 0, BITSIZE/8);

    // Hash functions
	func[0] = RSHash;
	func[1] = JSHash;
	func[2] = PJWHash;
	func[3] = ELFHash;
	func[4] = BKDRHash;
	func[5] = SDBMHash;
	func[6] = DJBHash;
	func[7] = DEKHash;
	func[8] = BPHash;
	func[9] = FNVHash;
	func[10] = APHash;

	return p;
}

void bloom_free(BloomFilter *bf){
    free(bf->filter);
    free(bf);
}

void bloom_add(BloomFilter *bf, const char *str){
    pthread_mutex_lock(&(bf->mutex));
    int len = strlen(str);
    for(int i = 0;i<HASHNUM; i++) {
        unsigned int hash = func[i](str,len);
        hash %= BITSIZE;
        SETBIT(bf->filter,hash);
	}
    pthread_mutex_unlock(&(bf->mutex));
}

int bloom_check(BloomFilter *bf, const char *str){
    pthread_mutex_lock(&(bf->mutex));
    int len = strlen(str);
    for(int i= 0;i <HASHNUM; i++) {
        unsigned int hash = func[i](str,len);
        hash %= BITSIZE;
        if(!GETBIT(bf->filter,hash)){
            pthread_mutex_unlock(&(bf->mutex));
            return 0;
        }
	}
    pthread_mutex_unlock(&(bf->mutex));
    return 1;
}
