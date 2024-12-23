#Uses python3

import sys

def explore(adj, x):
    global visited
    visited[x] = 1
    for i in range(len(adj[x])):
        if visited[adj[x][i]] == 0:
            explore(adj, adj[x][i])

def reach(adj, x, y):
    #write your code here
    #print(adj, x, y)
    global visited
    #print(visited)
    explore(adj, x)
    #print(visited)
    return visited[y]
    #return 0

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n, m = data[0:2]
    visited = [0 for i in range(n)]
    data = data[2:]
    edges = list(zip(data[0:(2 * m):2], data[1:(2 * m):2]))
    x, y = data[2 * m:]
    adj = [[] for _ in range(n)]
    x, y = x - 1, y - 1
    for (a, b) in edges:
        adj[a - 1].append(b - 1)
        adj[b - 1].append(a - 1)
    print(reach(adj, x, y))
