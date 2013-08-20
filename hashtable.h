
#ifndef MMTRAINING_HASHTABLE_H
#define MMTRAINING_HASHTABLE_H

#include <stdio.h>

namespace mmtraining {

template <class K, class V, size_t BucketSize> 
class KVHashTable {
public:
    enum {
	BUCKETSIZE = BucketSize,
    };

    struct KVHashTableNode {
	K key;
	V value;
	KVHashTableNode *next;
	KVHashTableNode() : next(NULL) {}
	KVHashTableNode(K k, V v, KVHashTableNode *n = NULL) {
	    key = k;
	    value = v;
	    next = n;
	}
    };

    KVHashTable () {
	heads = new KVHashTableNode[BUCKETSIZE];
    }

    ~KVHashTable () {
	if (heads != NULL) {
	    delete []heads;
	}
    }

    /**
     * Success: return true, value is the target value
     * False: return false, value unchanged
     */
    bool Get(K key, V &value) {
	unsigned int index = key.HashValue() % BUCKETSIZE;
	KVHashTableNode *now = heads[index].next;
	while (now != NULL) {
	    if (now->key == key) {
		value = now->value;
		return true;
	    }
	    now = now->next;
	}
	return false;
    }

    /**
     * key exist: overwrite the value, return 2
     * key not exist: insert into the hash bucket, return 1
     */
    int Set(K key, V value) {
	unsigned int index = key.HashValue() % BUCKETSIZE;
	KVHashTableNode *head = &heads[index];
	KVHashTableNode *node = head->next;
	while (node != NULL) {
	    if (node->key == key) {
		node->value = value;
		return 2;
	    }
	    node = node->next;
	}

	KVHashTableNode* tmp = new KVHashTableNode(key, value, head->next);
	head->next = tmp;
	return 1;
    }

    /**
     * key exist: delete the node, return true
     * key not exist: return false
     */
    bool Delete(K key) {
	unsigned int index = key.HashValue() % BUCKETSIZE;
	KVHashTableNode *node = &heads[index];
	while (node != NULL) {
	    KVHashTableNode *next = node->next;
	    if (next != NULL && next->key == key) {
		node->next = next->next;
		delete next;
		return true;
	    }
	    node = next;
	}
	return false;
    }

protected:
    KVHashTableNode *heads; //heads are dummy nodes
};


}
#endif
