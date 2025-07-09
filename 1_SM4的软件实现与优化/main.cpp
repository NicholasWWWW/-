#include"SM4.h"
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
    /*
    cout << "\n加密：" << endl;
    uint8_t enc[16*64] = { 0 };
    for (int i = 0; i < 64; i++) {
        SM4_encrypt(m+i*16, enc+i*16, &sm4_roundkeys);
    }
    for (int i = 0; i < 16*64; i++) {
        printf(" %02X", enc[i]);
    }
    cout << "\n解密：" << endl;
    uint8_t dnc[16*64] = { 0 };
    for (int i = 0; i < 64; i++) {
        SM4_decrypt(enc + i * 16, dnc + i * 16, &sm4_roundkeys);
    }
    for (int i = 0; i < 16*64; i++) {
        printf(" %02X", dnc[i]);
    }
    */
    uint8_t enc[16 * 64] = { 0 };
    SM4_aes_encrypt(m, enc, &sm4_roundkeys);
    return 0;
}