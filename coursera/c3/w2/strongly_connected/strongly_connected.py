#Uses python3

import sys

sys.setrecursionlimit(200000)

def explore(adj, x):
    global visited
    visited[x] = 1
    for i in range(len(adj[x])):
        if visited[adj[x][i]] == 0:
            explore(adj, adj[x][i])

def dfs(adj, used, order, x):
    used[x] = 1
    for i in range(len(adj[x])):
        if used[adj[x][i]] == 0:
           used, order = dfs(adj, used, order, adj[x][i])
    order.append(x)
    return used, order

def toposort(adj):
    used = [0] * len(adj)
    order = []
    for i in range(len(adj)):
        if used[i] == 0:
            used, order = dfs(adj, used, order, i)
    return reversed(order)

def number_of_strongly_connected_components(adj):
    #write your code here
    global visited
    #print(adj)
    revadj = [[] for _ in range(len(adj))]
    for i in range(len(adj)):
        for j in range(len(adj[i])):
            revadj[adj[i][j]].append(i)
    #print(revadj)
    order = toposort(revadj)
    result = 0
    for x in order:
        if visited[x] == 0:
            explore(adj, x)
            result += 1
    return result

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n, m = data[0:2]
    visited = [0 for i in range(n)]
    data = data[2:]
    edges = list(zip(data[0:(2 * m):2], data[1:(2 * m):2]))
    adj = [[] for _ in range(n)]
    for (a, b) in edges:
        adj[a - 1].append(b - 1)
    print(number_of_strongly_connected_components(adj))
