#ifndef ALGORITHM_ARRAYLIST_H
#define ALGORITHM_ARRAYLIST_H

typedef struct {
    unsigned int in, out;
}ArrayListValue;


typedef struct _ArrayList ArrayList;

struct _ArrayList {

	/** Entries in the array */

	ArrayListValue *data;

	/** Length of the array */

	unsigned int length;

	/** Private data and should not be accessed */

	unsigned int _alloced;
};

ArrayList *arraylist_new(unsigned int length);
void arraylist_free(ArrayList *arraylist);
int arraylist_append(ArrayList *arraylist, unsigned int in, unsigned int out);
void arraylist_clear(ArrayList *arraylist);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_ARRAYLIST_H */

