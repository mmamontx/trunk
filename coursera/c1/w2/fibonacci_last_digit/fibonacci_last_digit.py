# Uses python3
import sys

def get_fibonacci_last_digit(n):
    # write your code here
    if n <= 1:
        return n

    f = [0, 1]

    for i in range(2, n + 1):
        #f.insert(i, (f[i - 2] + f[i - 1]) % 10)
        f[i % 2] = (f[0] + f[1]) % 10

    #return f[n]
    return f[i % 2]

if __name__ == '__main__':
    #input = sys.stdin.read()
    n = int(input())
    print(get_fibonacci_last_digit(n))
