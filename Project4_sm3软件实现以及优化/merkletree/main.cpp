#include"merkletree.h"
#include"LengthExtension.h"
#include"sm3.h"
int main() {
	//sm3_test();
	//merkletree_leaf_sort_test();
	//merkletree_build_test();
	//merkletree_InclusionProof_test();
	//merkletree_ExclusionProof_test();
	//MAC_test();
	//MAC_forge_test();
	merkletree_100000_test();
	return 0;
}