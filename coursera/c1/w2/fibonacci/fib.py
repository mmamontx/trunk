# Uses python3
def calc_fib(n):
    if (n <= 1):
        return n

    f = [0, 1]

    for i in range(2, n + 1):
        f.insert(i, f[i - 2] + f[i - 1])

    return f[n]
    #return calc_fib(n - 1) + calc_fib(n - 2)

n = int(input())
print(calc_fib(n))
