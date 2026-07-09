#include <aos/kernel.h>
#include <aos/debug.h>
#include "osal.h"
#include "osal_errno.h"
#include "osal_inner.h"

typedef struct Hashmap Hashmap;

typedef struct Entry Entry;

struct Entry {
	void *key;
	int hash;
	void *value;
	Entry *next;
};

struct Hashmap {
	Entry **buckets;
	size_t bucketCount;
	int (*hash)(void *key);
	bool (*equals)(void *keyA, void *keyB);
	// pthread_mutex_t lock;
	size_t size;
};

static int hash_key(void *key)
{
	return (((uintptr_t)key) >> 10);
}

static bool hash_equals(void *keyA, void *keyB)
{
	return (keyA == keyB);
}

static inline int hashKey(Hashmap *map, void *key)
{
	int h = map->hash(key);

	// We apply this secondary hashing discovered by Doug Lea to defend
	// against bad hashes.
	h += ~(h << 9);
	h ^= (((unsigned int)h) >> 14);
	h += (h << 4);
	h ^= (((unsigned int)h) >> 10);

	return h;
}

static Entry *createEntry(void *key, int hash, void *value)
{
	Entry *entry = malloc(sizeof(Entry));

	if (entry == NULL) {
		return NULL;
	}
	entry->key = key;
	entry->hash = hash;
	entry->value = value;
	entry->next = NULL;
	return entry;
}

static inline bool equalKeys(void *keyA, int hashA, void *keyB, int hashB, bool (*equals)(void *, void *))
{
	if (keyA == keyB)
		return true;
	if (hashA != hashB)
		return false;
	return equals(keyA, keyB);
}

static inline size_t calculateIndex(size_t bucketCount, int hash)
{
	return ((size_t)hash) & (bucketCount - 1);
}

static void expandIfNecessary(Hashmap *map)
{
	// If the load factor exceeds 0.75...
	if (map->size > (map->bucketCount * 3 / 4)) {
		// Start off with a 0.33 load factor.
		size_t newBucketCount = map->bucketCount << 1;
		Entry **newBuckets = (Entry **)calloc(newBucketCount, sizeof(Entry *));

		if (newBuckets == NULL) {
			// Abort expansion.
			return;
		}

		// Move over existing entries.
		size_t i;

		for (i = 0; i < map->bucketCount; i++) {
			Entry *entry = map->buckets[i];

			while (entry != NULL) {
				Entry *next = entry->next;
				size_t index = calculateIndex(newBucketCount, entry->hash);

				entry->next = newBuckets[index];
				newBuckets[index] = entry;
				entry = next;
			}
		}
		// Copy over internals.
		free(map->buckets);
		map->buckets = newBuckets;
		map->bucketCount = newBucketCount;
	}
}

osal_hash *osal_hash_create(unsigned int capacity)
{
	osal_hash *p = NULL;

	p = (osal_hash *)malloc(sizeof(osal_hash));
	if (p == NULL) {
		osal_log("kmalloc error!\n");
		return NULL;
	}

	Hashmap *map = malloc(sizeof(Hashmap));
	if (map == NULL) {
		osal_log("kmalloc error!\n");
		free(p);
		return NULL;
	}

	// 0.75 load factor.
	size_t minimumBucketCount = capacity * 4 / 3;

	if (map == NULL) {
		return NULL;
	}

	map->bucketCount = 1;
	while (map->bucketCount <= minimumBucketCount) {
		// Bucket count must be power of 2.
		map->bucketCount <<= 1;
	}
	map->buckets = (Entry **)calloc(map->bucketCount, sizeof(Entry *));
	if (map->buckets == NULL) {
		free(map);
		return NULL;
	}

	map->size = 0;
	map->hash = hash_key;
	map->equals = hash_equals;

	p->hash = map;

	return p;
}

void osal_hash_destroy(osal_hash *hash)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	size_t i;

	for (i = 0; i < map->bucketCount; i++) {
		Entry *entry = map->buckets[i];

		while (entry != NULL) {
			Entry *next = entry->next;

			free(entry);
			entry = next;
		}
	}
	free(map->buckets);
	free(map);
}

int osal_hash_add(osal_hash *hash, long long unsigned int key, void *value)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	int h = hashKey(map, (void *)key);
	size_t index = calculateIndex(map->bucketCount, h);
	Entry **p = &(map->buckets[index]);

	while (true) {
		Entry *current = *p;

		// Add a new entry.
		if (current == NULL) {
			*p = createEntry((void *)key, h, value);
			if (*p == NULL) {
				errno = ENOMEM;
				return OSAL_FAILURE;
			}
			map->size++;
			expandIfNecessary(map);
			return OSAL_SUCCESS;
		}
		// Replace existing entry.
		if (equalKeys(current->key, current->hash, (void *)key, h, map->equals)) {
			current->value = value;
			return OSAL_SUCCESS;
		}
		// Move to next entry.
		p = &current->next;
	}

	return OSAL_SUCCESS;
}

void *osal_hash_get(osal_hash *hash, long long unsigned int key)
{
	void *ret = NULL;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return ret;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	int h = hashKey(map, (void *)key);
	size_t index = calculateIndex(map->bucketCount, h);
	Entry *entry = map->buckets[index];

	while (entry != NULL) {
		if (equalKeys(entry->key, entry->hash, (void *)key, h, map->equals)) {
			return entry->value;
		}
		entry = entry->next;
	}

	return ret;
}

void *osal_hash_remove(osal_hash *hash, long long unsigned int key)
{
	void *ret = NULL;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return ret;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	int h = hashKey(map, (void *)key);
	size_t index = calculateIndex(map->bucketCount, h);
	// Pointer to the current entry.
	Entry **p = &(map->buckets[index]);
	Entry *current;

	while ((current = *p) != NULL) {
		if (equalKeys(current->key, current->hash, (void *)key, h, map->equals)) {
			void *value = current->value;
			*p = current->next;
			free(current);
			map->size--;
			return value;
		}
		p = &current->next;
	}

	return ret;
}

int osal_hash_del(osal_hash *hash, long long unsigned int key)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	int h = hashKey(map, (void *)key);
	size_t index = calculateIndex(map->bucketCount, h);
	// Pointer to the current entry.
	Entry **p = &(map->buckets[index]);
	Entry *current;

	while ((current = *p) != NULL) {
		if (equalKeys(current->key, current->hash, (void *)key, h, map->equals)) {
			*p = current->next;
			free(current);
			map->size--;
			return OSAL_SUCCESS;
		}
		p = &current->next;
	}

	return OSAL_SUCCESS;
}

int osal_hash_size(osal_hash *hash)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	return (int)map->size;
}

void osal_hash_for_each(osal_hash *hash, hash_callback cb, void *context)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	size_t i;

	for (i = 0; i < map->bucketCount; i++) {
		Entry *entry = map->buckets[i];

		while (entry != NULL) {
			Entry *next = entry->next;

			if (cb((long long unsigned int )entry->key, entry->value, context) == OSAL_FAILURE) {
				return;
			}
			entry = next;
		}
	}
}

void osal_hash_for_each_safe(osal_hash *hash, hash_callback cb, void *context)
{
	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid!\n");
        return;
	}
	Hashmap *map = (Hashmap *)hash->hash;

	size_t i;

	for (i = 0; i < map->bucketCount; i++) {
		Entry *entry = map->buckets[i];

		while (entry != NULL) {
			Entry *next = entry->next;

			if (cb((long long unsigned int )entry->key, entry->value, context) == OSAL_FAILURE) {
				return;
			}
			entry = next;
		}
	}
}


