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

def minimum_distance(x, y):
    result = 0.
    #write your code here
    #print(x, y)
    edges = []
    for i in range(len(x)):
        for j in range(i + 1, len(x)):
            edges.append((i, j, math.sqrt(math.pow(x[i] - x[j], 2) + math.pow(y[i] - y[j], 2))))
    #print(edges)
    edges.sort(key=lambda edges: edges[2])
    #print(edges)
    global sets
    for i in range(len(x)):
        s = set()
        s.add(i)
        sets.append(s)
    #print(sets)
    x = []
    for i in range(len(edges)):
        u = edges[i][0]
        v = edges[i][1]
        #print("u is", u, "find() is", find(u))
        #print("v is", v, "find() is", find(v))
        ui = find(u)
        vi = find(v)
        if ui != vi:
            x.append(edges[i])
            sets[ui] |= sets[vi]
            sets.pop(vi)
        #print()
    #print(x)
    for i in range(len(x)):
        result += x[i][2]
    return result


if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n = data[0]
    x = data[1::2]
    y = data[2::2]
    print("{0:.9f}".format(minimum_distance(x, y)))
