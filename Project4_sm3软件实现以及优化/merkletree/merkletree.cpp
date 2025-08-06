#include"merkletree.h"



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
void merkletree_leaves_print(merkletree* tree) {
	for (int i = 0; i < tree->leaf_size; i++) {
		cout << "Leaf " << i << ": ";
		for (int j = 0; j < 64; j++) {
			cout << hex << setw(2) << setfill('0') << (int)tree->leaves[i].msg[j];
		}
		cout << endl;
	}
}
void merkletree_print(merkletree_node* root) {
	if (root != NULL) {
		node_print(root);
		merkletree_print(root->left);
		merkletree_print(root->right);
	}
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
void msg_get(unsigned char* output) {
	const char alphanumeric_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<size_t> distribution(0, sizeof(alphanumeric_chars) - 2);

	for (int i = 0; i < 64; i++) {
		output[i] = alphanumeric_chars[distribution(generator)];
	}
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
void merkletree_build(merkletree* tree, int leaf_size) {
	
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
	cout <<"merkletree build complete!\n" << endl;
	tree->root = head->next->node;
}
//存在性证明
vector<unsigned char*> InclusionProof_Set(unsigned char* msg, merkletree* root) {
	int index = 0;
	vector<unsigned char*> proof;
	for (int i = 0; i < root->leaf_size; i++) {
		if (memcmp(msg, root->leaves[i].msg, 64) < 0) {
			cout << "消息不存在" << endl;
			return proof;
		}
		//消息存在，提供存在性证明
		else if (memcmp(msg, root->leaves[i].msg, 64) == 0) {
			index = i;
			break;
		}
	}
	merkletree_node* current = root->leaves[index].node;
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
	if (proof.size() == 0) {
		return false; // 证明无效
	}
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
		cout << "the " << i << "th hash result:";
		for (int j = 0; j < 32; j++) {
			cout << hex << setw(2) << setfill('0') << (int)hash[j];
		}
		cout << endl;
	}
	cout << "root hash:";
	for (int j = 0; j < 32; j++) {
		cout << hex << setw(2) << setfill('0') << (int)root->root->hash[j];
	}
	cout << endl;
	if (memcmp(hash, root->root->hash, 32) == 0) {
		return true; // 验证成功
	}
	else return false; // 验证失败
}
//不存在性证明
vector<unsigned char*> ExclusionProof_Set(unsigned char* msg, merkletree* root) {
	int index = root->leaf_size-1;
	vector<unsigned char*> proof;
	for (int i = 0; i < root->leaf_size; i++) {
		if (memcmp(msg, root->leaves[i].msg, 64) < 0) {
			index = i;
			break;
		}
		//消息存在
		else if (memcmp(msg, root->leaves[i].msg, 64) == 0) {
			cout << "消息并非不存在" << endl;
			return proof;
		}
	}
	unsigned char* zero = new unsigned char[64];
	unsigned char* max = new unsigned char[64];
	memset(zero, 0x00, 64);
	memset(max, 0xFF, 64);
	if (index == 0) {
		proof = InclusionProof_Set(root->leaves[0].msg, root);
		unsigned char* hash = new unsigned char[64];
		sm3(root->leaves[0].msg, sizeof(root->leaves[0].msg), hash);
		proof.push_back(hash);
		proof.push_back(zero);
		proof.push_back(root->leaves[0].msg);
		return proof;
	}
	else if (index == root->leaf_size - 1) {
		proof = InclusionProof_Set(root->leaves[root->leaf_size - 1].msg, root);
		unsigned char* hash = new unsigned char[64];
		
		sm3(root->leaves[root->leaf_size - 1].msg, sizeof(root->leaves[root->leaf_size - 1].msg), hash);
		proof.push_back(hash);
		proof.push_back(root->leaves[root->leaf_size - 1].msg);
		proof.push_back(max);
		return proof;
	}
	else {
		proof = InclusionProof_Set(root->leaves[index].msg, root);
		unsigned char* hash = new unsigned char[64];
		sm3(root->leaves[index].msg, sizeof(root->leaves[index].msg), hash);
		proof.push_back(hash);
		proof.push_back(root->leaves[index - 1].msg);
		proof.push_back(root->leaves[index].msg);
		return proof;
	}
}
bool ExclusionProof_Verify(unsigned char* msg, vector<unsigned char*> proof, merkletree* root) {
	size_t n = proof.size();
	if (n < 3) {
		return false; // 证明无效
	}
	unsigned char hash[64];
	cout << "msg:";
	for (int j = 0; j < 64; j++) {
		cout << hex << setw(2) << setfill('0') << (int)msg[j];
	}
	cout << endl;

	cout << "leaf < msg:";
	for (int j = 0; j < 64; j++) {
		cout << hex << setw(2) << setfill('0') << (int)proof[n - 2][j];
	}
	cout << endl;

	cout << "leaf > msg:";
	for (int j = 0; j < 64; j++) {
		cout << hex << setw(2) << setfill('0') << (int)proof[n - 1][j];
	}
	cout << endl;

	if (memcmp(proof[n - 2], msg, 64) < 0 and memcmp(proof[n - 1], msg, 64) > 0 or
		memcmp(proof[n - 2], msg, 64) == 0 or memcmp(proof[n - 1], msg, 64) == 0) {
		memcpy(hash, proof[n - 3], 64);
		for (size_t i = 0; i < n - 3; i++) {
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
			cout << "the "<<i<<"th hash result:";
			for (int j = 0; j < 32; j++) {
				cout << hex << setw(2) << setfill('0') << (int)hash[j];
			}
			cout << endl;
		}
		cout << "root hash:";
		for (int j = 0; j < 32; j++) {
			cout << hex << setw(2) << setfill('0') << (int)root->root->hash[j];
		}
		cout << endl;
		if (memcmp(hash, root->root->hash, 32) == 0) {
			return true; // 验证成功
		}
		else return false; // 验证失败
	}
}
void merkletree_leaf_sort_test(){
	cout << "----------------merkletree_leaf_sort排序结果测试----------------\n";
	merkletree* r = new merkletree;
	r->leaves = new merkletree_leaf[10];
	r->leaf_size = 10;
	for (int i = 0; i < 10; i++) {
		msg_get(r->leaves[i].msg);
		r->leaves[i].node = new merkletree_node;
		sm3(r->leaves[i].msg, sizeof(r->leaves[i].msg) - 1, r->leaves[i].node->hash);
		merkletree_leaf_sort(r, &i);
	}
	//树过大时不建议打印构造结果
	/**/
	cout << "顺序结果：" << endl;
	for (int i = 0; i < 9; i++) {
		if (memcmp(r->leaves[i].msg, r->leaves[i + 1].msg, 64) < 0) {
			cout << "1";
		}
		else cout << "0";
	}
	cout << endl;
}
void merkletree_build_test() {
	cout << "----------------merkletree生成测试----------------\n";
	merkletree* root = new merkletree;
	merkletree_build(root, 10);
	cout << "Merkle Tree Structure:\n";
	merkletree_print(root->root);
	cout << "Leaves:\n";
	merkletree_leaves_print(root);
	cout << endl;
}
void merkletree_InclusionProof_test() {
	cout << "----------------存在性证明测试----------------\n";
	merkletree* root = new merkletree;
	merkletree_build(root, 3);
	vector<unsigned char*> proof = InclusionProof_Set(root->leaves[1].msg, root);
	cout << "Verfiy result:" << InclusionProof_Verify(root->leaves[1].msg, proof, root) << endl;

	cout << endl;
	proof = InclusionProof_Set(root->leaves[1].msg, root);
	cout << "Verfiy result:" << InclusionProof_Verify(root->leaves[1].msg, proof, root) << endl;
}
void merkletree_ExclusionProof_test(){
	cout << "----------------不存在性证明测试----------------\n";
	merkletree* root = new merkletree;
	merkletree_build(root, 10);
	unsigned char hash[64];
	unsigned char msg[64] = {0};
	msg_get(msg);
	vector<unsigned char*> proof = ExclusionProof_Set(msg, root);
	cout <<"Verfiy result:"<< ExclusionProof_Verify(msg, proof, root) << endl;
}