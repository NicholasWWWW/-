import SM2

"""==========利用广义梅森素数优化椭圆曲线运算 =========="""
def fast_reduce(x):
    """
    利用广义梅森素数性质实现的快速模约减
    输入：x (0 <= x < p²)
    输出：x mod p
    """
    # 将 x 分解为 256 位块
    x_low = x & ((1 << 256) - 1)  # 低 256 位
    x_high = x >> 256             # 高 256 位
    # 利用恒等式：2²⁵⁶ ≡ 2²²⁴ + 2⁹⁶ - 2⁶⁴ + 1 (mod p)
    term1 = x_high << 224  # x_high * 2²²⁴
    term2 = x_high << 96   # x_high * 2⁹⁶
    term3 = x_high << 64   # x_high * 2⁶⁴ (用于减法)
    term4 = x_high         # x_high * 1
    # 组合结果：x = x_low + term1 + term2 - term3 + term4
    result = x_low + term1 + term2 + term4 - term3
    
    # 模约减（最多需要两次减法）
    while result >= SM2.p:
        result -= SM2.p
    while result < 0:
        result += SM2.p
    return result

def fast_inverse(a):
    """
    利用费马小定理和快速模幂实现的模逆元
    输入：a (0 < a < p)
    输出：a⁻¹ mod p
    """
    # 计算指数：exp = p - 2
    a2 = fast_multiply(a, a)    # 计算 a²
    a4 = fast_multiply(a2, a2)  # 计算 a⁴ = (a²)²
    a8 = fast_multiply(a4, a4)   # 计算 a⁸ = (a⁴)²
    a16 = fast_multiply(a8, a8)    # 计算 a¹⁶ = (a⁸)²
    t = a16    # 继续平方直到 a²⁵⁶
    for _ in range(4):  # 16 -> 32 -> 64 -> 128 -> 256
        t = fast_multiply(t, t)
    a224 = t        # 计算 a²²⁴ = a²⁵⁶ / a³²
    for _ in range(32):
        a224 = fast_multiply(a224, a)
    a96 = a16    # 计算 a⁹⁶
    for _ in range(4):  # 16 -> 32 -> 64 -> 96 (需要额外乘以 a³²)
        a96 = fast_multiply(a96, a96)
    a32 = a16
    for _ in range(16):  # 16 -> 32
        a32 = fast_multiply(a32, a32)
    a96 = fast_multiply(a96, a32)
    
    # 组合结果：a^{p-2} = a²⁵⁶ / (a²²⁴ * a⁹⁶) * a⁻³
    denominator = fast_multiply(a224, a96)
    inverse_denom = fast_inverse(denominator)  # 递归计算小指数逆元
    result = fast_multiply(t, inverse_denom)
    result = fast_multiply(result, fast_inverse(fast_multiply(a, a2)))
    
    return result

def fast_multiply(a, b):
    """利用快速模约减的乘法"""
    return fast_reduce(a * b)
def ec_add_point(x1, y1, x2, y2):
    """优化后的椭圆曲线点加法函数"""
    # SM2 曲线参数
    a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
    
    if x1 != x2:
        # 计算 λ = (y2 - y1)/(x2 - x1)
        dy = fast_reduce(y2 - y1)  # 优化模减法
        dx = fast_reduce(x2 - x1)
        inv_dx = fast_inverse(dx)
        lamda = fast_reduce(dy * inv_dx)
    else:
        # 倍点运算：λ = (3x₁² + a)/(2y₁)
        x1_sq = fast_reduce(x1 * x1)
        three_x1_sq = fast_reduce(3 * x1_sq)
        numerator = fast_reduce(three_x1_sq + a)
        denominator = fast_reduce(2 * y1)
        inv_denom = fast_inverse(denominator)
        lamda = fast_reduce(numerator * inv_denom)
   
    # 计算 x3 = λ² - x1 - x2
    lamda_sq = fast_reduce(lamda * lamda)
    x3 = fast_reduce(lamda_sq - x1 - x2)
    # 计算 y3 = λ(x1 - x3) - y1
    dx1x3 = fast_reduce(x1 - x3)
    lamda_dx = fast_reduce(lamda * dx1x3)
    y3 = fast_reduce(lamda_dx - y1)
    
    return x3, y3

"""==========预计算优化椭圆曲线多倍点运算=========="""
def precompute_fixed_base(base_x, base_y, max_k_bits=256):
    """预计算固定基点的所有2^i倍点"""
    # 初始化表，存储[P, 2P, 4P, 8P,...]
    table = [(base_x, base_y)]  # 第一个元素是2^0*P = P
    
    # 循环计算2^i倍点
    for _ in range(1, max_k_bits):
        last = table[-1]  # 获取表中最新的点
        # 计算当前点的倍点：2^(i)P = 2 * 2^(i-1)P
        next_point = SM2.ec_add_point(last[0], last[1], last[0], last[1])
        table.append(next_point)
    return table

def ec_multiply_fixed_base(k, precomputed_table):
    """使用预计算表进行多倍点运算"""
    result = None  # 初始化结果为无穷远点
    # 遍历预计算表中的每个点
    for i, point in enumerate(precomputed_table):
        # 检查k的第i位是否为1
        if (k >> i) & 1:
            if result is None:
                # 首次遇到1的位，直接赋值
                result = point
            else:
                # 将当前点加到结果中
                result = SM2.ec_add_point(result[0], result[1], point[0], point[1])
    return result

