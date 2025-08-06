#pragma once
#include"sm3.h"
#include <random>
#include<string>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace std;

//树节点，包含哈希值，节点层次，左右子节点及父节点指针
struct merkletree_node {
	unsigned char hash[64];
	int level = 0;
	merkletree_node* left ;
	merkletree_node* right ;
	merkletree_node* parent =NULL;
};
//树叶子,包含消息和指向对应节点的指针
struct merkletree_leaf {
	merkletree_node* node ;
	unsigned char msg[64];
};
//树的层次，用于树的build
struct merkletree_floor {
	merkletree_node* node ;
	merkletree_floor* next ;
};
//树的整体结构，包含根节点，叶子节点数组和叶子节点数量
struct merkletree {
	merkletree_node* root ;
	merkletree_leaf* leaves;
	int leaf_size;
};

//随机消息生成函数
void msg_get(unsigned char* output);

//单个节点，树的层次，叶子节点，整棵树的打印函数
void node_print(merkletree_node* node);
void merkletree_floor_print(merkletree_floor* floor);
void merkletree_leaves_print(merkletree* tree);
void merkletree_print(merkletree_node* tree);

//节点合并函数，叶子节点排序函数（用于不存在性证明），Merkle树生成函数
void merkletree_merge(merkletree_node* left, merkletree_node* right, merkletree_node* parent); 
void merkletree_leaf_sort(merkletree* tree,int* end);
void merkletree_build(merkletree* tree,int leaf_size);

//存在性证明，不存在性证明及其验证函数
vector<unsigned char*> InclusionProof_Set(unsigned char* msg, merkletree* root);
bool InclusionProof_Verify(unsigned char* msg, vector<unsigned char*> proof, merkletree* root);
vector<unsigned char*> ExclusionProof_Set(unsigned char* msg, merkletree* root);
bool ExclusionProof_Verify(unsigned char* msg, vector<unsigned char*> proof, merkletree* root);

//测试函数
void merkletree_leaf_sort_test();
void merkletree_build_test();
void merkletree_InclusionProof_test();
void merkletree_ExclusionProof_test();