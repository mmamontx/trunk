#Uses python3

import sys

def explore(adj, x, c):
    global visited
    visited[x] = 1
    global cc
    cc[x] = c
    for i in range(len(adj[x])):
        if visited[adj[x][i]] == 0:
            explore(adj, adj[x][i], c)

def number_of_components(adj):
    result = 0
    #write your code here
    c = 0
    for i in range(len(adj)):
        if not visited[i]:
            explore(adj, i, c)
            c += 1
    global cc
    return len(set(cc))
    #return result

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n, m = data[0:2]
    visited = [0 for i in range(n)]
    cc = [0 for i in range(n)]
    data = data[2:]
    edges = list(zip(data[0:(2 * m):2], data[1:(2 * m):2]))
    adj = [[] for _ in range(n)]
    for (a, b) in edges:
        adj[a - 1].append(b - 1)
        adj[b - 1].append(a - 1)
    print(number_of_components(adj))
