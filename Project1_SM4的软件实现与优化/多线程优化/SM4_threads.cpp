#include"SM4.h"
#include<iostream>
#include<thread>
#include <windows.h>
#include <cstdlib> 
#include <ctime>
#include<vector>
using namespace std;
#define N 256
class TimeCounter {
public:
    TimeCounter(void) {
        QueryPerformanceFrequency(&CPUClock);
    }
    double timeInterval;
private:
    LARGE_INTEGER startTime, endTime, CPUClock;

public:
    void start() {
        QueryPerformanceCounter(&startTime);
    }
    void end() {
        QueryPerformanceCounter(&endTime);
        timeInterval = 1e3 * ((double)endTime.QuadPart - (double)startTime.QuadPart) / (double)CPUClock.QuadPart;
    }
};
void random_char_generator(unsigned char str[16]) {
    unsigned char r = 0, l = 255;
    srand(time(0));
    for (int i = 0; i < 16; i++) {
        str[i] = (rand() % (r - l + 1) + l);
    }
}

void init_set(unsigned char set[N][16]) {
    for (int i = 0; i < N; i += 4) {
        random_char_generator(set[i]);
        random_char_generator(set[i + 1]);
        random_char_generator(set[i + 2]);
        random_char_generator(set[i + 3]);
    }
}
void SM4_threads_encrypt(uint8_t key_set[N][16], uint8_t plain_set[N][16], uint8_t cipher_set[N][16], int index, Keys* round_keys) {
    int end = index + N / std::thread::hardware_concurrency();
    for (int i = index; i < end; i++) {
        SM4_encrypt(plain_set[i], cipher_set[i], round_keys);
    }
}

void SM4_threads_decrypt(uint8_t* input, uint8_t* dnc_result, Keys* round_keys);
int main() {
    // 只初始化一次随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));

    uint8_t key_set[N][16], plain_set[N][16], cipher_set[N][16];
    init_set(key_set);
    init_set(plain_set);
    // 初始化密钥和明文
    for (int i = 0; i < N; i++) {
        Keys sm4_roundkeys;
        SM4_Key_set(key_set[i], &sm4_roundkeys[i]);  // 为每个密钥生成轮密钥
    }

    // 获取CPU核心数
    int thread_num = std::thread::hardware_concurrency();
    std::vector<std::thread> pool(thread_num);

    // 加密计时
    auto start_enc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_num; i++) {
        pool[i] = std::thread(SM4_Enc_256, key_set, plain_set, cipher_set,
            i * (N / thread_num), round_keys);
    }
    for (auto& t : pool) t.join();
    auto end_enc = std::chrono::high_resolution_clock::now();

    // 输出加密耗时
    auto enc_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_enc - start_enc).count();
    std::cout << "Enc Time: " << enc_time << "ms" << std::endl;

    return 0;
}