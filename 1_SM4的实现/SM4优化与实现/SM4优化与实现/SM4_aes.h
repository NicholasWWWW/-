#pragma once
#include"SM4.h"
#include<immintrin.h>
__m256i aes_SBOX(__m256i x);
void SM4_aes_encrypt(uint8_t* input, uint8_t* enc_result, Keys* round_keys);
void SM4_aes_decrypt(uint8_t* input, uint8_t* dnc_result, Keys* round_keys);
void mm_print(__m256i x);