#include"merkletree.h"


void msg_get(unsigned char* output) {
	const char alphanumeric_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<size_t> distribution(0, sizeof(alphanumeric_chars) - 2);

	for (int i = 0; i < 64; i++) {
		output[i] = alphanumeric_chars[distribution(generator)];
	}
}
void node_print(merkletree_node* node) {
	if (node == nullptr) {
		return;
	}
	cout << "Node Level: " << node->level << ", Hash: ";
	for (int i = 0; i < 32; i++) {
		cout << hex << setw(2) << setfill('0') << (int)node->hash[i];
	}
	cout << endl;
}
void merkletree_floor_print(merkletree_floor* floor) {
	merkletree_floor* next = floor->next;
	int size = 0;
	while (next != NULL) {
		if (next->node != nullptr) {
			node_print(next->node);
		}
		next = next->next;
		size++;
	}
	cout << "Floor size: " << size << endl;
}

void merkletree_merge(merkletree_node* left, merkletree_node* right, merkletree_node* parent) {

	// Concatenate the hashes of the left and right nodes
	unsigned char combined_hash[64];
	memcpy(combined_hash, left->hash, 32);
	memcpy(combined_hash + 32, right->hash, 32);
	// Process the combined hash
	sm3(combined_hash, sizeof(combined_hash) - 1, parent->hash);
	left->parent = parent;
	right->parent = parent;
	parent->left = left;
	parent->right = right;
	parent->level = max(left->level, right->level) + 1;
}
void merkletree_leaf_sort(merkletree* tree,int* end) {
	int i = *end;
	while (i > 0) {
		int b = memcmp(tree->leaves[i].msg, tree->leaves[i - 1].msg, 64);
		if (b < 0) {
			// Swap the leaves
			merkletree_leaf temp = tree->leaves[i];
			tree->leaves[i] = tree->leaves[i - 1];
			tree->leaves[i - 1] = temp;
			i--;
		}
		else if (b == 0) {
			delete tree->leaves[i].node;
			for (int j = i; j < *end - 1; j++) {
				tree->leaves[j] = tree->leaves[j + 1];
			}
			(*end)--;
			break;
		}
		else{
			break;
		}
	}
}
void merkletree_print(merkletree_node* root) {
	if (root != NULL) {
		node_print(root);
		merkletree_print(root->left);
		merkletree_print(root->right);
	}
}

void merkletree_set(merkletree* tree, int leaf_size) {
	
	// Set the leaf nodes
	merkletree_floor* head = new merkletree_floor;
	merkletree_floor* next = head;
	tree->leaves = new merkletree_leaf[leaf_size];
	tree->leaf_size = leaf_size;
	int nodes_index = 0;
	
	for (int i = 0; i < leaf_size; i++) {
		tree->leaves[i].node = new merkletree_node;
		msg_get(tree->leaves[i].msg);
		merkletree_floor* cur = new merkletree_floor;
		sm3(tree->leaves[i].msg, sizeof(tree->leaves[i].msg), tree->leaves[i].node->hash);
		cur->node = tree->leaves[i].node;
		cur->node->left = nullptr;
		cur->node->right = nullptr;
		next->next = cur;
		next = cur;
		merkletree_leaf_sort(tree, &i);
	}
	cout << "floor_setting:" << head->next->node->level<<"complete!" << endl;
	// Build the tree
	int t = 1;
	while (t < leaf_size) {
		t *= 2;
	}
	int l = t/2;
	t = leaf_size - t / 2;
	next = head->next;
	for (int i = 0; i < t; i++) {
		merkletree_node* merge = new merkletree_node;
		merkletree_merge(next->node, next->next->node, merge);
		next->node = merge;
		next->next = next->next->next;
		next = next->next;
	}
	if (t != 0) {
		for (int i = 0; i < l - t; i++) {
			next->node->level = (next->node->level) + 1;
			next = next->next;
		}
	}
	cout << "floor_setting:" << head->next->node->level << "complete!" << endl;
	while (l > 1) {
		next = head->next;
		for (int i = 0; i < l / 2; i++) {
			merkletree_node* merge = new merkletree_node;
			merkletree_merge(next->node, next->next->node, merge);
			next->node = merge;
			next->next = next->next->next;
			next = next->next;
		}
		l = l / 2;
		cout << "floor_setting:" << head->next->node->level << "complete!" << endl;
	}
	tree->root = head->next->node;
}
//存在性证明
vector<unsigned char*> InclusionProof_Set(unsigned char* msg, merkletree* root) {
	int index = 0;
	for (int i = 0; i < root->leaf_size; i++) {
		//消息不存在，提供不存在证明
		if (memcmp(msg, root->leaves[i].msg, 64) < 0) {
			
		}
		//消息存在，提供存在性证明
		else if (memcmp(msg, root->leaves[i].msg, 64) == 0) {
			index = i;
		}
	}
	merkletree_node* current = root->leaves[index].node;
	vector<unsigned char*> proof;
	while (current->parent != NULL) {
		
		unsigned char* sibling = new unsigned char[65];
		if (current->parent->left == current) {
			memcpy(sibling, current->parent->right->hash, 64);
			sibling[64] = '0';
		}
		else {
			memcpy(sibling, current->parent->left->hash, 64);
			sibling[64] = '1';
		}
		proof.push_back(sibling);
		current = current->parent;
	}
	return proof;
}
bool InclusionProof_Verify(unsigned char* msg, vector<unsigned char*> proof, merkletree* root) {
	unsigned char hash[64];
	unsigned char msg_[64];
	memcpy(msg_, msg, 64);
	sm3(msg_,sizeof(msg_), hash);
	for (size_t i = 0; i < proof.size();i++) {
		unsigned char combined_hash[64];
		if (proof[i][64] == '0') {
			memcpy(combined_hash, hash, 32);
			memcpy(combined_hash + 32, proof[i], 32);
			sm3(combined_hash, sizeof(combined_hash) - 1, hash);
		}
		else {
			memcpy(combined_hash, proof[i], 32);
			memcpy(combined_hash + 32, hash, 32);
			sm3(combined_hash, sizeof(combined_hash) - 1, hash);
		}
	}
	if (memcmp(hash, root->root->hash, 32) == 0) {
		return true; // 验证成功
	}
	else return false; // 验证失败
}

void merkletree_test() {
	merkletree* root = new merkletree;
	//merkletree生成测试
	//merkletree_set(root, 10);

	//merkletree排序结果测试
	/*
	merkletree* r = new merkletree;
	r->leaves = new merkletree_leaf[10];
	r->leaf_size = 10;
	for (int i = 0; i < 10; i++) {
		msg_get(r->leaves[i].msg);
		r->leaves[i].node = new merkletree_node;
		sm3(r->leaves[i].msg, sizeof(r->leaves[i].msg) - 1, r->leaves[i].node->hash);
		merkletree_leaf_sort(r, &i);
	}
	for (int i = 0; i < 9; i++) {
		if (memcmp(r->leaves[i].msg, r->leaves[i + 1].msg, 65) < 0) {
			cout << "1";
		}
		else cout << "0";
	}
	cout << endl;
	*/

	//存在性证明测试
	merkletree_set(root, 3);
	unsigned char hash[64];
	vector<unsigned char*> proof = InclusionProof_Set(root->leaves[1].msg, root);
	cout << InclusionProof_Verify(root->leaves[1].msg, proof, root) << endl;
	sm3(root->leaves[1].msg, sizeof(root->leaves[1].msg), hash);

}