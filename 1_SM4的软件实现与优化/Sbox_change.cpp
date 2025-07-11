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
__m256i MulMatrix(__m256i x, __m256i higherMask, __m256i lowerMask) {
    __m256i tmp1, tmp2;
    __m256i andMask = _mm256_set1_epi32(0x0f0f0f0f); // ��4λ����

    // �����4λ�͵�4λ
    tmp2 = _mm256_srli_epi16(x, 4);       // ����4λ����ȡ��4λ
    tmp1 = _mm256_and_si256(x, andMask);  // �����������ȡ��4λ
    tmp2 = _mm256_and_si256(tmp2, andMask);
    
    tmp1 = _mm256_shuffle_epi8(lowerMask, tmp1); // ��4λ���
    
    tmp2 = _mm256_shuffle_epi8(higherMask, tmp2); // ��4λ���
    
    // �ϲ�
    tmp1 = _mm256_xor_si256(tmp1, tmp2);
    mm_print(tmp1);

    return tmp1;
}
__m256i MulMatrixATA(__m256i x) {
    // ��4λ�������
    __m256i higherMask = _mm256_setr_epi8(
        // ��128λ
        0x00, 0x13, 0xd2, 0xc1, 0x78, 0x6b, 0xaa, 0xb9,
        0xad, 0xbe, 0x7f, 0x6c, 0xd5, 0xc6, 0x07, 0x14,
        // ��128λ
        0x00, 0x13, 0xd2, 0xc1, 0x78, 0x6b, 0xaa, 0xb9,
        0xad, 0xbe, 0x7f, 0x6c, 0xd5, 0xc6, 0x07, 0x14
    );

    // ��4λ�������
    __m256i lowerMask = _mm256_setr_epi8(
        // ��128λ
        0x00, 0x60, 0x22, 0x42, 0x1d, 0x7d, 0x3f, 0x5f,
        0x87, 0xe7, 0xa5, 0xc5, 0x9a, 0xfa, 0xb8, 0xd8,
        // ��128λ
        0x00, 0x60, 0x22, 0x42, 0x1d, 0x7d, 0x3f, 0x5f,
        0x87, 0xe7, 0xa5, 0xc5, 0x9a, 0xfa, 0xb8, 0xd8
    );

    return MulMatrix(x, higherMask, lowerMask);
}
__m256i MulMatrixTA(__m256i x) {
    __m256i higherMask = _mm256_setr_epi8(
        //��128λ
        0x22, 0x58, 0x1a, 0x60, 0x02, 0x78, 0x3a, 0x40,
        0x62, 0x18, 0x5a, 0x20, 0x42, 0x38, 0x7a, 0x00,
        //��128λ
        0x22, 0x58, 0x1a, 0x60, 0x02, 0x78, 0x3a, 0x40,
        0x62, 0x18, 0x5a, 0x20, 0x42, 0x38, 0x7a, 0x00
    );

    __m256i lowerMask = _mm256_setr_epi8(
        //��128λ
        0xe2, 0x28, 0x95, 0x5f, 0x69, 0xa3, 0x1e, 0xd4,
        0x36, 0xfc, 0x41, 0x8b, 0xbd, 0x77, 0xca, 0x00,
        //��128λ
        0xe2, 0x28, 0x95, 0x5f, 0x69, 0xa3, 0x1e, 0xd4,
        0x36, 0xfc, 0x41, 0x8b, 0xbd, 0x77, 0xca, 0x00
    );
    return MulMatrix(x, higherMask, lowerMask);
}

__m256i aes_SBOX(__m256i x) {
    
    //������λ
    __m256i MASK = _mm256_setr_epi8(
        0x03, 0x06, 0x09, 0x0c, 0x0f, 0x02, 0x05, 0x08,
        0x0b, 0x0e, 0x01, 0x04, 0x07, 0x0a, 0x0d, 0x00,
        0x03, 0x06, 0x09, 0x0c, 0x0f, 0x02, 0x05, 0x08,
        0x0b, 0x0e, 0x01, 0x04, 0x07, 0x0a, 0x0d, 0x00
    );
    x = _mm256_shuffle_epi8(x, MASK);
    //��SM4�ϵ�����ת����AES��

    //���Ծ���TA
    x = MulMatrixTA(x);
    //�������
    __m256i index_re = _mm256_setr_epi8(
        7, 6, 5, 4, 3, 2, 1, 0,
        15, 14, 13, 12, 11, 10, 9, 8,
        23, 22, 21, 20, 19, 18, 17, 16,
        31, 30, 29, 28, 27, 26, 25, 24
    );
    x = _mm256_shuffle_epi8(x, index_re);
    __m256i TC = _mm256_set1_epi8(0b00100011);

    //��������TC
    x = _mm256_xor_si256(x, TC);

    //��S�У����б任����������ǰ�����б任�����
    x = _mm256_aesenclast_epi128(x, _mm256_setzero_si256());

    //����AES��S�к����������ת����AES��
    __m256i output[1];
    //���Ծ���ATA
    x = MulMatrixATA(x);

    //��������ATAC
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
