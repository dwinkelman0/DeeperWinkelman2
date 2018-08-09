#ifndef _ITERATIVE_BST_BST_H_
#define _ITERATIVE_BST_BST_H_

#include <stdio.h>

template <typename K, typename V>
class BST {
public:
	struct Node {
		friend class BST;
		
	protected:
		K key;
		V value;
		Node * right = NULL;
		Node * left = NULL;
	
	public:	
		inline K GetKey() {
			return this->key;
		}
		inline V GetValue() {
			return this->value;
		}
		inline void SetValue(V value) {
			this->value = value;
		}
	};
	
protected:
	Node * root = NULL;
	unsigned int n_nodes = 0;
	unsigned int max_depth = 0;
	
public:
	int (*compare_func)(K a, K b);
	
public:
	~BST();

	Node * Insert(K key, V value);
	Node * Find(K key);
	bool Remove(K key);
	
	Node * First();
	Node * Last();
	
	Node ** Serialize(unsigned int * n_nodes);
	
	BST * Rebalance();
	
protected:
	Node * Attach(Node * node);
};

#endif
