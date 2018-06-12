#include "dwbst.h"

template <typename K, typename V>
BST::Node::Node(K key, V value) {
	this->key = key;
	this->value = value;
}

template <typename K, typename V>
BST::Node::~Node() {
	if (this->left) {
		delete this->left;
		this->left = NULL;
	}
	if (this->right) {
		delete this->right;
		this->right = NULL;
	}
}

template <typename K, typename V>
BST::BST(K center_hint) {
	this->root = new Node(center_hint, 0);
}

template <typename K, typename V>
BST::~BST() {
	delete this->root;
}

template <typename K, typename V>
bool BST::Insert()