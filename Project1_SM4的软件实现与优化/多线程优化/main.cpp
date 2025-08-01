#include"SM4.h"
#include"SM4_aes.h"
int main() {
    unsigned char key[16 * 64] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
                             0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98,
                             0x76, 0x54, 0x32, 0x10 };
    unsigned char m[16 * 64] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
                             0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98,
                             0x76, 0x54, 0x32, 0x10 };
    for (int i = 16; i < 16 * 64; i++) {
        m[i] = m[i - 16];
    }
    Keys sm4_roundkeys;
    //轮密钥生成
    SM4_Key_set(key, &sm4_roundkeys);
    cout << "轮密钥：" << endl;
    for (int i = 0; i < 32; i++) {
        printf(" %02X", sm4_roundkeys.rk[i]);
    }
    cout << "\n消息m：" << endl;
    for (int i = 0; i < 16*64; i++) {
        if (i % 16 == 0)cout << endl;
        printf(" %02X", m[i]);
    }
    
    cout << "\n加密结果：" << endl;
    uint8_t enc[16 * 64] = { 0 };


    for (int i = 0; i < 64; i++) {
        SM4_encrypt(m+i*16, enc+i*16, &sm4_roundkeys);
    }
    
    for (int i = 0; i < 16 * 64; i++) {
        if (i % 16 == 0)cout << endl;
        printf(" %02X", enc[i]);
    }/**/
    
    
    cout << "\n解密：" << endl;
    uint8_t dnc[16*64] = { 0 };
    for (int i = 0; i < 64; i++) {
        SM4_decrypt(enc + i * 16, dnc + i * 16, &sm4_roundkeys);
    }
    for (int i = 0; i < 16*64; i++) {
        if (i % 16 == 0)cout << endl;
        printf(" %02X", dnc[i]);
    }/**/
    
    cout << endl;
    uint8_t sm4_aes_enc[32 * 32] = { 0 };
    for (int i = 0; i < 32; i += 4) {  // 每次处理 4 个块（128B）
        SM4_aes_encrypt(m + i * 32, sm4_aes_enc + i * 32, &sm4_roundkeys);
    }
    cout << "\n优化后加密结果：" << endl;
    for (int i = 0; i < 16 * 64; i++) {
        if (i % 16 == 0)cout << endl;
        printf(" %02X", sm4_aes_enc[i]);
    }

    cout << "\n优化后解密结果：" << endl;
    uint8_t sm4_aes_dnc[32*32] = { 0 };
    for (int i = 0; i < 32; i += 4) {  // 每次处理 4 个块（128B）
        SM4_aes_decrypt(sm4_aes_enc + i * 32, sm4_aes_dnc + i * 32, &sm4_roundkeys);
    }
    for (int i = 0; i < 16 * 64; i++) {
        if (i % 16 == 0)cout << endl;
        printf(" %02X", sm4_aes_dnc[i]);
    } 

    //S盒测试 
    /*
    uint8_t m1[16 * 16] = { 0 };
    for (int i = 0; i < 256; i++) {
        m1[i] = i;
    }

   

    __m256i tmp[16];
    cout << "\ntest" << endl;
    for (int i = 0; i < 8; i++) {
        tmp[i] = _mm256_loadu_si256((const __m256i*)m1 + i);
        tmp[i] = aes_SBOX(tmp[i]);
        mm_print(tmp[i]);
    }


    for (int i = 0; i < 256 ; i++) {
        if (i % 16 == 0)cout << endl;
        printf(" %02X", SBox[m1[i]]);
    }
    */
}