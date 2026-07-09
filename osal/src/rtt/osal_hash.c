#include <rtthread.h>
#include <stdlib.h>
#include "osal.h"
#include "osal_inner.h"

#define OSAL_HASH_SIZE 128
typedef struct hash_table hash_table;

typedef struct {
	long long unsigned int *key;
	void *value;
} hash_table_entry;

struct hash_table {
	hash_table_entry *entries;
	int capacity;
	int length;
};

osal_hash *osal_hash_create(unsigned int capacity)
{
	osal_hash *p = NULL;
	p = (osal_hash *)rt_malloc(sizeof(osal_hash));
	if (p == NULL) {
		osal_log("kmalloc error!\n");
		return NULL;
	}
	hash_table *table = (hash_table *)rt_malloc(sizeof(hash_table));
	if (table == NULL) {
		osal_log("kmalloc error!\n");
		return NULL;
	}

	table->length = 0;
	table->capacity = capacity ? capacity : OSAL_HASH_SIZE;
	table->entries = (hash_table_entry *)rt_calloc(
		capacity ? capacity : OSAL_HASH_SIZE, sizeof(hash_table_entry));

	if (table->entries == NULL) {
		rt_free(table);
		osal_log("init hash table entries failed\n");
		rt_free(p);
		return NULL;
	}

	p->hash = table;

	return p;
}
// EXPORT_SYMBOL(osal_hash_create);

void osal_hash_destroy(osal_hash *hash)
{
	if (hash == NULL || hash->hash == NULL) {
		osal_log("hash is null!\n");
		return;
	}
	hash_table *table = (hash_table *)hash->hash;

	for (int iterator = 0; iterator < (table->capacity); iterator++) {
		rt_free((void *)table->entries[iterator].key);
	}

	rt_free(table->entries);
	rt_free(table);
	hash->hash = NULL;
	rt_free(hash);
}
// EXPORT_SYMBOL(osal_hash_destroy);

#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

// FNV-1a 32-bit hash function
static int key_to_hash(long long unsigned int *key)
{
	long long unsigned int hash = FNV_OFFSET_BASIS;
	long long unsigned int value = *key;

	for (int i = 0; i < 4; i++) {
		hash ^= (value & 0xFF);
		hash *= FNV_PRIME;
		value >>= 8;
	}

	return (int)hash;
}

static int hash_table_set_entry(hash_table_entry *entries, int capacity,
				long long unsigned int *key, void *value,
				int *length)
{
	int hash = (int)key_to_hash(key);
	int index = (int)(hash & (int)(capacity - 1));

	while (entries[index].key != NULL) {
		if (*key == *(entries[index].key)) {
			entries[index].value = value;
			return OSAL_SUCCESS;
		}
		index++;
		if (index >= capacity) {
			index = 0;
		}
	}

	if (length != NULL) {
		(*length)++;
	}
	entries[index].key = key;
	entries[index].value = value;

	return OSAL_SUCCESS;
}

static int hash_table_expand(hash_table *table)
{
	int new_capacity = table->capacity * 2;

	if (new_capacity < table->capacity) {
		return OSAL_FAILURE;
	}

	hash_table_entry *new_entries = (hash_table_entry *)rt_calloc(
		new_capacity, sizeof(hash_table_entry));

	if (new_entries == NULL) {
		osal_log("alloc new entry failed, memory not enough!\n");
		return OSAL_FAILURE;
	}

	for (int i = 0; i < table->capacity; i++) {
		hash_table_entry *entry = &(table->entries[i]);

		if (entry->key != NULL) {
			hash_table_set_entry(new_entries, new_capacity,
					     entry->key, entry->value, NULL);
		}
	}

	rt_free(table->entries);
	table->entries = new_entries;
	table->capacity = new_capacity;
	return OSAL_SUCCESS;
}

int osal_hash_add(osal_hash *hash, long long unsigned int key, void *value)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	if (value == NULL) {
		osal_log("parameter value invalid!\n");
		return OSAL_FAILURE;
	}

	hash_table *table = (hash_table *)hash->hash;

	if (table->length > (table->capacity / 2)) {
		if (hash_table_expand(table) != OSAL_SUCCESS) {
			osal_log("parameter value invalid!\n");
		}
	}

	long long unsigned int *key_ptr = (long long unsigned int *)rt_malloc(
		sizeof(long long unsigned int));
	if (key_ptr == NULL) {
		osal_log("malloc key failed!\n");
		return OSAL_FAILURE;
	}
	*key_ptr = key;

	return hash_table_set_entry(table->entries, table->capacity, key_ptr,
				    value, &(table->length));
}
// EXPORT_SYMBOL(osal_hash_add);

void *osal_hash_get(osal_hash *hash, long long unsigned int key)
{
	int hash_index, index;

	if ((hash == NULL) || (hash->hash == NULL)) {
		osal_log("parameter invalid!\n");
		return NULL;
	}

	hash_table *table = (hash_table *)hash->hash;
	hash_index = (int)key_to_hash(&key);
	index = (int)(hash_index & (int)(table->capacity - 1));

	while (table->entries[index].key != NULL) {
		if (key == *(table->entries[index].key)) {
			return table->entries[index].value;
		}

		index++;
		if (index >= table->capacity) {
			index = 0;
		}
	}
	return NULL;
}
// EXPORT_SYMBOL(osal_hash_get);

static void *osal_hash_remove_or_del(osal_hash *hash, long long unsigned int key)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
		osal_log("parameter invalid!\n");
		return NULL;
	}

	hash_table *table = hash->hash;
	int hash_value = (int)key_to_hash(&key);
	int index = (int)(hash_value & (int)(table->capacity - 1));
	int start_index = index;

	while (table->entries[index].key != NULL) {
		if (key == *(table->entries[index].key)) {
			void *value = table->entries[index].value;
			rt_free((void *)table->entries[index].key);
			table->entries[index].key = NULL;
			table->entries[index].value = NULL;
			table->length--;

			int next_index = (index + 1) % table->capacity;
			while (table->entries[next_index].key != NULL) {
				int rehash_index =
					(int)(key_to_hash(
						      table->entries[next_index]
							      .key) &
					      (int)(table->capacity - 1));
				if ((rehash_index <= index &&
				     index < next_index) ||
				    (next_index < rehash_index &&
				     rehash_index <= index) ||
				    (index < next_index &&
				     next_index < rehash_index)) {
					table->entries[index] =
						table->entries[next_index];
					table->entries[next_index].key = NULL;
					table->entries[next_index].value = NULL;
					index = next_index;
				}
				// osal_log("index = %d, ")
				next_index = (next_index + 1) % table->capacity;
			}

			return value;
		}
		index++;
		if (index >= table->capacity) {
			index = 0;
		}
		if (index == start_index) {
			break;
		}
	}
	return NULL;
}

void *osal_hash_remove(osal_hash *hash, long long unsigned int key)
{
	return osal_hash_remove_or_del(hash, key);
}
// EXPORT_SYMBOL(osal_hash_remove);

int osal_hash_del(osal_hash *hash, long long unsigned int key)
{
	return osal_hash_remove_or_del(hash, key) != NULL ? OSAL_SUCCESS :
							    OSAL_FAILURE;
}
// EXPORT_SYMBOL(osal_hash_del);

int osal_hash_size(osal_hash *hash)
{
	hash_table *table = (hash_table *)hash->hash;
	return table->length;
}
// EXPORT_SYMBOL(osal_hash_size);

void osal_hash_for_each(osal_hash *hash, hash_callback cb, void *context)
{
	if ((hash == NULL) || (hash->hash == NULL) || (cb == NULL)) {
		osal_log("parameter invalid!\n");
		return;
	}

	hash_table *table = (hash_table *)hash->hash;
	for (int i = 0; i < table->capacity; i++) {
		if (table->entries[i].key != NULL) {
			cb(*(table->entries[i].key), table->entries[i].value,
			   context);
		}
	}
}
// EXPORT_SYMBOL(osal_hash_for_each);

void osal_hash_for_each_safe(osal_hash *hash, hash_callback cb, void *context)
{
	if ((hash == NULL) || (hash->hash == NULL) || (cb == NULL)) {
		osal_log("parameter invalid!\n");
		return;
	}

	hash_table *table = (hash_table *)hash->hash;

	size_t capacity = table->capacity;
	void **keys = (void **)malloc(capacity * sizeof(void *));
	if (keys == NULL) {
		osal_log("memory allocation failed!\n");
		return;
	}

	// get all the keys needed operation
	int count = 0;
	for (int i = 0; i < capacity; i++) {
		if (table->entries[i].key != NULL) {
			keys[count] = table->entries[i].key;
			count++;
		}
	}

	for (int i = 0; i < count; i++) {
        cb(*(unsigned long long int*)(keys[i]), osal_hash_get(hash, *(unsigned long long int*)keys[i]), context);
	}

	free(keys);
}
// EXPORT_SYMBOL(osal_hash_for_each_safe);
