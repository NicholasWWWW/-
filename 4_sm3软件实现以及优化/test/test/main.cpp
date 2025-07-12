#include "sm3.h"
#include <iostream>
void sm3_test() {
	unsigned char input[] = "abc";
	unsigned char output[32];
	gm_sm3(input, 3, output);
	std::cout << "SM3 Hash: ";
	for (int i = 0; i < 32; i++) {
		printf("%02x", output[i]);
	}
}
int main() {
	sm3_test();
	return 0;
}