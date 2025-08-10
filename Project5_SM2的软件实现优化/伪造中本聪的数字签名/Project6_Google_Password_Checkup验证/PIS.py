from sympy import isprime, mod_inverse
import gmpy2
import random
from Paillier import  generate_keypair,encrypt,decrypt,add
from BF import BloomFilter



#Paillier_test()
#洗牌函数
def FisherYates(a):
    l = len(a)
    for i in range(l):
        j = random.randint(0,l-i-1)
        temp = a[j]
        a[j] = a[l-i-1]
        a[l-i-1] = temp

#选择椭圆曲线群P256
from ecc.curve import P256, Point
n=P256.n
g=P256.G

#建立各自消息集合
#P1:消息
m1 = random.randint(0,100)
P1_data = []
for i in range(m1):
    P1_data.append(g*random.randint(1,100))

#P2：消息以及对应权重
m2 = random.randint(0,100)
P2_data = []
for i in range(m2):
    P2_data.append([g*random.randint(1,100),random.randint(1,100)])

#setup阶段
#P1，P2产生各自的私钥
P1_sk = random.randint(0,n)
P2_sk = random.randint(0,n)
#P2生成同态加密公私钥对,其中公钥公开
AHE_pk,AHE_sk = generate_keypair(1024)

#round1
#P1将vi进行哈希，将结果使用自己的私钥乘幂，打乱之后发送给P2
vi_Hash_sk1=[]
for i in range(m1):
    vi_Hash_sk1.append(pow(hash(P1_data[i].x^P1_data[i].y),P1_sk,n))

FisherYates(vi_Hash_sk1)

#round2
#P2将P1发来的消息使用自己的私钥乘幂，打乱之后发送给P1
vi_Hash_sk1_sk2 =[]
wi_Hash_sk2 =[]
for i in range(len(vi_Hash_sk1)):
    vi_Hash_sk1_sk2.append(pow(vi_Hash_sk1[i],P2_sk,n))

FisherYates(vi_Hash_sk1_sk2)

#P2将wi进行哈希，将结果使用自己的私钥乘幂，并将对应的权重进行同态加密，打乱之后发送给P1
for i in range(m2):
    wi_Hash_sk2.append([pow(hash(P2_data[i][0].x^P2_data[i][0].y),P2_sk,n),encrypt(AHE_pk,P2_data[i][1])])

#round3
#P1将P2发来的消息的wi处理过的部分使用自己的私钥乘幂，与先前得到的vi处理后的消息集合求交集
wi_Hash_sk2_sk1=[]
for i in range(len(wi_Hash_sk2)):
    wi_Hash_sk2_sk1.append(pow(wi_Hash_sk2[i][0],P1_sk,n))

AHE_wts =[]
bloom = BloomFilter(N=m1)
for i in range(m1):
    bloom.add(str(vi_Hash_sk1_sk2[i]))

wts_ex =[]
for i in range(len(wi_Hash_sk2_sk1)):
    if bloom.contains(str(wi_Hash_sk2_sk1[i])) == True:
        AHE_wts.append(wi_Hash_sk2[i][1])
        wts_ex.append(1)
    else: wts_ex.append(0)

#P1根据交集筛选P2发来的消息的权重部分，对其进行同态加法，将结果发送给P2
AHE_wts_sum =AHE_wts[0]
for i in range(1,len(AHE_wts)):
    AHE_wts_sum = add(AHE_pk,AHE_wts_sum,AHE_wts[i])

adds =0
wts_sum = decrypt(AHE_pk,AHE_sk,AHE_wts_sum)
for i in range(len(P2_data)):
    if wts_ex[i] == 1:
        adds+=P2_data[i][1]
print(wts_sum)
print(adds)


