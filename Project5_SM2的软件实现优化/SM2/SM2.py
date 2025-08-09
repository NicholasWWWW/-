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

if __name__ == '__main__':
    # 明文处理
    m = bytes(input(), encoding='UTF8')
    print('明文:', m)
    klen = size(int(m.hex(), 16))
    # 公私钥生成
    dB, xb, yb = key()
    print('私钥:', dB)
    print('公钥:', (xb, yb))
    # 调用SM2进行加解密
    C1_x, C1_y, C2, C3, C = sm2_encrypt(m, xb, yb)
    print('加密密文:', C)
    print('解密结果:', sm2_decrypt(C1_x, C1_y, C2, C3, dB).decode('utf-8'))
