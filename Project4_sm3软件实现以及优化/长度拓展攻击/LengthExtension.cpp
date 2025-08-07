#include"LengthExtension.h"

//MAC生成
void MAC(unsigned char* msg,size_t msg_size, unsigned char* key, size_t key_size ,unsigned char* output) {
	unsigned char* key_msg = new unsigned char[msg_size + key_size];
	memcpy(key_msg, key, key_size);
	memcpy(key_msg + key_size, msg, msg_size);
	sm3(key_msg, msg_size + key_size, output);//hash(key||msg)
}

//MAC验证
bool MAC_verfly(unsigned char* msg, size_t msg_size, unsigned char* key, size_t key_size, unsigned char* mac) {
	unsigned char* key_msg = new unsigned char[msg_size + key_size];
	unsigned char hash[32];
	memcpy(key_msg, key, key_size);
	memcpy(key_msg + key_size, msg, msg_size);
	sm3(key_msg, msg_size + key_size, hash);
	if (memcmp(hash, mac, 32) == 0) {
		return true;
	}
	else {
		return false;
	}
}

//随机消息生成函数
void msg_get(unsigned char* output,size_t size) {
	const char alphanumeric_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<size_t> distribution(0, sizeof(alphanumeric_chars) - 2);

	for (int i = 0; i < size; i++) {
		output[i] = alphanumeric_chars[distribution(generator)];
	}
}

// SM3 填充函数
unsigned char* sm3_pad(size_t msg_size, size_t* pad_len) {
	const size_t block_size = 64;  // SM3 块大小
	size_t required_pad_len;

	required_pad_len = block_size - (msg_size % block_size);
	if (required_pad_len < 9) {//至少需要9字节的填充
		required_pad_len += block_size;
	}

	*pad_len = required_pad_len;
	unsigned char* padding = (unsigned char*)malloc(*pad_len);
	if (!padding) return NULL;

	padding[0] = 0x80;//固定的1位
	memset(padding + 1, 0x00, required_pad_len - 9); //填充0位

	uint64_t bit_len = (uint64_t)msg_size * 8;
	for (int i = 0; i < 8; i++) {
		padding[required_pad_len - 8 + i] = (bit_len >> (56 - i * 8)) & 0xFF;
	}//消息长度信息

	return padding;
}
void MAC_test() {
	unsigned char key[32];
	unsigned char msg[64];
	unsigned char mac[64];
	msg_get(key, 32);
	msg_get(msg, 64);
	MAC(msg, 64, key, 32, mac);
	cout << "key: ";
	for (int i = 0; i < 32; i++) {
		cout << hex << setw(2) << setfill('0') << (int)key[i];
	}
	cout << endl;
	cout << "msg: ";
	for (int i = 0; i < 64; i++) {
		cout << hex << setw(2) << setfill('0') << (int)msg[i];
	}
	cout << endl;
	cout << "mac: ";
	for (int i = 0; i < 32; i++) {
		cout << hex << setw(2) << setfill('0') << (int)mac[i];
	}
	cout << endl;
	if (MAC_verfly(msg, 64, key, 32, mac)) {
		cout << "MAC verification passed." << endl;
	}
	else {
		cout << "MAC verification failed." << endl;
	}
}
//长度拓展攻击
unsigned char* MAC_forge(unsigned char* mac ,unsigned char* msg,size_t msg_size,size_t key_size,unsigned char* mac_forge,int* len) {
	//iv替换
	sm3_ctx* ctx = new sm3_ctx;
	sm3_init(ctx);
	for (int i = 0; i < 8; i++) {
		ctx->state[i] = ((uint32_t)mac[4 * i] << 24) | 
			((uint32_t)mac[4 * i + 1] << 16) |
			((uint32_t)mac[4 * i + 2] << 8) |
			((uint32_t)mac[4 * i + 3]); 
	}

	unsigned char msg_forge_tail[32];
	unsigned char* msg_padding;
	size_t padded_len;
	msg_get(msg_forge_tail, 32);//产生附加消息
	msg_padding = sm3_pad(msg_size + key_size, &padded_len);//计算key||msg的填充

	//msg||padding||S
	unsigned char* msg_forge_result = new unsigned char[msg_size + padded_len + 32];
	memcpy(msg_forge_result, msg, msg_size);
	memcpy(msg_forge_result + msg_size, msg_padding, padded_len);
	memcpy(msg_forge_result + msg_size + padded_len, msg_forge_tail, 32);
	*len = msg_size + padded_len + 32;

	//计算key||msg||padding||S的填充
	unsigned char* msg_forge_padding;
	size_t padded_len_;
	msg_forge_padding = sm3_pad(key_size + msg_size + padded_len + 32, &padded_len_);

	//计算Hash(S||padded_)得到伪造的MAC
	unsigned char* msg_for_mac_forge = new unsigned char[32 + padded_len_];
	memcpy(msg_for_mac_forge, msg_forge_tail, 32);
	memcpy(msg_for_mac_forge + 32, msg_forge_padding, padded_len_);

	for (int i = 0; i < 64; i++) {
		ctx->buf[i] = msg_for_mac_forge[i];
	}

	sm3_compress(ctx);
	GM_PUT_UINT32_BE(ctx->state[0], mac_forge, 0);
	GM_PUT_UINT32_BE(ctx->state[1], mac_forge, 4);
	GM_PUT_UINT32_BE(ctx->state[2], mac_forge, 8);
	GM_PUT_UINT32_BE(ctx->state[3], mac_forge, 12);
	GM_PUT_UINT32_BE(ctx->state[4], mac_forge, 16);
	GM_PUT_UINT32_BE(ctx->state[5], mac_forge, 20);
	GM_PUT_UINT32_BE(ctx->state[6], mac_forge, 24);
	GM_PUT_UINT32_BE(ctx->state[7], mac_forge, 28);

	return msg_forge_result;
}
void MAC_forge_test() {
	unsigned char msg[64];
	msg_get(msg, 64);
	unsigned char mac[32];
	unsigned char key[32];
	unsigned char* msg_forge;
	unsigned char mac_forge[32];
	int msg_forge_len = 0;
	msg_get(key, 32);
	sm3(msg, 64, mac);
	MAC(msg, 64, key, 32, mac);
	//MAC伪造
	msg_forge = MAC_forge(mac, msg, 64, 32, mac_forge,&msg_forge_len);
	cout << "MAC_forge_test result:"<<MAC_verfly(msg_forge, msg_forge_len, key, 32, mac_forge) << endl;
}