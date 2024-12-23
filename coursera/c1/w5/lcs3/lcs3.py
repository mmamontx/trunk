#Uses python3

import sys

def lcs3(a, b, c):
    #write your code here
    if len(a) == 0 or len(b) == 0 or len(c) == 0:
        return 0
    n = len(a) + 1
    m = len(b) + 1
    p = len(c) + 1
    d = [[[0 for k in range(p)] for j in range(m)] for i in range(n)]
    for k in range(1, p):
        for j in range(1, m):
            for i in range(1, n):
                if a[i - 1] == b[j - 1] == c[k - 1]:
                    d[i][j][k] = d[i - 1][j - 1][k - 1] + 1
                else:
                    d[i][j][k] = max(d[i - 1][j][k], d[i][j - 1][k], d[i][j][k - 1])
    #for i in range(n):
    #    print(d[i])
    return d[i][j][k]
    #return min(len(a), len(b), len(c))

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    an = data[0]
    data = data[1:]
    a = data[:an]
    data = data[an:]
    bn = data[0]
    data = data[1:]
    b = data[:bn]
    data = data[bn:]
    cn = data[0]
    data = data[1:]
    c = data[:cn]
    print(lcs3(a, b, c))
