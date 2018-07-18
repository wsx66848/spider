#ifndef HASH_MAP_H
#define HASH_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A hash table structure.
 */

typedef struct _HashTable HashTable;

/**
 * Internal structure representing an entry in a hash table.
 */

typedef struct _HashTableEntry HashTableEntry;


/**
 * Internal structure representing an entry in hash table
 * used as @ref HashTableIterator next result.
 */

typedef struct _HashTablePair{
    char* key;
    int value;
} HashTablePair;

/**
 * Definition of a @ref HashTableIterator.
 */

struct _HashTableIterator {
	HashTable *hash_table;
	HashTableEntry *next_entry;
	unsigned int next_chain;
};

/**
 * A null @ref HashTableValue.
 */

#define HASH_TABLE_NULL 0

/**
 * Create a new hash table.
 *
 * @return                     A new hash table structure, or NULL if it
 *                             was not possible to allocate the new hash
 *                             table.
 */

HashTable *hash_table_new();

/**
 * Destroy a hash table.
 *
 * @param hash_table           The hash table to destroy.
 */

void hash_table_free(HashTable *hash_table);


/**
 * Insert a value into a hash table, overwriting any existing entry
 * using the same key.
 *
 * @param hash_table           The hash table.
 * @param key                  The key for the new value.
 * @param value                The value to insert.
 * @return                     Non-zero if the value was added successfully,
 *                             or zero if it was not possible to allocate
 *                             memory for the new entry.
 */

int hash_table_insert(HashTable *hash_table,
                      char* key,
                      int value);

/**
 * Look up a value in a hash table by key.
 *
 * @param hash_table          The hash table.
 * @param key                 The key of the value to look up.
 * @return                    The value, or @ref HASH_TABLE_NULL if there
 *                            is no value with that key in the hash table.
 */

int hash_table_lookup(HashTable *hash_table,
                                 char *key);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef HASH_MAP_H */

