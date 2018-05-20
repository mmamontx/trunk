# Uses python3
import sys

def calc_fibm(n, m):
    if (n <= 1) or (m <= 1):
        return n

    f = [0, 1]

    for i in range(2, n + 1):
        f[i % 2] = (f[0] + f[1]) % m

    return f[i % 2]

def calc_pis(n, m):
    if (n <= 1) or (m <= 1):
        return n

    f = [0, 1]

    for i in range(2, n + 1):
        f[i % 2] = (f[0] + f[1]) % m

        if (f[i % 2] == 1) and (f[(i - 1) % 2] == 0):
            break

    return i - 1

def get_fibonaccihuge(n, m):
    # write your code here
    if (m > n):
        return calc_fibm(n, m)

    pis = calc_pis(n, m)
    #print("The period is", pis)
    rem = n % pis
    #print("The remainder is", rem)
    return calc_fibm(rem, m)
    #return 0

if __name__ == '__main__':
    input = sys.stdin.read();
    n, m = map(int, input.split())
    print(get_fibonaccihuge(n, m))
