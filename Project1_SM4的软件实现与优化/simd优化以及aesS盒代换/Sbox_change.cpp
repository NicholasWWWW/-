#include"SM4_aes.h"
#include"SM4.h"
void mm_print(__m256i x) {
    __m256i output;
    _mm256_storeu_si256((__m256i*) & output, x);
    uint8_t* p = (uint8_t*)&output;
    cout << endl;
    for (int j = 0; j < 32; j++) {
        printf("%02X ", p[j]);
    }
}
void mm_print_128(__m128i x) {
    __m128i output;
    _mm_storeu_si128((__m128i*) & output, x);
    uint8_t* p = (uint8_t*)&output;
    cout << endl;
    for (int j = 0; j < 16; j++) {
        printf("%02X ", p[j]);
    }
}
__m256i MulMatrix(__m256i x, __m256i higherMask, __m256i lowerMask) {
    __m256i tmp1, tmp2;
    __m256i andMask = _mm256_set1_epi32(0x0f0f0f0f); // 低4位掩码

    // 分离高4位和低4位
    tmp2 = _mm256_srli_epi16(x, 4);       // 右移4位，提取高4位
    
    tmp1 = _mm256_and_si256(x, andMask);  // 掩码操作，提取低4位
    tmp2 = _mm256_and_si256(tmp2, andMask);
    
    tmp1 = _mm256_shuffle_epi8(lowerMask, tmp1); // 低4位查表
    tmp2 = _mm256_shuffle_epi8(higherMask, tmp2); // 高4位查表

    // 合并
    tmp1 = _mm256_xor_si256(tmp1, tmp2);
    return tmp1;
}
__m256i MulMatrixATA(__m256i x) {
    // 高4位查表掩码
    __m256i higherMask = _mm256_setr_epi8(
        // 高128位
        0x00, 0x13, 0xd2, 0xc1, 0x78, 0x6b, 0xaa, 0xb9,
        0xad, 0xbe, 0x7f, 0x6c, 0xd5, 0xc6, 0x07, 0x14,
        // 低128位
        0x00, 0x13, 0xd2, 0xc1, 0x78, 0x6b, 0xaa, 0xb9,
        0xad, 0xbe, 0x7f, 0x6c, 0xd5, 0xc6, 0x07, 0x14
    );

    // 低4位查表掩码
    __m256i lowerMask = _mm256_setr_epi8(
        // 高128位
        0x00, 0x60, 0x22, 0x42, 0x1d, 0x7d, 0x3f, 0x5f,
        0x87, 0xe7, 0xa5, 0xc5, 0x9a, 0xfa, 0xb8, 0xd8,
        // 低128位
        0x00, 0x60, 0x22, 0x42, 0x1d, 0x7d, 0x3f, 0x5f,
        0x87, 0xe7, 0xa5, 0xc5, 0x9a, 0xfa, 0xb8, 0xd8
    );

    return MulMatrix(x, higherMask, lowerMask);
}
__m256i MulMatrixTA(__m256i x) {
    __m256i higherMask = _mm256_setr_epi8(
        // 高128位
        0x00, 0x7a, 0x38, 0x42, 0x20, 0x5a, 0x18, 0x62,
        0x40, 0x3a, 0x78, 0x02, 0x60, 0x1a, 0x58, 0x22,
        // 低128位
        0x00, 0x7a, 0x38, 0x42, 0x20, 0x5a, 0x18, 0x62,
        0x40, 0x3a, 0x78, 0x02, 0x60, 0x1a, 0x58, 0x22
    );

    __m256i lowerMask = _mm256_setr_epi8(
        // 高128位
        0x00, 0xca, 0x77, 0xbd, 0x8b, 0x41, 0xfc, 0x36,
        0xd4, 0x1e, 0xa3, 0x69, 0x5f, 0x95, 0x28, 0xe2,
        // 低128位
        0x00, 0xca, 0x77, 0xbd, 0x8b, 0x41, 0xfc, 0x36,
        0xd4, 0x1e, 0xa3, 0x69, 0x5f, 0x95, 0x28, 0xe2
    );
    return MulMatrix(x, higherMask, lowerMask);
}

__m256i aes_SBOX(__m256i x) {
    
    //逆行移位
    __m256i MASK = _mm256_setr_epi8(
        0x00, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x0e, 0x0b,
        0x08, 0x05, 0x02, 0x0f, 0x0c, 0x09, 0x06, 0x03,
        0x00, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x0e, 0x0b,
        0x08, 0x05, 0x02, 0x0f, 0x0c, 0x09, 0x06, 0x03
    );
    x = _mm256_shuffle_epi8(x, MASK);
    //将SM4上的向量转换到AES上
    //乘以矩阵TA
    x = MulMatrixTA(x);

    __m256i TC = _mm256_set1_epi8(0b00100011);

    //加上向量TC
    x = _mm256_xor_si256(x, TC);

    //过S盒，与行变换，这里与先前的逆行变换相抵消
	__m128i x0 = _mm256_extracti128_si256(x, 0);
    __m128i x1 = _mm256_extracti128_si256(x, 1);
    x0 = _mm_aesenclast_si128(x0, _mm_setzero_si128());
    x1 = _mm_aesenclast_si128(x1, _mm_setzero_si128());
    x  = _mm256_set_m128i(x1, x0);

    //将过AES的S盒后的向量重新转换到AES上
    __m256i output[1];
    //乘以矩阵ATA
    x = MulMatrixATA(x);
    //加上向量ATAC
    __m256i ATAC = _mm256_set1_epi8(0b00111011);
    x = _mm256_xor_si256(x, ATAC);

    _mm256_storeu_si256((__m256i*)output, x);
    /*
    uint8_t* p = (uint8_t*)&output;
    cout << endl;
    for (int j = 0; j < 32; j++) {
        printf("%02X ", p[j]);
    }
    */
    return x;
}

