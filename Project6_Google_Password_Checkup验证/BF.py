import math

class HashFuncBKDR:
    def __call__(self, s: str) -> int:
        hash_val = 0
        for ch in s:
            hash_val = hash_val * 31 + ord(ch)
        return hash_val

class HashFuncAP:
    def __call__(self, s: str) -> int:
        hash_val = 0
        for i in range(len(s)):
            if (i & 1) == 0:
                hash_val ^= ((hash_val << 7) ^ ord(s[i]) ^ (hash_val >> 3))
            else:
                hash_val ^= (~((hash_val << 11) ^ ord(s[i]) ^ (hash_val >> 5)))
        return hash_val

class HashFuncDJB:
    def __call__(self, s: str) -> int:
        hash_val = 5381
        for ch in s:
            hash_val = (hash_val * 33) ^ ord(ch)
        return hash_val

class BloomFilter:
    def __init__(self, N, X = 6):
        self.M = N * X  # 位数组大小（总比特数）
        self.bit_array = bytearray(math.ceil(self.M / 8))  # 用 bytearray 存储位信息
        self.hash_funcs = [HashFuncBKDR(), HashFuncAP(), HashFuncDJB()]  # 三种哈希函数

    def _set_bit(self, index: int) -> None:
        byte_pos = index // 8
        bit_pos = index % 8
        self.bit_array[byte_pos] |= (1 << bit_pos)

    def _get_bit(self, index: int) -> bool:
        byte_pos = index // 8
        bit_pos = index % 8
        return (self.bit_array[byte_pos] & (1 << bit_pos)) != 0

    def add(self, key: str) -> None:
        for hash_func in self.hash_funcs:
            index = hash_func(key) % self.M
            self._set_bit(index)

    def contains(self, key: str) -> bool:
        for hash_func in self.hash_funcs:
            index = hash_func(key) % self.M
            if not self._get_bit(index):
                return False
        return True

# BF测试
if __name__ == "__main__":
    bloom = BloomFilter(N=1000) 
    bloom.add("hello")
    bloom.add("world")

    print(bloom.contains("hello")) 
    print(bloom.contains("world")) 
    print(bloom.contains("python")) 