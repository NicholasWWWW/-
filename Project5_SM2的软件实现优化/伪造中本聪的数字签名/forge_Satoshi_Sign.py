from Crypto.Util.number import *
from ecpy.curves import Curve, Point
import hashlib

#采用 secp256k1 椭圆曲线
curve = Curve.get_curve('secp256k1')
n = curve.order  # 曲线阶
G = curve.generator  # 基点
k = getRandomRange(1, n - 1)  # 生成随机数 k ∈ [1, n-1]

# 中本聪的公钥
satoshi_pubkey = Point(
    0x678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb6,
    0x49f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f,
    curve
)

def ECDSA_sign(m: str,d: int):
    """
    ECDSA 签名算法
    :param d: 私钥 (整数)
    :param m: 待签名的消息 (字符串)
    :return: 签名 (r, s)
    """
    e_bytes = hashlib.sha256(m.encode()).digest()   # 计算消息的哈希 e = hash(m)
    e = bytes_to_long(e_bytes)
    k = getRandomRange(1, n-1)  # 随机选取 k ∈ [1, n-1]
    R = k * G                   # 计算 R = kG
    r = R.x % n                 # 计算 r = R.x mod n
    k_inv = inverse(k, n)       # 计算 s = k⁻¹ (e + dr) mod n
    s = (k_inv * (e + d * r)) % n  
    signature = (r, s)
    return signature

def ECDSA_verify_signature(e , P, signature):
    """
    验证ECDSA签名
    Args:
        e : 消息哈希
        P : 公钥
        signature(r,s):消息签名
    Returns:.
        bool : 验证结果
    """
    r, s = signature

    if not (1 <= r < n and 1 <= s < n):
        return False
    w = inverse(s, n)    #w=s^{−1} modn
    u1 = (e * w) % n
    u2 = (r * w) % n
    P_ = u1 * G + u2 * P
    return P_.x % n == r

def forge_Satoshi_Sign():
    """
    伪造中本聪的数字签名
    Returns:.
        signature(r,s):伪造消息签名
    """
    a = getRandomRange(1, n - 1) #随机选取a
    b = getRandomRange(1, n - 1) #随机选取b
    R_prime = a * G + b * satoshi_pubkey
    r_prime = R_prime.x % n
    b_inv = inverse(b, n)  
    s_prime = (r_prime * b_inv) % n

    e_prime = (a * s_prime) % n
    # 构造伪造的签名
    forged_signature = (r_prime, s_prime)
    forged_message_hash = e_prime
    return forged_signature, forged_message_hash


if __name__ == "__main__":
    """主函数：伪造并验证签名"""
    # 伪造中本聪的签名
    forged_signature, forged_message_hash = forge_Satoshi_Sign()
    print("伪造的签名 (r, s):")
    print(f"r = {hex(forged_signature[0])}")
    print(f"s = {hex(forged_signature[1])}")
    print(f"伪造的消息哈希: {hex(forged_message_hash)}")
    # 验证伪造的签名
    is_valid = ECDSA_verify_signature(forged_message_hash, satoshi_pubkey, forged_signature)
    print(f"ECDSA签名验证结果: {'成功' if is_valid else '失败'}")
