#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/slab.h>

#include "osal.h"
#include "osal_inner.h"

#define OSAL_HASH_SIZE 128

struct osal_hash_node {
	long long unsigned int key;
	void *value_p;
	int value_i;
	struct hlist_node node;
};

struct osal_hash_head {
	struct hlist_head head[OSAL_HASH_SIZE];
};

osal_hash *osal_hash_create(unsigned int capacity)
{
	osal_hash *p = NULL;
	struct osal_hash_head *head;

	p = (osal_hash *)kmalloc(sizeof(osal_hash), GFP_ATOMIC);
	if (p == NULL) {
		osal_log("kmalloc error!\n");
		return NULL;
	}

	head = (struct osal_hash_head *)kmalloc(sizeof(*head), GFP_ATOMIC);
	if (head == NULL) {
		osal_log("kmalloc error!\n");
		kfree(p);
		return NULL;
	}
	hash_init(head->head);
	p->hash = head;

	return p;
}
EXPORT_SYMBOL(osal_hash_create);


void osal_hash_destroy(osal_hash *hash)
{
	struct osal_hash_node *obj = NULL;
	struct osal_hash_head *head;
	struct hlist_node *tmp;
	int bkt;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each_safe(head->head, bkt, tmp, obj, node) {
		if (obj) {
			hash_del(&obj->node);
			kfree(obj);
			obj = NULL;
		}
	}

	kfree(head);
	hash->hash = NULL;
	kfree(hash);
}
EXPORT_SYMBOL(osal_hash_destroy);

int osal_hash_add(osal_hash *hash, long long unsigned int key, void *value)
{
	struct osal_hash_head *head;
	struct osal_hash_node *hash_node;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
	}

	hash_node = (struct osal_hash_node *)kmalloc(sizeof(struct osal_hash_node), GFP_ATOMIC);
	if (hash_node == NULL) {
		osal_log("kmalloc error!\n");
		return OSAL_FAILURE;
	}
	head = (struct osal_hash_head *)hash->hash;
	hash_node->value_p = value;
	hash_node->key = key;
	hash_add(head->head, &hash_node->node, key);


    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_hash_add);

void *osal_hash_get(osal_hash *hash, long long unsigned int key)
{
	struct osal_hash_node *obj;
	struct osal_hash_head *head;
	void *ret = NULL;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return ret;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each_possible(head->head, obj, node, key) {
		if (obj->key == key) {
			ret = obj->value_p;
			break;
		}
	}

	return ret;
}
EXPORT_SYMBOL(osal_hash_get);

void *osal_hash_remove(osal_hash *hash, long long unsigned int key)
{
	struct osal_hash_node *obj;
	struct osal_hash_head *head;
	void *ret = NULL;
	bool is_found = false;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return ret;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each_possible(head->head, obj, node, key) {
		if (obj->key == key) {
			is_found = true;
			break;
		}
	}

	if (is_found) {
		hash_del(&obj->node);
		ret = obj->value_p;
		kfree(obj);
	}

	return ret;
}
EXPORT_SYMBOL(osal_hash_remove);

int osal_hash_del(osal_hash *hash, long long unsigned int key)
{
	struct osal_hash_node *obj;
	struct osal_hash_head *head;
	bool is_found = false;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each_possible(head->head, obj, node, key) {
		if (obj->key == key) {
			is_found = true;
			break;
		}
	}

	if (is_found) {
		hash_del(&obj->node);
		kfree(obj);
	}

	return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_hash_del);

int osal_hash_size(osal_hash *hash)
{
	struct osal_hash_node *obj;
	struct osal_hash_head *head;
	int cnt = 0;
	int bkt;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return cnt;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each(head->head, bkt, obj, node) {
		cnt++;
	}

	return cnt;
}
EXPORT_SYMBOL(osal_hash_size);

void osal_hash_for_each(osal_hash *hash, hash_callback cb, void *context)
{
	struct osal_hash_node *obj;
	struct osal_hash_head *head;
	int bkt;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each(head->head, bkt, obj, node) {
		if (cb(obj->key, obj->value_p, context) == OSAL_FAILURE)
			break;
	}
}
EXPORT_SYMBOL(osal_hash_for_each);

void osal_hash_for_each_safe(osal_hash *hash, hash_callback cb, void *context)
{
	struct osal_hash_node *obj;
	struct osal_hash_head *head;
	struct hlist_node *tmp;
	int bkt;

	if ((hash == NULL) || (hash->hash == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
	}
	head = (struct osal_hash_head *)hash->hash;

	hash_for_each_safe(head->head, bkt, tmp, obj, node) {
		if (cb(obj->key, obj->value_p, context) == OSAL_FAILURE)
			break;
	}
}
EXPORT_SYMBOL(osal_hash_for_each_safe);

