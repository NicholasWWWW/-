#pragma once
#include"sm3.h"
#include <random>
#include<string>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace std;

//���ڵ㣬������ϣֵ���ڵ��Σ������ӽڵ㼰���ڵ�ָ��
struct merkletree_node {
	unsigned char hash[64];
	int level = 0;
	merkletree_node* left ;
	merkletree_node* right ;
	merkletree_node* parent =NULL;
};
//��Ҷ��,������Ϣ��ָ���Ӧ�ڵ��ָ��
struct merkletree_leaf {
	merkletree_node* node ;
	unsigned char msg[64];
};
//���Ĳ�Σ���������build
struct merkletree_floor {
	merkletree_node* node ;
	merkletree_floor* next ;
};
//��������ṹ���������ڵ㣬Ҷ�ӽڵ������Ҷ�ӽڵ�����
struct merkletree {
	merkletree_node* root ;
	merkletree_leaf* leaves;
	int leaf_size;
};

//�����Ϣ���ɺ���
void msg_get(unsigned char* output);

//�����ڵ㣬���Ĳ�Σ�Ҷ�ӽڵ㣬�������Ĵ�ӡ����
void node_print(merkletree_node* node);
void merkletree_floor_print(merkletree_floor* floor);
void merkletree_leaves_print(merkletree* tree);
void merkletree_print(merkletree_node* tree);

//�ڵ�ϲ�������Ҷ�ӽڵ������������ڲ�������֤������Merkle�����ɺ���
void merkletree_merge(merkletree_node* left, merkletree_node* right, merkletree_node* parent); 
void merkletree_leaf_sort(merkletree* tree,int* end);
void merkletree_build(merkletree* tree,int leaf_size);

//������֤������������֤��������֤����
vector<unsigned char*> InclusionProof_Set(unsigned char* msg, merkletree* root);
bool InclusionProof_Verify(unsigned char* msg, vector<unsigned char*> proof, merkletree* root);
vector<unsigned char*> ExclusionProof_Set(unsigned char* msg, merkletree* root);
bool ExclusionProof_Verify(unsigned char* msg, vector<unsigned char*> proof, merkletree* root);

//���Ժ���
void merkletree_leaf_sort_test();
void merkletree_build_test();
void merkletree_InclusionProof_test();
void merkletree_ExclusionProof_test();