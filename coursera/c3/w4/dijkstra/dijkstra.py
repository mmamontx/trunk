#Uses python3

import sys
import queue


def distance(adj, cost, s, t):
    #write your code here
    #print(adj)
    #print(cost)
    #print(s, t)
    dist = [sys.maxsize for i in range(len(adj))]
    #prev = [-1 for i in range(len(adj))]
    #print(dist)
    dist[s] = 0
    h = queue.PriorityQueue()
    for i in range(len(adj)):
        h.put((dist[i], i))
    #print("true" if h.empty() else "false")
    #while not h.empty():
    for j in range(len(adj)):
        (p, u) = h.get()
        #print(u)
        for i in range(len(adj[u])):
            v = adj[u][i]
            if dist[v] > dist[u] + cost[u][i]:
                dist[v] = dist[u] + cost[u][i]
                #prev[v] = u
                h.put((dist[v], v))
    return dist[t] if dist[t] != sys.maxsize else -1
    #return -1


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
    s, t = data[0] - 1, data[1] - 1
    print(distance(adj, cost, s, t))
