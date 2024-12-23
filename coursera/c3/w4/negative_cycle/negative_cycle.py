#Uses python3

import sys


def negative_cycle(adj, cost):
    #write your code here
    dist = [sys.maxsize for i in range(len(adj))]
    dist[0] = 0
    #print(dist)
    #for i in range(len(adj) - 1):
    for i in range(len(adj)): # The additional iteration
        flag = 0
        for j in range(len(adj)):
            for k in range(len(adj[j])):
                v = adj[j][k]
                if dist[v] > dist[j] + cost[j][k]:
                    dist[v] = dist[j] + cost[j][k]
                    flag = 1
        if not flag:
            break
    #print(dist)
    #print(flag)
    return flag
    #return 0


if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n, m = data[0:2]
    data = data[2:]
    edges = list(zip(zip(data[0:(3 * m):3], data[1:(3 * m):3]), data[2:(3 * m):3]))
    data = data[3 * m:]
    adj = [[] for _ in range(n)]
    cost = [[] for _ in range(n)]
    for ((a, b), w) in edges:
        adj[a - 1].append(b - 1)
        cost[a - 1].append(w)
    print(negative_cycle(adj, cost))
