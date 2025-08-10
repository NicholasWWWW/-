## Project 6: Google Password Checkup
#### 项目分工
| 姓名 | 分工                     |
|-------|--------------------------|
|  童皓琛   |          |
|  崔倡通  |  |

### 实验目标
参考论文 https://eprint.iacr.org/2019/723.pdf 的 section 3.1 ，也即 Figure 2 中展示的协议，尝试实现该协议
### 实验内容
#### a) 分析协议，实现所需组件
协议如下，其流程在后面会结合代码一起讲解。这里首先确定协议需要的组件
![alt text](.\image\image.png)
1：**加法同态加密AHE**
这里考虑使用Paillier同态加密。先编写一些辅助函数，generate_prime_candidate：生成指定长度的奇数，generate_large_prime：生成指定长度的素数，
方法是先生成奇数再检验是不是素数。用到了sympy库的素数检验函数isprime
![alt text](.\image\image-1.png)
然后生成paillier的公私钥。生成两个大素数p与q，公钥为n=p*q，以及g=n+1；私钥为p-1与q-1的最小公倍数$\lambda$
![alt text](.\image\image-2.png)
加密为：选择随机数r，计算
$c= g^{m}*r^n mod n^2$
![alt text](.\image\image-3.png)
解密为：先计算$u = c^{\lambda} mod n^2$,其中的L()函数的表达式是
$L(x) = (x-1)//n$，用此计算出$L(u)$以及$L(g^{\lambda}mod n^2)^{-1}$,最后返回
$L(u)*L(g^{\lambda}mod n^2)^{-1} mod n$
![alt text](.\image\image-4.png)
同态加法非常简单，对于明文$c1$与$c2$，其同态和为$(c1*c2)modn^2$
![alt text](.\image\image-5.png)
同态加法测试：
![alt text](.\image\image-6.png)
测试结果如下：
![alt text](.\image\image-7.png)
值得一提的是，Paillier同态加密也有部分乘法同态的性质，但不能像BFV一样进行多次同态乘法，这里与此实验无关便不多提及。还有就是公钥中g的选择，此处选择了g=n+1。实际上选择一个满足$gcd(L(g^x mod n^2), n)=1$的数即可。

2：**布隆过滤器BF**
在实现协议时有一步是对两个集合求交集，此处本想使用穷举，但其时间复杂度高达$O(n^2)$，效率低下。这里使用效率更高的布隆过滤器。

布隆过滤器（Bloom Filter）是一种空间效率高、查询速度快的概率型数据结构，用于判断一个元素是否属于某个集合。他具有占用内存少，查询时间快的优点，仅仅需要$O(n)$的时间复杂度即可完成求交集的任务。其有一定的误判率，但不会漏判。误判率取决于其使用的哈希函数，这里会结合实现来说明。

首先，选择过滤器所使用的哈希函数。这里选择了三种，分别是BKDR Hash，AP Hash，DJB Hash。
![alt text](.\image\image-8.png)
然后是结构体BF，其中N表示其中的数据规模，X则表示每个数据分配的内存大小，bit_array数组存储数据的每一位bit。hash_funcs存储了所用的哈希函数。_set_bit与_get_bit分别用于将bit_array的某一位设为1，和检查bit_array的某一位是否为1
![alt text](.\image\image-9.png)
接下来两个函数add于contains分别用于将数据添加进BF，和监测数据是否在BF中。前者分别使用三种hash函数对数据进行哈希，对每个哈希值取模M，得到在位数组中的索引，然后调用 _set_bit 将对应比特位设为1；后者则一一对比数据的哈希结果与BF中存储的结果，如果所有哈希对应的比特位均为1则返回 True（概率误判），如果任意一个比特位为0则返回 False（一定不存在）。
![alt text](.\image\image-10.png)
下面是BF过滤器的测试
![alt text](.\image\image-11.png)
测试结果如下：
![alt text](.\image\image-12.png)
3：**洗牌函数**
协议中需要将数组打乱，这里使用洗牌函数FisherYates实现
![alt text](.\image\image-13.png)

4：**所用的群**
协议中的集合中的元素所在的群使用了椭圆曲线群P256，基点为g，群的阶数为n
![alt text](.\image\image-14.png)

完成这些后，接下来实现协议
#### b) 实现协议
建立二者消息集合，P1拥有$V = {v_i}_{i=1}^{m_1}$,P2拥有$W = {w_i}_{i=1}^{m_2}$,以及$T = {t_i}_{i=1}^{m_2}$，其中$t_i$表示$w_i$的权重

![alt text](.\image\image-15.png)

setup阶段，P1与P2生成各自的私钥，P2生成同态加密公私钥对，并将公钥分享给P1
![alt text](.\image\image-16.png)

round1，P1将$v_i$哈希，然后将结果使用自己的私钥进行乘幂，打乱顺序后发送给P2
![alt text](.\image\image-17.png)

round2，P2将P1发来的消息使用自己的私钥乘幂之后发送给P1；将$w_i$哈希，然后将结果使用自己的私钥进行乘幂，打乱顺序后发送给P1；将$t_i$进行同态加密后发送给P1
![alt text](.\image\image-18.png)

round3，P1将P2发来的$W_i$的处理后的消息使用自己的私钥乘幂，并与$v_i$处理后的消息一起，使用BF过滤器，求出二者的交集，再根据二者交集筛选出对应的同态加密后的权重集，对此进行同态加法。将结果发送给P2，P2解密出权重和
求交集：
![alt text](.\image\image-19.png)
求权重：
![alt text](.\image\image-20.png)
其中的wts_sum是按照协议求出的权重和，adds是用来测试其准确性的。打印出结果，发现二者相同，表示协议执行成功
![alt text](.\image\image-21.png)