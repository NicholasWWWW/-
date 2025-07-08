#pragma once
#include<iostream>
using namespace std;
struct Keys {
	uint32_t rk[32];
};;

void SM4_Key_set(uint8_t* key, Keys* round_keys);//ÃÜÔ¿Éú³É
void SM4_encrypt(uint8_t* input, uint8_t* enc_result, Keys* round_keys);
void SM4_decrypt(uint8_t* input, uint8_t* dnc_result, Keys* round_keys);