#include"merkletree.h"
int main() {
	merkletree_leaf_sort_test();
	merkletree_build_test();
	merkletree_InclusionProof_test();
	merkletree_ExclusionProof_test();
	return 0;
}