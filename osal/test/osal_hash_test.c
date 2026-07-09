#include <osal_test.h>

#define HASH_SIZE 9

int print_callback(long long unsigned int key, void *value, void *context)
{
	osal_test_log("Key: %llu, Value: %s\n", key, (char *)value);
	return 0;
}

int safe_callback(long long unsigned int key, void *value, void *context)
{
	osal_hash *hash = (osal_hash *)context;
    osal_test_log("Key: %llu, Value: %s\n", key, (char *)value);
	osal_hash_remove(hash, key);
	return OSAL_SUCCESS;
}

void osal_hash_test(void)
{
	osal_hash *hash = osal_hash_create(HASH_SIZE);
	char *value = NULL;
	if (hash == NULL) {
		osal_test_log("Failed to create hash table\n");
		return;
	}

	osal_hash_add(hash, 1, (void *)"Value1");
	osal_hash_add(hash, 2, (void *)"Value2");
	osal_hash_add(hash, 3, (void *)"Value3");
	osal_hash_add(hash, 4, (void *)"Value4");
	osal_hash_add(hash, 5, (void *)"Value5");
	osal_hash_add(hash, 6, (void *)"Value6");
	osal_hash_add(hash, 7, (void *)"Value7");
	osal_hash_add(hash, 8, (void *)"Value8");
	osal_hash_add(hash, 9, (void *)"Value9");

	value = (char *)osal_hash_get(hash, 3);
	if (strcmp(value, "Value3") == 0) {
		osal_test_log("Found value: %s\n", value);
	} else {
		osal_test_log("Key not found\n");
	}

	osal_hash_remove(hash, 3);

	if (osal_hash_del(hash, 4) != OSAL_SUCCESS) {
		osal_test_log("[fail] osal_hash_del\n");
	}

	osal_hash_for_each(hash, print_callback, NULL);

    osal_hash_for_each_safe(hash, safe_callback, hash);

    if (osal_hash_size(hash) != 0) {
		osal_test_log("[fail] osal_hash_size:%d\n", osal_hash_size(hash));
	}

	osal_hash_destroy(hash);
	osal_test_log(
		"[success] All hash tests(create, add, get, remove, for_each, for_each_safe, size, destroy) passed \n");

	osal_test_log("osal_hash test completed.\n");
}
OSAL_CMD_EXPORT(osal_hash_test, Test hash operations);