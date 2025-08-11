#pragma once
#include<iostream>
using namespace std;

//¶ËÐò×ª»»
#define GM_GET_UINT32_BE(n,b,i)                         \
{                                                       \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )             \
        | ( (uint32_t) (b)[(i) + 1] << 16 )             \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 3]       );            \
}

#define GM_PUT_UINT32_BE(n, b ,i)                            \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}

//Ñ­»µ×óÒÆ
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

//º¯ÊýP0£¬P1£¬FFj£¬GGj
#define P_0(x) ((x) ^ ROTL((x), 9) ^ ROTL((x), 17))
#define P_1(x) ((x) ^ ROTL((x), 15) ^ ROTL((x), 23))


#define FF_0(x, y, z) ((x) ^ (y) ^ (z))
#define FF_1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))

#define GG_0(x, y, z) ((x) ^ (y) ^ (z))
#define GG_1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))

struct sm3_ctx
{
	uint32_t state[8];
    unsigned char buf[64] = {0};
	unsigned int buf_len;
	uint64_t compressed_len;
};

const uint32_t T_0 = 0x79cc4519, T_1 = 0x7a879d8a;

void sm3_init(sm3_ctx* ctx);
void sm3_do(sm3_ctx* ctx, unsigned char* output);
void sm3_compress(sm3_ctx* ctx);
void sm3_test();
void ctx_print(sm3_ctx* ctx);
void buf_print(sm3_ctx* ctx);