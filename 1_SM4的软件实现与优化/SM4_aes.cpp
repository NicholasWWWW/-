#include"SM4.h"
#include<immintrin.h>
void SM4_aes_encrypt(uint8_t* input, uint8_t* enc_result, Keys* round_keys) {
    //采用avx指令集
    __m256i x[4], tmp[4],output[4];
    //加载数据
    tmp[0] = _mm256_loadu_si256((const __m256i*)input + 0);
    tmp[1] = _mm256_loadu_si256((const __m256i*)input + 1);
    tmp[2] = _mm256_loadu_si256((const __m256i*)input + 2);
    tmp[3] = _mm256_loadu_si256((const __m256i*)input + 3);
    //重新打包
    x[0] = _mm256_unpacklo_epi64(_mm256_unpacklo_epi32(tmp[0], tmp[1]), _mm256_unpacklo_epi32(tmp[2], tmp[3]));
    x[1] = _mm256_unpackhi_epi64(_mm256_unpacklo_epi32(tmp[0], tmp[1]), _mm256_unpacklo_epi32(tmp[2], tmp[3]));
    x[2] = _mm256_unpacklo_epi64(_mm256_unpackhi_epi32(tmp[0], tmp[1]), _mm256_unpackhi_epi32(tmp[2], tmp[3]));
    x[3] = _mm256_unpackhi_epi64(_mm256_unpackhi_epi32(tmp[0], tmp[1]), _mm256_unpackhi_epi32(tmp[2], tmp[3]));
    //打包结果测试
    _mm256_storeu_si256((__m256i*)output + 0, x[0]);
    _mm256_storeu_si256((__m256i*)output + 1, x[1]);
    _mm256_storeu_si256((__m256i*)output + 2, x[2]);
    _mm256_storeu_si256((__m256i*)output + 3, x[3]);
    for (int i = 0; i < 4; i++) {
        cout << endl;
        uint8_t* p = (uint8_t*)&output[i];
        for (int j = 0; j < 32; j++) {
            printf("%02X ", p[j]);
        }
    }
}
void SM4_aes_decrypt(uint8_t* input, uint8_t* dnc_result, Keys* round_keys) {

}