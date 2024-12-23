# Uses python3
import sys

def evalt(a, b, op):
    if op == '+':
        #return a + b
        return int(a) + int(b)
    elif op == '-':
        #return a - b
        return int(a) - int(b)
    elif op == '*':
        #return a * b
        return int(a) * int(b)
    else:
        assert False

def MinAndMax(d, op, m, M, i, j):
    mi = sys.maxsize
    ma = -sys.maxsize - 1
    for k in range(i, j):
        a = evalt(M[i][k], M[k + 1][j], op[k])
        b = evalt(M[i][k], m[k + 1][j], op[k])
        c = evalt(m[i][k], M[k + 1][j], op[k])
        e = evalt(m[i][k], m[k + 1][j], op[k])
        #print([mi, a, b, c, e])
        mi = min([mi, a, b, c, e])
        ma = max([ma, a, b, c, e])
    #print(ma)
    #print(mi)
    return mi, ma

def get_maximum_value(dataset):
    #write your code here
    #print(dataset)
    d = []
    op = []
    for i in range(len(dataset)):
        if i % 2:
            op.append(dataset[i])
        else:
            d.append(dataset[i])
    #print(d)
    #print(op)
    n = len(d)
    m = [[0 for j in range(n)] for i in range(n)]
    M = [[0 for j in range(n)] for i in range(n)]
    for i in range(n):
        m[i][i] = d[i]
        M[i][i] = d[i]
    for s in range(1, n):
        for i in range(0, n - s):
            j = i + s
            #print("i is", i, "j is", j)
            m[i][j], M[i][j] = MinAndMax(d, op, m, M, i, j)
    #return 0
    return M[0][n - 1]


if __name__ == "__main__":
    print(get_maximum_value(input()))
