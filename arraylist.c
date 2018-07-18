#include <stdlib.h>
#include <string.h>

#include "arraylist.h"

/* Automatically resizing array */

ArrayList *arraylist_new(unsigned int length)
{
	ArrayList *new_arraylist;

	/* If the length is not specified, use a sensible default */

	if (length <= 0) {
		length = 16;
	}

	/* Allocate the new ArrayList and fill in the fields.  There are
	 * initially no entries. */

	new_arraylist = (ArrayList *) malloc(sizeof(ArrayList));

	if (new_arraylist == NULL) {
		return NULL;
	}

	new_arraylist->_alloced = length;
	new_arraylist->length = 0;

	/* Allocate the data array */

	new_arraylist->data = malloc(length * sizeof(ArrayListValue));

	if (new_arraylist->data == NULL) {
		free(new_arraylist);
		return NULL;
	}

	return new_arraylist;
}

void arraylist_free(ArrayList *arraylist)
{
	/* Do not free if a NULL pointer is passed */

	if (arraylist != NULL) {
		free(arraylist->data);
		free(arraylist);
	}
}

static int arraylist_enlarge(ArrayList *arraylist)
{
	ArrayListValue *data;
	unsigned int newsize;

	/* Double the allocated size */

	newsize = arraylist->_alloced * 2;

	/* Reallocate the array to the new size */

	data = realloc(arraylist->data, sizeof(ArrayListValue) * newsize);

	if (data == NULL) {
		return 0;
	} else {
		arraylist->data = data;
		arraylist->_alloced = newsize;

		return 1;
	}
}

int arraylist_insert(ArrayList *arraylist, unsigned int index,
                     unsigned int in, unsigned int out)
{
	/* Sanity check the index */

	if (index > arraylist->length) {
		return 0;
	}

	/* Increase the size if necessary */

	if (arraylist->length + 1 > arraylist->_alloced) {
		if (!arraylist_enlarge(arraylist)) {
			return 0;
		}
	}

	/* Move the contents of the array forward from the index
	 * onwards */

	memmove(&arraylist->data[index + 1],
	        &arraylist->data[index],
	        (arraylist->length - index) * sizeof(ArrayListValue));

	/* Insert the new entry at the index */

    arraylist->data[index].in = in;
    arraylist->data[index].out = out;
    ++arraylist->length;

	return 1;
}

int arraylist_append(ArrayList *arraylist, unsigned int in, unsigned int out)
{
    return arraylist_insert(arraylist, arraylist->length, in, out);
}

void arraylist_clear(ArrayList *arraylist)
{
	/* To clear the list, simply set the length to zero */

	arraylist->length = 0;
}
