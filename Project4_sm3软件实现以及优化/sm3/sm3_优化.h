#include<iostream>
#include<immintrin.h>
#include"sm3.h"
using namespace std;

#define ROTL_32(x, n) _mm_xor_si128(_mm_slli_epi32((x), (n)), _mm_srli_epi32((x), (32 - (n))))


void sm3_pro_input(sm3_ctx* ctx, const unsigned char* input, unsigned int length);
void sm3_pro_do(sm3_ctx* ctx, unsigned char* output);
void sm3_pro_compress(sm3_ctx* ctx);
void sm3_pro_test();
void mm_print_128(__m128i x);

