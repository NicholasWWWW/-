#include"sm3_优化.h"
#include"sm3.h"
void mm_print_128(__m128i x) {
	__m128i output;
	_mm_storeu_si128((__m128i*) & output, x);
	uint8_t* p = (uint8_t*)&output;
	for (int j = 0; j < 16; j++) {
		printf("%02X ", p[j]);
	}
	cout << endl;
}
void sm3_pro_input(sm3_ctx* ctx, const unsigned char* input, unsigned int length) {
	//将输入数据添加到缓冲区
	for (unsigned int i = 0; i < length; i++) {
		ctx->buf[ctx->buf_len] = input[i];
		ctx->buf_len++;
		if (ctx->buf_len == 64) { //如果缓冲区满了，则进行压缩
			sm3_compress(ctx);
			ctx->compressed_len += 512;
			ctx->buf_len = 0;
		}
	}
}

void sm3_pro_do(sm3_ctx* ctx, unsigned char* output) {
	//填充数据
	unsigned int i = ctx->buf_len;
	unsigned char msglen[8];
	uint64_t  total_len, high, low;

	ctx->buf[i++] = 0x80; //添加一个1位
	if (i > 56) { //如果填充后超过56字节，则需要先压缩一次
		while (i < 64) {
			ctx->buf[i++] = 0x00;
		}
		sm3_pro_compress(ctx);
		i = 0;
	}
	while (i < 56) { //填充到56字节
		ctx->buf[i++] = 0x00;
	}

	//添加长度信息
	total_len = ctx->compressed_len + (ctx->buf_len * 8);
	high = (total_len >> 32) & 0x0FFFFFFFF;
	low = (total_len) & 0x0FFFFFFFF;

	GM_PUT_UINT32_BE(high, msglen, 0);
	GM_PUT_UINT32_BE(low, msglen, 4);

	for (int i = 0; i < 8; i++) {
		ctx->buf[56 + i] = msglen[i];
	}


	sm3_pro_compress(ctx);

	GM_PUT_UINT32_BE(ctx->state[0], output, 0);
	GM_PUT_UINT32_BE(ctx->state[1], output, 4);
	GM_PUT_UINT32_BE(ctx->state[2], output, 8);
	GM_PUT_UINT32_BE(ctx->state[3], output, 12);
	GM_PUT_UINT32_BE(ctx->state[4], output, 16);
	GM_PUT_UINT32_BE(ctx->state[5], output, 20);
	GM_PUT_UINT32_BE(ctx->state[6], output, 24);
	GM_PUT_UINT32_BE(ctx->state[7], output, 28);
}


//压缩函数
void sm3_pro_compress(sm3_ctx* ctx) {
	__m128i xmm[16] = {0},tmp;
	uint32_t W1[64] = {0};
	uint32_t W[64] = { 0 };

	xmm[0] = _mm_loadu_si128((const __m128i*)(ctx->buf + 0));   // 字 0,1,2,3
	xmm[1] = _mm_loadu_si128((const __m128i*)(ctx->buf + 12));  // 字 3,4,5,6
	xmm[2] = _mm_loadu_si128((const __m128i*)(ctx->buf + 24));  // 字 6,7,8,9
	xmm[3] = _mm_loadu_si128((const __m128i*)(ctx->buf + 36));  // 字 9,10,11,12
	xmm[4] = _mm_loadu_si128((const __m128i*)(ctx->buf + 48));  // 字 12,13,14,15

	//换为大端序
	__m128i vindex = _mm_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);
	xmm[0] = _mm_shuffle_epi8(xmm[0], vindex);
	xmm[1] = _mm_shuffle_epi8(xmm[1], vindex);
	xmm[2] = _mm_shuffle_epi8(xmm[2], vindex);
	xmm[3] = _mm_shuffle_epi8(xmm[3], vindex);
	xmm[4] = _mm_shuffle_epi8(xmm[4], vindex);

	//消息拓展

	cout << "xmm:" << endl;
	mm_print_128(xmm[0]);
	mm_print_128(xmm[1]);
	mm_print_128(xmm[2]);
	mm_print_128(xmm[3]);
	mm_print_128(xmm[4]);



	// 第1段
	_mm_storeu_si128((__m128i*)W, xmm[0]);
	xmm[7] = ROTL_32(xmm[4], 15);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[2]);
	xmm[7] = _mm_shuffle_epi32(xmm[7], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[6] = _mm_xor_si128(xmm[7], xmm[0]);
	xmm[7] = ROTL_32(xmm[6], 23);
	xmm[9] = ROTL_32(xmm[6], 15);
	xmm[6] = _mm_xor_si128(xmm[6], xmm[7]);
	xmm[6] = _mm_xor_si128(xmm[6], xmm[9]);
	xmm[5] = _mm_shuffle_epi32(xmm[3], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[9] = ROTL_32(xmm[1], 7);
	xmm[9] = _mm_xor_si128(xmm[9], xmm[5]);
	xmm[15] = _mm_xor_si128(xmm[0], _mm_shuffle_epi32(xmm[1], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[0] = _mm_xor_si128(xmm[6], xmm[9]);
	xmm[5] = _mm_alignr_epi8(xmm[0], xmm[4], 12);
	_mm_storeu_si128((__m128i*)W1, xmm[15]);



	// 第2段
	_mm_storeu_si128((__m128i*)(W + 3), xmm[1]);
	xmm[8] = ROTL_32(xmm[5], 15);
	xmm[8] = _mm_xor_si128(xmm[8], xmm[3]);
	xmm[8] = _mm_shuffle_epi32(xmm[8], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[7] = _mm_xor_si128(xmm[8], xmm[1]);
	xmm[8] = ROTL_32(xmm[7], 23);
	xmm[10] = ROTL_32(xmm[7], 15);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[8]);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[10]);
	xmm[6] = _mm_shuffle_epi32(xmm[4], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[10] = ROTL_32(xmm[2], 7);
	xmm[10] = _mm_xor_si128(xmm[10], xmm[6]);
	xmm[0] = _mm_xor_si128(xmm[1], _mm_shuffle_epi32(xmm[2], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[1] = _mm_xor_si128(xmm[7], xmm[10]);
	xmm[6] = _mm_alignr_epi8(xmm[1], xmm[5], 12); // W19-W21
	_mm_storeu_si128((__m128i*)(W1+3), xmm[0]);

	// 第3段
	_mm_storeu_si128((__m128i*)(W + 6), xmm[2]);
	xmm[9] = ROTL_32(xmm[6], 15);
	xmm[9] = _mm_xor_si128(xmm[9], xmm[4]);
	xmm[9] = _mm_shuffle_epi32(xmm[9], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[8] = _mm_xor_si128(xmm[9], xmm[2]);
	xmm[9] = ROTL_32(xmm[8], 23);
	xmm[11] = ROTL_32(xmm[8], 15);
	xmm[8] = _mm_xor_si128(xmm[8], xmm[9]);
	xmm[8] = _mm_xor_si128(xmm[8], xmm[11]);
	xmm[7] = _mm_shuffle_epi32(xmm[5], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[11] = ROTL_32(xmm[3], 7);
	xmm[11] = _mm_xor_si128(xmm[11], xmm[7]);
	xmm[1] = _mm_xor_si128(xmm[2], _mm_shuffle_epi32(xmm[3], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[2] = _mm_xor_si128(xmm[8], xmm[11]);
	xmm[7] = _mm_alignr_epi8(xmm[2], xmm[6], 12); // W22-W24
	_mm_storeu_si128((__m128i*)(W1+6), xmm[1]);

	// 第4段
	_mm_storeu_si128((__m128i*)(W + 9), xmm[3]);
	xmm[10] = ROTL_32(xmm[7], 15);
	xmm[10] = _mm_xor_si128(xmm[10], xmm[5]);
	xmm[10] = _mm_shuffle_epi32(xmm[10], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[9] = _mm_xor_si128(xmm[10], xmm[3]);
	xmm[10] = ROTL_32(xmm[9], 23);
	xmm[12] = ROTL_32(xmm[9], 15);
	xmm[9] = _mm_xor_si128(xmm[9], xmm[10]);
	xmm[9] = _mm_xor_si128(xmm[9], xmm[12]);
	xmm[8] = _mm_shuffle_epi32(xmm[6], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[12] = ROTL_32(xmm[4], 7);
	xmm[12] = _mm_xor_si128(xmm[12], xmm[8]);
	xmm[2] = _mm_xor_si128(xmm[3], _mm_shuffle_epi32(xmm[4], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[3] = _mm_xor_si128(xmm[9], xmm[12]);
	xmm[8] = _mm_alignr_epi8(xmm[3], xmm[7], 12); // W25-W27
	_mm_storeu_si128((__m128i*)(W1 + 9), xmm[2]);

	// 第5段
	_mm_storeu_si128((__m128i*)(W + 12), xmm[4]);
	xmm[11] = ROTL_32(xmm[8], 15);
	xmm[11] = _mm_xor_si128(xmm[11], xmm[6]);
	xmm[11] = _mm_shuffle_epi32(xmm[11], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[10] = _mm_xor_si128(xmm[11], xmm[4]);
	xmm[11] = ROTL_32(xmm[10], 23);
	xmm[13] = ROTL_32(xmm[10], 15);
	xmm[10] = _mm_xor_si128(xmm[10], xmm[11]);
	xmm[10] = _mm_xor_si128(xmm[10], xmm[13]);
	xmm[9] = _mm_shuffle_epi32(xmm[7], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[13] = ROTL_32(xmm[5], 7);
	xmm[13] = _mm_xor_si128(xmm[13], xmm[9]);
	xmm[3] = _mm_xor_si128(xmm[4], _mm_shuffle_epi32(xmm[5], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[4] = _mm_xor_si128(xmm[10], xmm[13]);
	xmm[9] = _mm_alignr_epi8(xmm[4], xmm[8], 12); // W28-W30
	_mm_storeu_si128((__m128i*)(W1 + 12), xmm[3]);

	// 第6段
	_mm_storeu_si128((__m128i*)(W + 15), xmm[5]);
	xmm[12] = ROTL_32(xmm[9], 15);
	xmm[12] = _mm_xor_si128(xmm[12], xmm[7]);
	xmm[12] = _mm_shuffle_epi32(xmm[12], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[11] = _mm_xor_si128(xmm[12], xmm[5]);
	xmm[12] = ROTL_32(xmm[11], 23);
	xmm[14] = ROTL_32(xmm[11], 15);
	xmm[11] = _mm_xor_si128(xmm[11], xmm[12]);
	xmm[11] = _mm_xor_si128(xmm[11], xmm[14]);
	xmm[10] = _mm_shuffle_epi32(xmm[8], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[14] = ROTL_32(xmm[6], 7);
	xmm[14] = _mm_xor_si128(xmm[14], xmm[10]);
	xmm[4] = _mm_xor_si128(xmm[5], _mm_shuffle_epi32(xmm[6], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[5] = _mm_xor_si128(xmm[11], xmm[14]);
	xmm[10] = _mm_alignr_epi8(xmm[5], xmm[9], 12); // W31-W33
	_mm_storeu_si128((__m128i*)(W1 + 15), xmm[4]);

	// 第7段
	_mm_storeu_si128((__m128i*)(W + 18), xmm[6]);
	xmm[13] = ROTL_32(xmm[10], 15);
	xmm[13] = _mm_xor_si128(xmm[13], xmm[8]);
	xmm[13] = _mm_shuffle_epi32(xmm[13], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[12] = _mm_xor_si128(xmm[13], xmm[6]);
	xmm[13] = ROTL_32(xmm[12], 23);
	xmm[15] = ROTL_32(xmm[12], 15);
	xmm[12] = _mm_xor_si128(xmm[12], xmm[13]);
	xmm[12] = _mm_xor_si128(xmm[12], xmm[15]);
	xmm[11] = _mm_shuffle_epi32(xmm[9], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[15] = ROTL_32(xmm[7], 7);
	xmm[15] = _mm_xor_si128(xmm[15], xmm[11]);
	xmm[5] = _mm_xor_si128(xmm[6], _mm_shuffle_epi32(xmm[7], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[6] = _mm_xor_si128(xmm[12], xmm[15]);
	xmm[11] = _mm_alignr_epi8(xmm[6], xmm[10], 12); // W34-W36
	_mm_storeu_si128((__m128i*)(W1 + 18), xmm[5]);

	// 第8段
	_mm_storeu_si128((__m128i*)(W + 21), xmm[7]);
	xmm[14] = ROTL_32(xmm[11], 15);
	xmm[14] = _mm_xor_si128(xmm[14], xmm[9]);
	xmm[14] = _mm_shuffle_epi32(xmm[14], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[13] = _mm_xor_si128(xmm[14], xmm[7]);
	xmm[14] = ROTL_32(xmm[13], 23);
	xmm[0] = ROTL_32(xmm[13], 15);
	xmm[13] = _mm_xor_si128(xmm[13], xmm[14]);
	xmm[13] = _mm_xor_si128(xmm[13], xmm[0]);
	xmm[12] = _mm_shuffle_epi32(xmm[10], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[0] = ROTL_32(xmm[8], 7);
	xmm[0] = _mm_xor_si128(xmm[0], xmm[12]);
	xmm[6] = _mm_xor_si128(xmm[7], _mm_shuffle_epi32(xmm[8], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[7] = _mm_xor_si128(xmm[13], xmm[0]);
	xmm[12] = _mm_alignr_epi8(xmm[7], xmm[11], 12); // W37-W39
	_mm_storeu_si128((__m128i*)(W1 + 21), xmm[6]);

	// 第9段
	_mm_storeu_si128((__m128i*)(W + 24), xmm[8]);
	xmm[15] = ROTL_32(xmm[12], 15);
	xmm[15] = _mm_xor_si128(xmm[15], xmm[10]);
	xmm[15] = _mm_shuffle_epi32(xmm[15], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[14] = _mm_xor_si128(xmm[15], xmm[8]);
	xmm[15] = ROTL_32(xmm[14], 23);
	xmm[1] = ROTL_32(xmm[14], 15);
	xmm[14] = _mm_xor_si128(xmm[14], xmm[15]);
	xmm[14] = _mm_xor_si128(xmm[14], xmm[1]);
	xmm[13] = _mm_shuffle_epi32(xmm[11], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[1] = ROTL_32(xmm[9], 7);
	xmm[1] = _mm_xor_si128(xmm[1], xmm[13]);
	xmm[7] = _mm_xor_si128(xmm[8], _mm_shuffle_epi32(xmm[9], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[8] = _mm_xor_si128(xmm[14], xmm[1]);
	xmm[13] = _mm_alignr_epi8(xmm[8], xmm[12], 12); // W40-W42
	_mm_storeu_si128((__m128i*)(W1 + 24), xmm[7]);

	// 第10段
	_mm_storeu_si128((__m128i*)(W + 27), xmm[9]);
	xmm[0] = ROTL_32(xmm[13], 15);
	xmm[0] = _mm_xor_si128(xmm[0], xmm[11]);
	xmm[0] = _mm_shuffle_epi32(xmm[0], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[15] = _mm_xor_si128(xmm[0], xmm[9]);
	xmm[0] = ROTL_32(xmm[15], 23);
	xmm[2] = ROTL_32(xmm[15], 15);
	xmm[15] = _mm_xor_si128(xmm[15], xmm[0]);
	xmm[15] = _mm_xor_si128(xmm[15], xmm[2]);
	xmm[14] = _mm_shuffle_epi32(xmm[12], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[2] = ROTL_32(xmm[10], 7);
	xmm[2] = _mm_xor_si128(xmm[2], xmm[14]);
	xmm[8] = _mm_xor_si128(xmm[9], _mm_shuffle_epi32(xmm[10], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[9] = _mm_xor_si128(xmm[15], xmm[2]);
	xmm[14] = _mm_alignr_epi8(xmm[9], xmm[13], 12); // W43-W45
	_mm_storeu_si128((__m128i*)(W1 + 27), xmm[8]);

	// 第11段
	_mm_storeu_si128((__m128i*)(W + 30), xmm[10]);
	xmm[1] = ROTL_32(xmm[14], 15);
	xmm[1] = _mm_xor_si128(xmm[1], xmm[12]);
	xmm[1] = _mm_shuffle_epi32(xmm[1], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[0] = _mm_xor_si128(xmm[1], xmm[10]);
	xmm[1] = ROTL_32(xmm[0], 23);
	xmm[3] = ROTL_32(xmm[0], 15);
	xmm[0] = _mm_xor_si128(xmm[0], xmm[1]);
	xmm[0] = _mm_xor_si128(xmm[0], xmm[3]);
	xmm[15] = _mm_shuffle_epi32(xmm[13], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[3] = ROTL_32(xmm[11], 7);
	xmm[3] = _mm_xor_si128(xmm[3], xmm[15]);
	xmm[9] = _mm_xor_si128(xmm[10], _mm_shuffle_epi32(xmm[11], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[10] = _mm_xor_si128(xmm[0], xmm[3]);
	xmm[15] = _mm_alignr_epi8(xmm[10], xmm[14], 12); // W46-W48
	_mm_storeu_si128((__m128i*)(W1 + 30), xmm[9]);

	// 第12段
	_mm_storeu_si128((__m128i*)(W + 33), xmm[11]);
	xmm[2] = ROTL_32(xmm[15], 15);
	xmm[2] = _mm_xor_si128(xmm[2], xmm[13]);
	xmm[2] = _mm_shuffle_epi32(xmm[2], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[1] = _mm_xor_si128(xmm[2], xmm[11]);
	xmm[2] = ROTL_32(xmm[1], 23);
	xmm[4] = ROTL_32(xmm[1], 15);
	xmm[1] = _mm_xor_si128(xmm[1], xmm[2]);
	xmm[1] = _mm_xor_si128(xmm[1], xmm[4]);
	xmm[0] = _mm_shuffle_epi32(xmm[14], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[4] = ROTL_32(xmm[12], 7);
	xmm[4] = _mm_xor_si128(xmm[4], xmm[0]);
	xmm[10] = _mm_xor_si128(xmm[11], _mm_shuffle_epi32(xmm[12], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[11] = _mm_xor_si128(xmm[1], xmm[4]);
	xmm[0] = _mm_alignr_epi8(xmm[11], xmm[15], 12); // W49-W51
	_mm_storeu_si128((__m128i*)(W1 + 33), xmm[10]);

	// 第13段
	_mm_storeu_si128((__m128i*)(W + 36), xmm[12]);
	xmm[3] = ROTL_32(xmm[0], 15);
	xmm[3] = _mm_xor_si128(xmm[3], xmm[14]);
	xmm[3] = _mm_shuffle_epi32(xmm[3], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[2] = _mm_xor_si128(xmm[3], xmm[12]);
	xmm[3] = ROTL_32(xmm[2], 23);
	xmm[5] = ROTL_32(xmm[2], 15);
	xmm[2] = _mm_xor_si128(xmm[2], xmm[3]);
	xmm[2] = _mm_xor_si128(xmm[2], xmm[5]);
	xmm[1] = _mm_shuffle_epi32(xmm[15], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[5] = ROTL_32(xmm[13], 7);
	xmm[5] = _mm_xor_si128(xmm[5], xmm[1]);
	xmm[11] = _mm_xor_si128(xmm[12], _mm_shuffle_epi32(xmm[13], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[12] = _mm_xor_si128(xmm[2], xmm[5]);
	xmm[1] = _mm_alignr_epi8(xmm[12], xmm[0], 12); // W52-W54
	_mm_storeu_si128((__m128i*)(W1 + 36), xmm[11]);

	// 第14段
	_mm_storeu_si128((__m128i*)(W + 39), xmm[13]);
	xmm[4] = ROTL_32(xmm[1], 15);
	xmm[4] = _mm_xor_si128(xmm[4], xmm[15]);
	xmm[4] = _mm_shuffle_epi32(xmm[4], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[3] = _mm_xor_si128(xmm[4], xmm[13]);
	xmm[4] = ROTL_32(xmm[3], 23);
	xmm[6] = ROTL_32(xmm[3], 15);
	xmm[3] = _mm_xor_si128(xmm[3], xmm[4]);
	xmm[3] = _mm_xor_si128(xmm[3], xmm[6]);
	xmm[2] = _mm_shuffle_epi32(xmm[0], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[6] = ROTL_32(xmm[14], 7);
	xmm[6] = _mm_xor_si128(xmm[6], xmm[2]);
	xmm[12] = _mm_xor_si128(xmm[13], _mm_shuffle_epi32(xmm[14], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[13] = _mm_xor_si128(xmm[3], xmm[6]);
	xmm[2] = _mm_alignr_epi8(xmm[13], xmm[1], 12); // W55-W57
	_mm_storeu_si128((__m128i*)(W1 + 39), xmm[12]);

	// 第15段
	_mm_storeu_si128((__m128i*)(W + 42), xmm[14]);
	xmm[5] = ROTL_32(xmm[2], 15);
	xmm[5] = _mm_xor_si128(xmm[5], xmm[0]);
	xmm[5] = _mm_shuffle_epi32(xmm[5], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[4] = _mm_xor_si128(xmm[5], xmm[14]);
	xmm[5] = ROTL_32(xmm[4], 23);
	xmm[7] = ROTL_32(xmm[4], 15);
	xmm[4] = _mm_xor_si128(xmm[4], xmm[5]);
	xmm[4] = _mm_xor_si128(xmm[4], xmm[7]);
	xmm[3] = _mm_shuffle_epi32(xmm[1], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[7] = ROTL_32(xmm[15], 7);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[3]);
	xmm[13] = _mm_xor_si128(xmm[14], _mm_shuffle_epi32(xmm[15], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[14] = _mm_xor_si128(xmm[4], xmm[7]);
	xmm[3] = _mm_alignr_epi8(xmm[14], xmm[2], 12); // W58-W60
	_mm_storeu_si128((__m128i*)(W1 + 42), xmm[13]);

	// 第16段
	_mm_storeu_si128((__m128i*)(W + 45), xmm[15]);
	xmm[6] = ROTL_32(xmm[3], 15);
	xmm[6] = _mm_xor_si128(xmm[6], xmm[1]);
	xmm[6] = _mm_shuffle_epi32(xmm[6], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[5] = _mm_xor_si128(xmm[6], xmm[15]);
	xmm[6] = ROTL_32(xmm[5], 23);
	xmm[8] = ROTL_32(xmm[5], 15);
	xmm[5] = _mm_xor_si128(xmm[5], xmm[6]);
	xmm[5] = _mm_xor_si128(xmm[5], xmm[8]);
	xmm[4] = _mm_shuffle_epi32(xmm[2], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[8] = ROTL_32(xmm[0], 7);
	xmm[8] = _mm_xor_si128(xmm[8], xmm[4]);
	xmm[14] = _mm_xor_si128(xmm[15], _mm_shuffle_epi32(xmm[0], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[15] = _mm_xor_si128(xmm[5], xmm[8]);
	xmm[4] = _mm_alignr_epi8(xmm[15], xmm[3], 12); // W61-W63
	_mm_storeu_si128((__m128i*)(W1 + 45), xmm[14]);

	// 第17段
	_mm_storeu_si128((__m128i*)(W + 48), xmm[0]);
	xmm[7] = ROTL_32(xmm[4], 15);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[2]);
	xmm[7] = _mm_shuffle_epi32(xmm[7], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[6] = _mm_xor_si128(xmm[7], xmm[0]);
	xmm[7] = ROTL_32(xmm[6], 23);
	xmm[9] = ROTL_32(xmm[6], 15);
	xmm[6] = _mm_xor_si128(xmm[6], xmm[7]);
	xmm[6] = _mm_xor_si128(xmm[6], xmm[9]);
	xmm[5] = _mm_shuffle_epi32(xmm[3], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[9] = ROTL_32(xmm[1], 7);
	xmm[9] = _mm_xor_si128(xmm[9], xmm[5]);
	xmm[15] = _mm_xor_si128(xmm[0], _mm_shuffle_epi32(xmm[1], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[0] = _mm_xor_si128(xmm[6], xmm[9]);
	xmm[5] = _mm_alignr_epi8(xmm[0], xmm[4], 12); // W64-W66
	_mm_storeu_si128((__m128i*)(W1 + 48), xmm[15]);

	// 第18段
	_mm_storeu_si128((__m128i*)(W + 51), xmm[1]);
	_mm_storeu_si128((__m128i*)(W + 54), xmm[2]);
	_mm_storeu_si128((__m128i*)(W + 57), xmm[3]);
	_mm_storeu_si128((__m128i*)(W + 60), xmm[4]);
	_mm_storeu_si32((__m128i*)(W + 63), xmm[5]);
	xmm[8] = ROTL_32(xmm[5], 15);
	xmm[8] = _mm_xor_si128(xmm[8], xmm[3]);
	xmm[8] = _mm_shuffle_epi32(xmm[8], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[7] = _mm_xor_si128(xmm[8], xmm[1]);
	xmm[8] = ROTL_32(xmm[7], 23);
	xmm[10] = ROTL_32(xmm[7], 15);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[8]);
	xmm[7] = _mm_xor_si128(xmm[7], xmm[10]);
	xmm[6] = _mm_shuffle_epi32(xmm[4], _MM_SHUFFLE(0, 3, 2, 1));
	xmm[10] = ROTL_32(xmm[2], 7);
	xmm[10] = _mm_xor_si128(xmm[10], xmm[6]);
	xmm[0] = _mm_xor_si128(xmm[1], _mm_shuffle_epi32(xmm[2], _MM_SHUFFLE(0, 3, 2, 1)));
	xmm[1] = _mm_xor_si128(xmm[7], xmm[10]);
	xmm[6] = _mm_alignr_epi8(xmm[1], xmm[5], 12); // W67-W69
	_mm_storeu_si128((__m128i*)(W1 + 51), xmm[0]);
	_mm_storeu_si128((__m128i*)(W1 + 54), _mm_xor_si128(xmm[2], _mm_shuffle_epi32(xmm[3], _MM_SHUFFLE(0, 3, 2, 1))));
	_mm_storeu_si128((__m128i*)(W1 + 57), _mm_xor_si128(xmm[3], _mm_shuffle_epi32(xmm[4], _MM_SHUFFLE(0, 3, 2, 1))));
	_mm_storeu_si128((__m128i*)(W1 + 60), _mm_xor_si128(xmm[4], _mm_shuffle_epi32(xmm[5], _MM_SHUFFLE(0, 3, 2, 1))));
	_mm_storeu_si32((__m128i*)(W1 + 63), _mm_xor_si128(xmm[5], _mm_shuffle_epi32(xmm[6], _MM_SHUFFLE(0, 3, 2, 1))));

	cout << "W1:" << endl;
	for (int i = 0; i < 64; i++)
	{
		cout << hex << W1[i] << endl;
	}
	cout << endl;
	cout << "W:" << endl;
	for (int i = 0; i < 64; i++)
	{
		cout << hex << W[i] << endl;
	}
	cout << endl;
	return;
	//消息压缩
	unsigned int SS1;
	unsigned int SS2;
	unsigned int TT1;
	unsigned int TT2;
	unsigned int A, B, C, D, E, F, G, H;
	unsigned int Tj;
	int j;

	// ABCDEFGH = V (i)
	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];
	F = ctx->state[5];
	G = ctx->state[6];
	H = ctx->state[7];
	
	for (j = 0; j < 64; j++)
	{

		if (j < 16)
		{
			Tj = T_0;
		}
		else
		{
			Tj = T_1;
		}
		SS1 = ROTL((ROTL(A, 12) + E + ROTL(Tj, j)), 7);
		SS2 = SS1 ^ ROTL(A, 12);

		if (j < 16)
		{
			TT1 = FF_0(A, B, C) + D + SS2 + W1[j];
			TT2 = GG_0(E, F, G) + H + SS1 + W[j];
		}
		else
		{
			TT1 = FF_1(A, B, C) + D + SS2 + W1[j];
			TT2 = GG_1(E, F, G) + H + SS1 + W[j];
		}

		D = C;
		C = ROTL(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F, 19);
		F = E;
		E = P_0(TT2);
	}



	// V(i+1) = ABCDEFGH ^ V(i)
	ctx->state[0] ^= A;
	ctx->state[1] ^= B;
	ctx->state[2] ^= C;
	ctx->state[3] ^= D;
	ctx->state[4] ^= E;
	ctx->state[5] ^= F;
	ctx->state[6] ^= G;
	ctx->state[7] ^= H;

}

void sm3_pro(unsigned char* input, unsigned int iLen, unsigned char* output) {
	sm3_ctx ctx;
	sm3_init(&ctx);
	sm3_pro_input(&ctx, input, iLen);
	sm3_pro_do(&ctx, output);
}

void sm3_pro_test() {
	unsigned char input[] = "abcdefghijklmn";
	unsigned char output[32];
	sm3_pro(input, sizeof(input) - 1, output);
	cout << "要加密的信息: " << input << endl;
	cout << "SM3 Hash加密结果: ";
	for (int i = 0; i < 32; i++) {
		cout << hex << (int)output[i];
	}
}