import SM2
from Crypto.Util.number import *

# SM2椭圆曲线参数(F_p-256上的椭圆曲线方程为：y^2=x^3+ax+b)
p=int('FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF',base=16) #素数p
a=int('FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC',base=16) #系数a
b=int('28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93',base=16) #系数b
n=int('FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123',base=16) #阶n
Gx=int('32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7',base=16) #基点G坐标xG
Gy=int('BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0',base=16) #基点G坐标yG

def POC_SM2_sig_leaking_k():
    """POC验证：随机数k泄露导致私钥泄露"""
    print("\n===== POC验证：随机数k泄露导致私钥泄露 =====")
    private_key, pub_x, pub_y = SM2.key()
    # 生成签名并记录使用的k值
    test_msg = b"Hello, SM2!"  # 固定测试消息
    k = SM2.getRandomRange(1, n - 1)  # 生成随机数 k ∈ [1, n-1]
    user_id = "test_user"      # 用户标识
    signature = SM2.sm2_sign(test_msg, user_id, private_key, pub_x, pub_y,k)
    r, s = signature
    print(f"使用的随机数k: {hex(k)}")
    print(f"【密钥生成】\n私钥: 0x{private_key:064x}")
    # 从泄露的k恢复私钥: dA = (k - s) * (s + r)^(-1) mod n
    denominator = (s + r) % SM2.n

    inv_denom = inverse(denominator, n)
    recovered_private = ((k - s) * inv_denom) % n

    print(f"恢复的私钥: {hex(recovered_private)}")
    print(f"恢复结果: {'成功 ' if private_key == recovered_private else '失败'}")

def POC_SM2_sig_sameuser_reusing_k():
    """POC验证：随机数k重复使用导致私钥d泄露"""
    print("\n===== POC验证：随机数k重复使用导致私钥d泄露 =====")
    private_key, pub_x, pub_y = SM2.key()
    # 生成签名并记录使用的k值
    msg1 = b"Hello, SM2A!"
    msg2 = b"Hello, SM2B!"
    k = SM2.getRandomRange(1, n - 1)  # 生成随机数 k ∈ [1, n-1]
    user_id = "test_user"      # 用户标识
    print(f"使用的随机数k: {hex(k)}")
    print(f"【密钥生成】\n私钥: 0x{private_key:064x}")
    signature1 = SM2.sm2_sign(msg1, user_id, private_key, pub_x, pub_y,k)
    signature2 = SM2.sm2_sign(msg2, user_id, private_key, pub_x, pub_y,k)
    
    r1, s1 = signature1
    r2, s2 = signature2
     # 推导公式：dA = (s2 - s1) / (s1 - s2 + r1 - r2) mod n
    numerator = (s2 - s1) % n
    denominator = (s1 - s2 + r1 - r2) % n
    inv_denom = inverse(denominator, n)
    recovered_private = numerator * inv_denom % n
    print(f"恢复的私钥: {hex(recovered_private)}")
    print(f"恢复结果: {'成功 ' if private_key == recovered_private else '失败'}")

def POC_SM2_sig_diff_user_reusing_k():
    """POC验证：两用户使用相同k导致私钥d泄露"""
    print("\n===== POC验证：两用户使用相同k导致私钥d泄露 =====")
    private_keyA, pubA_x, pubA_y = SM2.key()
    private_keyB, pubB_x, pubB_y = SM2.key()
    user_id = "userA"      # 用户A标识
    user_id = "userB"      # 用户A标识
    k = SM2.getRandomRange(1, n - 1)  # 生成随机数 k ∈ [1, n-1]
    msgA = b"Hello, SM2A!"
    msgB = b"Hello, SM2B!"
    print(f"使用的随机数k: {hex(k)}")
    print(f"【密钥生成】\n用户A私钥: 0x{private_keyA:064x}")
    print(f"用户B私钥: 0x{private_keyB:064x}")
    signatureA = SM2.sm2_sign(msgA, user_id, private_keyA, pubA_x, pubA_y,k)
    signatureB = SM2.sm2_sign(msgB, user_id, private_keyB, pubB_x, pubB_y,k)
    
    r1, s1 = signatureA
    r2, s2 = signatureB
    # 恢复用户B的私钥
    denominator = (s2 + r2) % n
    inv_denom = inverse(denominator, n)
    recovered_private_B = ((k - s2) * inv_denom) % n
    print(f"用户A恢复的私钥d_B: {hex(recovered_private_B)}")
    print(f"恢复结果: {'成功 ' if private_keyB == recovered_private_B else '失败'}")
    # 恢复用户A的私钥
    denominator = (s1 + r1) % n
    inv_denom = inverse(denominator, n)
    recovered_private_A = ((k - s1) * inv_denom) % n
    print(f"用户B恢复的私钥d_A: {hex(recovered_private_A)}")
    print(f"恢复结果: {'成功 ' if private_keyA == recovered_private_A else '失败'}")


if __name__ == "__main__":
    # 执行所有POC验证
    POC_SM2_sig_leaking_k()
    POC_SM2_sig_sameuser_reusing_k()
    POC_SM2_sig_diff_user_reusing_k()