#include"SM4.h"

uint32_t rotate_left(uint32_t val, int b) {
    return  (val << b) | (val >> (32 - b));
}

void SM4_Key_set(uint8_t* key, Keys* round_keys) {
    // 初始密钥
    uint32_t k[4];
    uint32_t tmp;
    for (int i = 0; i < 16; i = i + 4) {
        k[i / 4] = (key[i] << 24) | (key[i + 1] << 16) | (key[i + 2] << 8) | (key[i + 3]);
        k[i / 4] = k[i / 4] ^ FK[i / 4];
    }
    for (int i = 0; i < 32; i++) {
        uint32_t tmp = k[1] ^ k[2] ^ k[3] ^ CK[i];

        // S 
        tmp = (SBox[uint8_t(tmp >> 24)] << 24) | (SBox[uint8_t(tmp >> 16)]) << 16 |
            (SBox[uint8_t(tmp >> 8)]) << 8 | (SBox[uint8_t(tmp)]);
        //L
        round_keys->rk[i] = k[0] ^ tmp ^ ((tmp << 13) | (tmp >> 19)) ^ ((tmp << 23) | (tmp >> 9));
        k[0] = k[1];
        k[1] = k[2];
        k[2] = k[3];
        k[3] = round_keys->rk[i];
    }
}
void SM4_encrypt(uint8_t* input, uint8_t* enc_result, Keys* round_keys) {
    uint32_t x[4];
    //载入需要加密的内容
    for (int i = 0; i < 16; i = i + 4) {
        x[i / 4] = (input[i] << 24) | (input[i + 1] << 16) | (input[i + 2] << 8) | (input[i + 3]);
    }
    //32轮轮函数
    for (int i = 0; i < 32; i++) {
        //异或轮密钥
        uint32_t tmp = x[1] ^ x[2] ^ x[3] ^ round_keys->rk[i];
        //过S盒
        uint8_t b0 = SBox[uint8_t(tmp >> 24)];
        uint8_t b1 = SBox[uint8_t(tmp >> 16)];
        uint8_t b2 = SBox[uint8_t(tmp >> 8)];
        uint8_t b3 = SBox[uint8_t(tmp >> 0)];
        tmp = (b0 << 24) | (b1 << 16) | (b2 << 8) | (b3 << 0);
        //线性变换L
        tmp = x[0] ^ ((tmp << 2) | (tmp >> 30)) ^ ((tmp << 10) | (tmp >> 22))
            ^ ((tmp << 18) | (tmp >> 14)) ^ ((tmp << 24) | (tmp >> 8));
        //交叉
        x[0] = x[1];
        x[1] = x[2];
        x[2] = x[3];
        x[3] = tmp;
    }
    //最后一轮反序输出
    uint32_t tmp = x[0];
    x[0] = x[3];
    x[3] = tmp;
    tmp = x[1];
    x[1] = x[2];
    x[2] = tmp;
    //大端序输出
    for (int i = 0; i < 4; i++) {
        enc_result[4 * i + 0] = uint8_t(x[i] >> 24);
        enc_result[4 * i + 1] = uint8_t(x[i] >> 16);
        enc_result[4 * i + 2] = uint8_t(x[i] >> 8);
        enc_result[4 * i + 3] = uint8_t(x[i] >> 0);
    }
}
void SM4_decrypt(uint8_t* input, uint8_t* dnc_result, Keys* round_keys) {
    uint32_t x[4];
    //载入需要解密的内容
    for (int i = 0; i < 16; i = i + 4) {
        x[i / 4] = (input[i] << 24) | (input[i + 1] << 16) | (input[i + 2] << 8) | (input[i + 3]);
    }
    //32轮轮函数
    for (int i = 0; i < 32; i++) {
        //异或轮密钥，解密时轮密钥倒过来处理
        uint32_t tmp = x[1] ^ x[2] ^ x[3] ^ round_keys->rk[32 - i - 1];
        //过S盒
        uint8_t b0 = SBox[uint8_t(tmp >> 24)];
        uint8_t b1 = SBox[uint8_t(tmp >> 16)];
        uint8_t b2 = SBox[uint8_t(tmp >> 8)];
        uint8_t b3 = SBox[uint8_t(tmp >> 0)];
        tmp = (b0 << 24) | (b1 << 16) | (b2 << 8) | (b3 << 0);
        //线性变换L
        tmp = x[0] ^ ((tmp << 2) | (tmp >> 30)) ^ ((tmp << 10) | (tmp >> 22))
            ^ ((tmp << 18) | (tmp >> 14)) ^ ((tmp << 24) | (tmp >> 8));
        //交叉
        x[0] = x[1];
        x[1] = x[2];
        x[2] = x[3];
        x[3] = tmp;
    }
    //最后一轮反序输出
    uint32_t tmp = x[0];
    x[0] = x[3];
    x[3] = tmp;
    tmp = x[1];
    x[1] = x[2];
    x[2] = tmp;
    //大端序输出
    for (int i = 0; i < 4; i++) {
        dnc_result[4 * i + 0] = uint8_t(x[i] >> 24);
        dnc_result[4 * i + 1] = uint8_t(x[i] >> 16);
        dnc_result[4 * i + 2] = uint8_t(x[i] >> 8);
        dnc_result[4 * i + 3] = uint8_t(x[i] >> 0);
    }
}