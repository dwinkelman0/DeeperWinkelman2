#ifndef _DWBST_DWBST_H_
#define _DWBST_DWBST_H_

template <typename K, typename V>
class BST {
protected:
	class Node {
		Node<K, V> * left = NULL
		Node<K, V> * right = NULL;
		K key;
		V value;
		
		Node(K key, V value);
		~Node();
	};
	
	Node * root = NULL;
	uint32_t n_elements = 0;
	
public:
	BST(K center_hint);
	~BST();

	bool Insert(const K key, const V value);
	bool Remove(const K key);
	bool Set(const K key, const V value);
	V Get(const K key) const;
	bool Contains(const K key) const;
	
	uint32_t Size() const;
	void Serialize(K * keys, V * values);
	
protected:
	static void Insert(Node * child, Node * parent);
};