#Uses python3

import sys
import queue

def bipartite(adj):
    #write your code here
    part = [-1 for i in range(len(adj))]
    part[0] = 0
    d = queue.Queue()
    d.put(0)
    #print(part)
    while not d.empty():
        u = d.get()
        for i in range(len(adj[u])):
            if part[adj[u][i]] == -1:
                 d.put(adj[u][i])
                 part[adj[u][i]] = (part[u] + 1) % 2
            elif part[adj[u][i]] == part[u]:
                 return 0
    #print(part)
    return 1
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
    print(bipartite(adj))
