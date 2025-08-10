from sympy import isprime, mod_inverse
import gmpy2
import random

def generate_prime_candidate(length):
    p = random.getrandbits(length) 
    p |= (1 << length - 1) | 1 
    return p

#生成一个指定长度的大素数
def generate_large_prime(length):
    p = 4  
    while not isprime(p): 
        p = generate_prime_candidate(length)
    return p

#生成Paillier公钥和私钥
def generate_keypair(bit_length):
    p = generate_large_prime(bit_length)
    q = generate_large_prime(bit_length)
    n = p * q  
    g = n + 1 
    lambda_val = gmpy2.lcm(p - 1, q - 1)  # 计算最小公倍数
    return (n, g), lambda_val

def encrypt(pk, m):
    n, g = pk
    r = random.randint(1, n)
    c = pow(g, m, n ** 2) * pow(r, n, n ** 2) % n ** 2

    return c

def decrypt(pk, sk, c):
    n, g = pk
    lambda_val = sk

    #u = c^lambda mod n^2
    u = pow(c, lambda_val, n ** 2)
    L_u = (u - 1) // n

    # 计算 L(g^lambda mod n^2) 的模逆元素
    L_g_lambda_inv = mod_inverse((pow(g, lambda_val, n ** 2) - 1) // n, n)
    return (L_u * L_g_lambda_inv) % n

#Paillier同态加法
def add(pk, c1, c2):
    n, g = pk
    return c1 * c2 % (n ** 2)

#Paillier同态乘法
def mul(pk, c1, c2):
    n, g = pk
    return pow(c1,c2,(n ** 2))

if __name__ == "__main__":
    print('同态加密测试')
    public_key, private_key = generate_keypair(1024)
    m1=112
    c1=encrypt(public_key,m1)
    m_1=decrypt(public_key,private_key,c1)
    m2=222
    c2=encrypt(public_key,m2)
    m_2=decrypt(public_key,private_key,c2)
    c3=add(public_key,c1,c2)
    c4=mul(public_key,c1,m2)
    print('c1解密结果：',m_1)
    print('c2解密结果：',m_2)
    print('c1+c2解密结果：',decrypt(public_key,private_key,c3))
    print('c1*m2解密结果：',decrypt(public_key,private_key,c4))