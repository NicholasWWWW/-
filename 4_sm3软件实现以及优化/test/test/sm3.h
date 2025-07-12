#pragma once
//
// Created by saint on 2021/12/9.
//

#ifndef YISECUREBOXCPP_SM3_H
#define YISECUREBOXCPP_SM3_H

#include <stdint.h>
#include<iostream>
using namespace std;
# ifdef  __cplusplus
extern "C" {
# endif

    typedef struct {
        unsigned int state[8]; // �Ĵ����м�״̬
        unsigned char buf[64]; // ��ѹ����Ϣ
        uint64_t cur_buf_len; // ��ǰ��ѹ����Ϣ���ȣ��ֽڣ�
        uint64_t compressed_len; // ��ѹ����Ϣ���ȣ����أ�
    } gm_sm3_context;

    /**
     * ժҪ�㷨��ʼ��
     * @param ctx ������
     */
    void gm_sm3_init(gm_sm3_context* ctx);

    /**
     * �����Ϣ
     * @param ctx ������
     * @param input ��Ϣ
     * @param iLen ��Ϣ���ȣ��ֽڣ�
     */
    void gm_sm3_update(gm_sm3_context* ctx, const unsigned char* input, unsigned int iLen);

    /**
     * ����ժҪ
     * @param ctx ������
     * @param output ���ժҪ���
     */
    void gm_sm3_done(gm_sm3_context* ctx, unsigned char output[32]);

    /**
     * ���������ģ��Դﵽ���ô������ĵ�Ŀ��
     * @param ctx ������
     */
    inline void gm_sm3_reset(gm_sm3_context* ctx) {
        gm_sm3_init(ctx);
    }

    /**
     * ֱ�Ӽ�����Ϣ��ժҪ
     * @param input ��Ϣ
     * @param iLen ��Ϣ���ȣ��ֽڣ�
     * @param output ���ժҪ���
     */
    void gm_sm3(const unsigned char* input, unsigned int iLen, unsigned char output[32]);

# ifdef  __cplusplus
}
# endif

#endif //YISECUREBOXCPP_SM3_H