#include"sm3.h"


void ctx_print(sm3_ctx* ctx) {
	cout << "State: ";
	for (int i = 0; i < 8; i++) {
		cout << hex << ctx->state[i] << " ";
	}
	cout << endl;
	cout << "Buffer Length: " << ctx->buf_len << endl;
	cout << "Compressed Length: " << ctx->compressed_len << endl;
}
void buf_print(sm3_ctx* ctx) {
	cout << "Buffer: ";
	for (int i = 0; i < 64; i++) {
		cout << hex << (int)ctx->buf[i] << " ";
	}
	cout << endl;
}

//初始化常量
void sm3_init(sm3_ctx* ctx) {
	ctx->state[0] = 0x7380166f;
	ctx->state[1] = 0x4914b2b9;
	ctx->state[2] = 0x172442d7;
	ctx->state[3] = 0xda8a0600;
	ctx->state[4] = 0xa96f30bc;
	ctx->state[5] = 0x163138aa;
	ctx->state[6] = 0xe38dee4d;
	ctx->state[7] = 0xb0fb0e4e;
	ctx->buf_len = 0;
	ctx->compressed_len = 0;
}


void sm3_input(sm3_ctx* ctx, const unsigned char* input, unsigned int length) {
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

void sm3_do(sm3_ctx* ctx, unsigned char* output) {
	//填充数据
	unsigned int i = ctx->buf_len;
	unsigned char msglen[8];
	uint64_t  total_len, high, low;

	ctx->buf[i++] = 0x80; //添加一个1位
	if (i > 56) { //如果填充后超过56字节，则需要先压缩一次
		while (i < 64) {
			ctx->buf[i++] = 0x00;
		}
		sm3_compress(ctx);
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


	sm3_compress(ctx); 

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
void sm3_compress(sm3_ctx* ctx) {
	uint32_t W[68],W1[64];


	// 读取输入数据
	GM_GET_UINT32_BE(W[0], ctx->buf, 0);
	GM_GET_UINT32_BE(W[1], ctx->buf, 4);
	GM_GET_UINT32_BE(W[2], ctx->buf, 8);
	GM_GET_UINT32_BE(W[3], ctx->buf, 12);
	GM_GET_UINT32_BE(W[4], ctx->buf, 16);
	GM_GET_UINT32_BE(W[5], ctx->buf, 20);
	GM_GET_UINT32_BE(W[6], ctx->buf, 24);
	GM_GET_UINT32_BE(W[7], ctx->buf, 28);
	GM_GET_UINT32_BE(W[8], ctx->buf, 32);
	GM_GET_UINT32_BE(W[9], ctx->buf, 36);
	GM_GET_UINT32_BE(W[10], ctx->buf, 40);
	GM_GET_UINT32_BE(W[11], ctx->buf, 44);
	GM_GET_UINT32_BE(W[12], ctx->buf, 48);
	GM_GET_UINT32_BE(W[13], ctx->buf, 52);
	GM_GET_UINT32_BE(W[14], ctx->buf, 56);
	GM_GET_UINT32_BE(W[15], ctx->buf, 60);


	// 消息扩展
	for (int i = 16; i <= 67; i++)
	{
		unsigned int tmp;
		tmp = W[i - 16] ^ W[i - 9] ^ ROTL(W[i - 3], 15);
		W[i] = P_1(tmp) ^ (ROTL(W[i - 13], 7)) ^ W[i - 6];
	}

	for (int i = 0; i <= 63; i++)
	{
		W1[i] = W[i] ^ W[i + 4];
	}

	cout << endl;
	cout << "W1:" << endl;
	for (int i = 0; i < 64; i++)
	{
		cout << hex << W1[i] << endl;
	}


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

void sm3(unsigned char* input, unsigned int iLen, unsigned char* output) {
	sm3_ctx ctx;
	sm3_init(&ctx);
	sm3_input(&ctx, input, iLen);
	sm3_do(&ctx, output);
}

void sm3_test() {
	unsigned char input[] = "abcdefghijklmn";
	unsigned char output[32];
	sm3(input, sizeof(input) - 1, output);
	cout << "要加密的信息: " << input << endl;
	cout << "SM3 Hash加密结果: ";
	for (int i = 0; i < 32; i++) {
		cout << hex << (int)output[i];
	}

}