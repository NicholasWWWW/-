#include"SM4.h"
#include"SM4_aes.h"

void SM4_aes_encrypt(uint8_t* input, uint8_t* enc_result, Keys* round_keys) {
    //����avxָ�
    __m256i x[4], tmp[4],output[4];
    //��������
    tmp[0] = _mm256_loadu_si256((const __m256i*)input + 0);
    tmp[1] = _mm256_loadu_si256((const __m256i*)input + 1);
    tmp[2] = _mm256_loadu_si256((const __m256i*)input + 2);
    tmp[3] = _mm256_loadu_si256((const __m256i*)input + 3);
    //���´��
    x[0] = _mm256_unpacklo_epi64(_mm256_unpacklo_epi32(tmp[0], tmp[1]), _mm256_unpacklo_epi32(tmp[2], tmp[3]));
    x[1] = _mm256_unpackhi_epi64(_mm256_unpacklo_epi32(tmp[0], tmp[1]), _mm256_unpacklo_epi32(tmp[2], tmp[3]));
    x[2] = _mm256_unpacklo_epi64(_mm256_unpackhi_epi32(tmp[0], tmp[1]), _mm256_unpackhi_epi32(tmp[2], tmp[3]));
    x[3] = _mm256_unpackhi_epi64(_mm256_unpackhi_epi32(tmp[0], tmp[1]), _mm256_unpackhi_epi32(tmp[2], tmp[3]));
    //��Ϊ�����
    __m256i vindex =_mm256_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12,
            3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);
    x[0] = _mm256_shuffle_epi8(x[0], vindex);
    x[1] = _mm256_shuffle_epi8(x[1], vindex);
    x[2] = _mm256_shuffle_epi8(x[2], vindex);
    x[3] = _mm256_shuffle_epi8(x[3], vindex);
    //����������
    /*
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
    */
    //32�ּ���
    for (int i = 0; i < 32; i++) {
        //�������Կ
        tmp[0] = _mm256_xor_si256(_mm256_xor_si256(x[1], x[2]),
            _mm256_xor_si256(x[3], _mm256_set1_epi32(round_keys->rk[i])));
        //��S��
        tmp[0] = aes_SBOX(tmp[0]);
        //L
        tmp[1] = _mm256_xor_si256(_mm256_slli_epi32(tmp[0],  2), _mm256_slli_epi32(tmp[0], 30));
        tmp[2] = _mm256_xor_si256(_mm256_slli_epi32(tmp[0], 10), _mm256_slli_epi32(tmp[0], 22));
        tmp[3] = _mm256_xor_si256(_mm256_slli_epi32(tmp[0], 18), _mm256_slli_epi32(tmp[0], 14));
        tmp[1] = _mm256_xor_si256(tmp[1], tmp[2]);
        tmp[1] = _mm256_xor_si256(tmp[1], tmp[3]);
        tmp[3] = _mm256_xor_si256(_mm256_slli_epi32(tmp[0], 24), _mm256_slli_epi32(tmp[0], 8));
        tmp[1] = _mm256_xor_si256(tmp[1], tmp[3]);
        tmp[0] = _mm256_xor_si256(x[0], tmp[0]);
        tmp[0] = _mm256_xor_si256(tmp[1], tmp[0]);
        //����
        x[0] = x[1];
        x[1] = x[2];
        x[2] = x[3];
        x[3] = tmp[0];
    }
    //��Ϊ�����
    tmp[0] = _mm256_shuffle_epi8(x[0], vindex);
    tmp[1] = _mm256_shuffle_epi8(x[1], vindex);
    tmp[2] = _mm256_shuffle_epi8(x[2], vindex);
    tmp[3] = _mm256_shuffle_epi8(x[3], vindex);
    //���´����ע����Ҫ����
    x[0] = _mm256_unpacklo_epi64(_mm256_unpacklo_epi32(tmp[3], tmp[2]), _mm256_unpacklo_epi32(tmp[1], tmp[0]));
    x[1] = _mm256_unpackhi_epi64(_mm256_unpacklo_epi32(tmp[3], tmp[2]), _mm256_unpacklo_epi32(tmp[1], tmp[0]));
    x[2] = _mm256_unpacklo_epi64(_mm256_unpackhi_epi32(tmp[3], tmp[2]), _mm256_unpackhi_epi32(tmp[1], tmp[0]));
    x[3] = _mm256_unpackhi_epi64(_mm256_unpackhi_epi32(tmp[3], tmp[2]), _mm256_unpackhi_epi32(tmp[1], tmp[0]));
    //�ָ����鲢װ��
    _mm256_storeu_si256((__m256i*)enc_result + 0,x[0]);
    _mm256_storeu_si256((__m256i*)enc_result + 1,x[1]);
    _mm256_storeu_si256((__m256i*)enc_result + 2,x[2]);
    _mm256_storeu_si256((__m256i*)enc_result + 3,x[3]);
}

void SM4_aes_decrypt(uint8_t* input, uint8_t* dnc_result, Keys* round_keys) {

}