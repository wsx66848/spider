﻿#include <stdlib.h>
#include <string.h>

#include "map.h"

struct _HashTableEntry {
	HashTablePair pair;
	HashTableEntry *next;
};

struct _HashTable {
	HashTableEntry **table;
	unsigned int table_size;
	unsigned int entries;
	unsigned int prime_index;
};

/* This is a set of good hash table prime numbers, from:
 *   http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 * Each prime is roughly double the previous value, and as far as
 * possible from the nearest powers of two. */

static const unsigned int hash_table_primes[] = {
	193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869, 3145739, 6291469,
	12582917, 25165843, 50331653, 100663319, 201326611,
	402653189, 805306457, 1610612741,
};

static const unsigned int hash_table_num_primes
	= sizeof(hash_table_primes) / sizeof(int);

unsigned int string_nocase_hash(char *string)
{
    unsigned int result = 5381;
    unsigned char *p;

    p = (unsigned char *) string;

    while (*p != '\0') {
        result = (result << 5) + result + (unsigned int) tolower(*p);
        ++p;
    }

    return result;
}

/* Internal function used to allocate the table on hash table creation
 * and when enlarging the table */

static int hash_table_allocate_table(HashTable *hash_table)
{
	unsigned int new_table_size;

	/* Determine the table size based on the current prime index.
	 * An attempt is made here to ensure sensible behavior if the
	 * maximum prime is exceeded, but in practice other things are
	 * likely to break long before that happens. */

	if (hash_table->prime_index < hash_table_num_primes) {
		new_table_size = hash_table_primes[hash_table->prime_index];
	} else {
		new_table_size = hash_table->entries * 10;
	}

	hash_table->table_size = new_table_size;

	/* Allocate the table and initialise to NULL for all entries */

	hash_table->table = calloc(hash_table->table_size,
	                           sizeof(HashTableEntry *));

	return hash_table->table != NULL;
}

/* Free an entry, calling the free functions if there are any registered */

static void hash_table_free_entry(HashTable *hash_table, HashTableEntry *entry)
{
	HashTablePair *pair;

	pair = &(entry->pair);

    free(pair->key);

	/* Free the data structure */

	free(entry);
}

HashTable *hash_table_new()
{
	HashTable *hash_table;

	/* Allocate a new hash table structure */

	hash_table = (HashTable *) malloc(sizeof(HashTable));

	if (hash_table == NULL) {
		return NULL;
	}

	hash_table->entries = 0;
	hash_table->prime_index = 0;

	/* Allocate the table */

	if (!hash_table_allocate_table(hash_table)) {
		free(hash_table);

		return NULL;
	}

	return hash_table;
}

void hash_table_free(HashTable *hash_table)
{
	HashTableEntry *rover;
	HashTableEntry *next;
	unsigned int i;

	/* Free all entries in all chains */

	for (i=0; i<hash_table->table_size; ++i) {
		rover = hash_table->table[i];
		while (rover != NULL) {
			next = rover->next;
			hash_table_free_entry(hash_table, rover);
			rover = next;
		}
	}

	/* Free the table */

	free(hash_table->table);

	/* Free the hash table structure */

	free(hash_table);
}

static int hash_table_enlarge(HashTable *hash_table)
{
	HashTableEntry **old_table;
	unsigned int old_table_size;
	unsigned int old_prime_index;
	HashTableEntry *rover;
	HashTablePair *pair;
	HashTableEntry *next;
	unsigned int index;
	unsigned int i;

	/* Store a copy of the old table */

	old_table = hash_table->table;
	old_table_size = hash_table->table_size;
	old_prime_index = hash_table->prime_index;

	/* Allocate a new, larger table */

	++hash_table->prime_index;

	if (!hash_table_allocate_table(hash_table)) {

		/* Failed to allocate the new table */

		hash_table->table = old_table;
		hash_table->table_size = old_table_size;
		hash_table->prime_index = old_prime_index;

		return 0;
	}

	/* Link all entries from all chains into the new table */

	for (i=0; i<old_table_size; ++i) {
		rover = old_table[i];

		while (rover != NULL) {
			next = rover->next;

			/* Fetch rover HashTablePair */

			pair = &(rover->pair);

			/* Find the index into the new table */

            index = string_nocase_hash(pair->key) % hash_table->table_size;

			/* Link this entry into the chain */

			rover->next = hash_table->table[index];
			hash_table->table[index] = rover;

			/* Advance to next in the chain */

			rover = next;
		}
	}

	/* Free the old table */

	free(old_table);

	return 1;
}

int hash_table_insert(HashTable *hash_table, char *key,
                      int value)
{
	HashTableEntry *rover;
	HashTablePair *pair;
	HashTableEntry *newentry;
	unsigned int index;
    int len = strlen(key) + 1;
    char *newkey = malloc(len);
    memcpy(newkey, key, len);

	/* If there are too many items in the table with respect to the table
	 * size, the number of hash collisions increases and performance
	 * decreases. Enlarge the table size to prevent this happening */

	if ((hash_table->entries * 3) / hash_table->table_size > 0) {

		/* Table is more than 1/3 full */

		if (!hash_table_enlarge(hash_table)) {

			/* Failed to enlarge the table */

			return 0;
		}
	}

	/* Generate the hash of the key and hence the index into the table */

    index = string_nocase_hash(key) % hash_table->table_size;

	/* Traverse the chain at this location and look for an existing
	 * entry with the same key */

	rover = hash_table->table[index];

	while (rover != NULL) {

		/* Fetch rover's HashTablePair entry */

		pair = &(rover->pair);

        if (strcasecmp(pair->key, key) == 0) {

			/* Same with the key: use the new key value and free
			 * the old one */

            free(pair->key);

            pair->key = newkey;
			pair->value = value;

			/* Finished */

			return 1;
		}

		rover = rover->next;
	}

	/* Not in the hash table yet.  Create a new entry */

	newentry = (HashTableEntry *) malloc(sizeof(HashTableEntry));

	if (newentry == NULL) {
		return 0;
	}

    newentry->pair.key = newkey;
	newentry->pair.value = value;

	/* Link into the list */

	newentry->next = hash_table->table[index];
	hash_table->table[index] = newentry;

	/* Maintain the count of the number of entries */

	++hash_table->entries;

	/* Added successfully */

	return 1;
}

int hash_table_lookup(HashTable *hash_table, char *key)
{
	HashTableEntry *rover;
	HashTablePair *pair;
	unsigned int index;

	/* Generate the hash of the key and hence the index into the table */

    index = string_nocase_hash(key) % hash_table->table_size;

	/* Walk the chain at this index until the corresponding entry is
	 * found */

	rover = hash_table->table[index];

	while (rover != NULL) {
		pair = &(rover->pair);

        if (strcasecmp(key, pair->key) == 0) {

			/* Found the entry.  Return the data. */

			return pair->value;
		}

		rover = rover->next;
	}

	/* Not found */

	return HASH_TABLE_NULL;
}
