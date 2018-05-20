#Uses python3
import sys
import math

sets = []

def find(x):
    global sets
    for i in range(len(sets)):
        if x in sets[i]:
            return i
    return -1

def clustering(x, y, k):
    #write your code here
    edges = []
    for i in range(len(x)):
        for j in range(i + 1, len(x)):
            edges.append((i, j, math.sqrt(math.pow(x[i] - x[j], 2) + math.pow(y[i] - y[j], 2))))
    edges.sort(key=lambda edges: edges[2])
    global sets
    for i in range(len(x)):
        s = set()
        s.add(i)
        sets.append(s)
    x = []
    for i in range(len(edges)):
        u = edges[i][0]
        v = edges[i][1]
        ui = find(u)
        vi = find(v)
        if ui != vi:
            x.append(edges[i])
            sets[ui] |= sets[vi]
            sets.pop(vi)
    #print(x)
    return x[-k + 1][2]
    #return -1.


if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n = data[0]
    data = data[1:]
    x = data[0:2 * n:2]
    y = data[1:2 * n:2]
    data = data[2 * n:]
    k = data[0]
    print("{0:.9f}".format(clustering(x, y, k)))
