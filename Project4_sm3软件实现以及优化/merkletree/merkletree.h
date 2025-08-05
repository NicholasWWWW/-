#pragma once
#include"sm3.h"
#include <random>
#include<string>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace std;
void msg_get(unsigned char* output);

struct merkletree_node {
	unsigned char hash[64];
	int level = 0;
	merkletree_node* left;
	merkletree_node* right;
	merkletree_node* parent = NULL;
};
struct merkletree_leaf {
	merkletree_node* node;
	unsigned char msg[64];
};
struct merkletree_floor {
	merkletree_node* node ;
	merkletree_floor* next = NULL;
};
struct merkletree {
	merkletree_node* root;
	merkletree_leaf* leaves;
	int leaf_size;
};

void merkletree_merge(merkletree_node* left, merkletree_node* right, merkletree_node* parent); 
void merkletree_set(merkletree* tree,int leaf_size);
void node_print(merkletree_node* node);
void merkletree_floor_print(merkletree_floor* floor);
void merkletree_test();
void merkletree_leaf_sort(merkletree* tree,int* end);
void merkletree_print(merkletree_node* tree);