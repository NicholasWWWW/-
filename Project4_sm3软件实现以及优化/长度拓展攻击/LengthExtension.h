#pragma once
#include "sm3.h"
#include <random>
#include<string>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cmath>

void MAC(unsigned char* msg, size_t msg_size, unsigned char* key, size_t key_size, unsigned char* output);
bool MAC_verfly(unsigned char* msg, size_t msg_size, unsigned char* key, size_t key_size, unsigned char* mac);
void msg_get(unsigned char* output);
void MAC_test();
void MAC_forge_test();