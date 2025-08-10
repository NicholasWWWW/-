from gmssl import sm3, func
from Crypto.Util.number import *

# SM2椭圆曲线参数(F_p-256上的椭圆曲线方程为：y^2=x^3+ax+b)
p=int('FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF',base=16) #素数p
a=int('FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC',base=16) #系数a
b=int('28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93',base=16) #系数b
n=int('FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123',base=16) #阶n
Gx=int('32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7',base=16) #基点G坐标xG
Gy=int('BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0',base=16) #基点G坐标yG


def ec_add_point(x1, y1, x2, y2): 
    """椭圆曲线点加法函数 对域元素进行运算"""
    if x1 != x2:
        lamda = ((y2 - y1) * inverse((x2 - x1), p)) % p  
        x3 = ((lamda ** 2) % p - x1 - x2) % p
        y3 = ((lamda * (x1 - x3)) % p - y1) % p
    else:
        lamda = ((3 * (x1 ** 2) % p + a) * inverse(2 * y1, p)) % p
        x3 = ((lamda ** 2) % p - 2 * x1) % p
        y3 = (lamda * (x1 - x3) % p - y1) % p
    return x3, y3

def ec_multiply_point(k, x1, y1): 
    """椭圆曲线倍点运算函数 递归转化为加法运算"""
    if k == 1:
        return x1, y1
    elif k == 2:
        return ec_add_point(x1, y1, x1, y1)
    elif k % 2 == 0:
        x, y = ec_multiply_point(k // 2, x1, y1)
        x, y = ec_multiply_point(2, x, y)
        return x, y
    elif k % 2 == 1:
        x, y = ec_multiply_point((k - 1) // 2, x1, y1)
        x, y = ec_multiply_point(2, x, y)
        x, y = ec_add_point(x, y, x1, y1)
        return x, y


def KDF(Z: str, klen: int):
    """密钥派生函数"""
    v = 256          # SM3 输出长度
    v_bytes = 32 
    ct = 0x00000001  # 计数器
    H = []           # 哈希值列表
    K = ''           # 最终密钥比特串
    klen_bytes= ceil_div(klen, 8)
    interation = ceil_div(klen_bytes, v_bytes)

    for _ in range(interation):
        # 将输入数据转为字节列表
        data = Z + bin(ct)[2:].zfill(32)
        byte_list = list(data.encode('utf-8'))
        # 调用 gmssl 的 SM3
        m = sm3.sm3_hash(byte_list)
        # 将哈希结果（十六进制字符串）转为二进制并补齐256位
        H.append(bin(int(m, 16))[2:].zfill(v))
        ct += 1
    # 每块密钥进行拼接，最后一块如果不足256位，用左边比特补齐
    if klen % v == 0:
        for i in range(0, interation):
            K += H[i]
    else:
        for i in range(0, interation - 1):
            K += H[i]
        K += H[interation - 1][:klen % v]
    # 验证密钥生成长度满足预期后再return
    if len(K) == klen:
        return int(K, 2)
    else:
        print("KDF error!")

def key():
    """密钥生成函数"""
    dB = getRandomNBitInteger(256)
    xb, yb = ec_multiply_point(dB, Gx, Gy)
    return dB, xb, yb
def compute_user_hash(user_id, pub_x, pub_y):
    """计算用户标识哈希ZA (SM3)"""
    id_bitlen = len(user_id.encode('utf-8')) * 8
    id_bitlen_bytes = id_bitlen.to_bytes(2, 'big')
    components = [
        id_bitlen_bytes,
        user_id.encode('utf-8'),
        a.to_bytes(32, 'big'),
        b.to_bytes(32, 'big'),
        Gx.to_bytes(32, 'big'),
        Gy.to_bytes(32, 'big'),
        pub_x.to_bytes(32, 'big'),
        pub_y.to_bytes(32, 'big')
    ]
    joint_bytes = b''.join(components)
    hash_bytes = func.bytes_to_list(joint_bytes)
    result = sm3.sm3_hash(hash_bytes)
    return result

def sm2_sign(M: bytes, user_id, d_A: int, xb: int, yb: int):
    """
    SM2签名算法
    Args:
        M (bytes): 待签名的原始消息
        d_A (int): 用户A的私钥
        xb(int):公钥的 x 坐标。
        yb(int):公钥的 y 坐标。
    Returns:.
        r_str (str): 签名r值的字符串表示
        s_str (str): 签名s值的字符串表示
    """
    #拼接消息 M = ZA || M
    ZA = compute_user_hash(user_id, xb, yb)
    M_prime = ZA.encode('utf-8') + M
    e_bytes = sm3.sm3_hash(list(M_prime))
    e = int(e_bytes, 16)   
     
    k = getRandomRange(1, n - 1)  # 生成随机数 k ∈ [1, n-1]
    x1, y1 = ec_multiply_point(k, Gx, Gy) # 计算椭圆曲线点 (x1, y1) = [k]G
    r = (e + x1) % n            # 计算 r = (e + x1) mod n 
    # 计算 s = ((1 + d_A)⁻¹ · (k - r·d_A)) mod n
    s_value = inverse(1 + d_A, n)
    s = s_value* (k - r * d_A) % n 
    return (r, s)

def sm2_verify_signature(M_prime: bytes, user_id, signature,  xb: int, yb: int) :
    """
    SM2签名验证算法
    Args:
        M_prime (bytes): 接收到的消息
        signature (Tuple[str, str]): 数字签名 (r', s')
        ZA (bytes): 用户A的标识哈希值
        xb(int):公钥的 x 坐标。
        yb(int):公钥的 y 坐标。
    Returns:
        bool: 验证结果 
    """
    r_, s_ = signature
    # 检验 r' ∈ [1, n-1], s' ∈ [1, n-1]
    if not (0 < r_ < n and 0 < s_ < n):
        return False    
    # M = ZA || M' 
    ZA = compute_user_hash(user_id, xb, yb)   
    M_ = ZA.encode('utf-8') + M_prime
    #计算 e' = H_v(M) 并转换为整数
    e_bytes = sm3.sm3_hash(list(M_))
    e_ = int(e_bytes, 16)   
    # 计算 t = (r' + s') mod n
    t = (r_+ s_) % n
    # 计算椭圆曲线点 (x1', y1') = [s']G + [t]P_A
    s_Gx, s_Gy = ec_multiply_point(s_ % n, Gx, Gy)
    # 计算 [t]P_A
    tPAx, tPAy = ec_multiply_point(t % n, xb, yb)
    # 点相加: [s']G + [t]P_A
    x1_prime, y1_prime = ec_add_point(s_Gx, s_Gy, tPAx, tPAy)
    #计算 R = (e' + x1') mod n
    R = (e_ + x1_prime) % n
    # 检验 R = r' 是否成立 
    return R == r_

def sm2_encrypt(M: bytes, xb, yb):
    """
    SM2加密算法
    Args:
        m(bytes):待加密的明文。
        xb(int):公钥的 x 坐标。
        yb(int):公钥的 y 坐标。
    Returns:
        - C1_x (int):  C1 的 x 坐标。
        - C1_y (int):  C1 的 y 坐标。
        - C2 (int): 明文与密钥派生函数结果的异或值。
        - C3 (int): SM3 哈希函数生成的摘要值。
        - C (str): 完整的密文字符串。
    """
    if (xb, yb) == (0, 0):
        raise ValueError("公钥无效 (无穷远点)")
    M = int(M.hex(), 16)
    k = getRandomRange(1, n)                    #随机数发生器产生随机数k
    #计算椭圆曲线点C1
    C1_x, C1_y = ec_multiply_point(k, Gx, Gy)   
    PC = '0x04' 
    C1 = int(PC[2:] + hex(C1_x)[2:] + hex(C1_y)[2:], 16)  #将C1的数据类型转换为比特串
    x2, y2 = ec_multiply_point(k, xb, yb)
    x2_y2 = bin(x2)[2:] + bin(y2)[2:]
    t = KDF(x2_y2, size(int(x2_y2,2)))
    #计算C2 =M⊕t
    C2 = M ^ t       
    # 计算哈希 C3 = SM3(x2 || m || y2)
    data = bin(x2)[2:] + bin(M)[2:] + bin(y2)[2:]
    byte_list = list(data.encode('utf-8'))
    C3 = int(sm3.sm3_hash(byte_list), 16)
    C = hex(C1)[2:] + hex(C2)[2:] + hex(C3)[2:]
    return C1_x, C1_y, C2, C3, C


def sm2_decrypt(C1_x, C1_y, C2, C3, dB):
    """
    SM2解密算法
    Args:
        C1_x (int): 密文分量 C1 的 x 坐标（椭圆曲线点）。
        C1_y (int): 密文分量 C1 的 y 坐标（椭圆曲线点）。
        C2 (int): 密文分量（明文与密钥派生结果的异或值）。
        C3 (int): 密文分量（SM3 哈希摘要）。
        dB (int): 接收方的私钥。
    Returns:
        Union[bytes, bool]: 
            - 成功时返回解密后的明文（字节串）。
            - 失败时返回 False（若 C1 或 C3 校验失败）。
    """   
    if (C1_x ** 3 + (a * C1_x) + b) % p != (C1_y ** 2) % p:
        print('C1 check error!')
        return False
    else:
        x2, y2 = ec_multiply_point(dB, C1_x, C1_y)
        x2_y2 = bin(x2)[2:] + bin(y2)[2:]
        t = KDF(x2_y2, size(int(x2_y2,2)))
        m = C2 ^ t
        # 杂凑函数用sm3
        data = bin(x2)[2:] + bin(m)[2:] + bin(y2)[2:]
        byte_list = list(data.encode('utf-8'))    
        u = int(sm3.sm3_hash(byte_list), 16)

        if u != C3:
            print('C3 check error!')
            return False
        else:
            M = bytes.fromhex(hex(m)[2:])  # 截取得到 hexstr
            return M
def test_sm2_operations():
    """SM2综合测试：签名验证与加解密"""
    # 测试数据准备
    test_msg = b"Hello, SM2!"  # 固定测试消息
    user_id = "test_user"      # 用户标识
    # 1. 密钥生成
    private_key, pub_x, pub_y = key()
    print(f"\n【密钥生成】\n私钥: 0x{private_key:064x}\n公钥: (0x{pub_x:064x}, 0x{pub_y:064x})")
    # 2. 签名验证测试
    print("\n【签名验证测试】")
    signature = sm2_sign(test_msg, user_id, private_key, pub_x, pub_y)
    print(f"消息: {test_msg.decode()}\n签名: (r=0x{signature[0]:064x}, s=0x{signature[1]:064x})")
    # 验证原始签名（应成功）
    is_valid = sm2_verify_signature(test_msg, user_id, signature, pub_x, pub_y)
    print(f"验证结果: {'通过 ' if is_valid else '失败'} (预期: 通过)")
    # 3. 加解密测试
    print("\n【加解密测试】")
    plaintext = input("输入待加密文本: ").encode('utf-8')
    # 加密
    C1_x, C1_y, C2, C3, ciphertext = sm2_encrypt(plaintext, pub_x, pub_y)
    print(f"加密结果: 0x{ciphertext[:64]}...{ciphertext[-64:]}")  
    # 解密
    decrypted = sm2_decrypt(*sm2_encrypt(plaintext, pub_x, pub_y)[:4], private_key)
    print(f"解密结果: {decrypted.decode('utf-8')}")
if __name__ == "__main__":
    test_sm2_operations()