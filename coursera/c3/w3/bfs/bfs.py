#Uses python3

import sys
import queue
#from collections import deque

def distance(adj, s, t):
    #write your code here
    #print(adj, s, t)
    dist = [-1 for i in range(len(adj))]
    dist[s] = 0
    #d = deque()
    d = queue.Queue()
    d.put(s)
    #print(dist)
    while not d.empty():
        u = d.get()
        for i in range(len(adj[u])):
            if dist[adj[u][i]] == -1:
                 d.put(adj[u][i])
                 dist[adj[u][i]] = dist[u] + 1
    #print(dist)
    return dist[t]
    #return -1

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n, m = data[0:2]
    data = data[2:]
    edges = list(zip(data[0:(2 * m):2], data[1:(2 * m):2]))
    adj = [[] for _ in range(n)]
    for (a, b) in edges:
        adj[a - 1].append(b - 1)
        adj[b - 1].append(a - 1)
    s, t = data[2 * m] - 1, data[2 * m + 1] - 1
    print(distance(adj, s, t))
