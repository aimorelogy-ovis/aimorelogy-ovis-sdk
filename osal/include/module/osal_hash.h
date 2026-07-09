#ifndef __OSAL_HASH_H__
#define __OSAL_HASH_H__


typedef struct {
    void *hash;
} osal_hash;

typedef int (*hash_callback)(long long unsigned int key, void *value, void *context);


/**
 * @ingroup osal_hash
 *
 * @brief Provides an interface for creating a hash table.
 *
 * @par Description:
 * Create the hash table and specify the size.
 *
 * @param capacity [in] Size of the hash table. Invalid for linux kernel.
 *
 * @retval   osal_hash*  If the hash is successfully created, the pointer of the hash is returned.
 * @retval   NULL        If the hash fails to be created, NULL is returned.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
osal_hash *osal_hash_create(unsigned int capacity);

/**
 * @ingroup osal_hash
 *
 * @brief Used to destroy the hash table you created.
 *
 * @par Description:
 * Destroy a specified hash.
 *
 * @param osal_hash[in] hash table.
 *
 * @par Support System:
 * linux alios rt_thread
 */
void osal_hash_destroy(osal_hash *hash);

/**
 * @ingroup osal_hash
 *
 * @brief Add key-value to hash.
 *
 * @param osal_hash[in] hash table.
 * @param key[in] key.
 * @param value[in] value.
 *
 * @return OSAL_FAILURE/OSAL_SUCCESS.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_hash_add(osal_hash *hash, long long unsigned int key, void *value);

/**
 * @ingroup osal_hash
 *
 * @brief Get the value from the hash with the key.
 *
 * @param osal_hash[in] hash table.
 * @param key[in] key.
 *
 * @return value or NULL.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void *osal_hash_get(osal_hash *hash, long long unsigned int key);

/**
 * @ingroup osal_hash
 *
 * @brief Get the value from the hash and removes it.
 *
 * @param osal_hash[in] hash table.
 * @param key[in] key.
 *
 * @return value or NULL.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void *osal_hash_remove(osal_hash *hash, long long unsigned int key);

/**
 * @ingroup osal_hash
 *
 * @brief Deletes a key-value from the hash.
 *
 * @param osal_hash[in] hash table.
 * @param key[in] key.
 *
 * @return OSAL_FAILURE/OSAL_SUCCESS.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_hash_del(osal_hash *hash, long long unsigned int key);

/**
 * @ingroup osal_hash
 *
 * @brief Get the hash size.
 *
 * @param osal_hash[in] hash table.
 *
 * @return hash size.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_hash_size(osal_hash *hash);

/**
 * @ingroup osal_hash
 *
 * @brief Iterate over the entire hash.
 * Invokes the given callback on each entry in the map. Stops iterating if
 * the callback returns OSAL_FAILURE.
 *
 * @param osal_hash[in] hash table.
 * @param cb[in] callback.
 * @param context[in] Private data.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_hash_for_each(osal_hash *hash, hash_callback cb, void *context);

/**
 * @ingroup osal_hash
 *
 * &brief Safely traverses the entire hash
 * Invokes the given callback on each entry in the map. Stops iterating if
 * the callback returns OSAL_FAILURE.
 *
 * @param osal_hash[in] hash table.
 * @param cb[in] callback.
 * @param context[in] Private data.
 *
 * @par Support System:
 * linux alios rt_thread.
 *
 * @attention
 * When performing deletion operations during iteration, please use osal_hash_for_each_safe.
 */
void osal_hash_for_each_safe(osal_hash *hash, hash_callback cb, void *context);

#endif
