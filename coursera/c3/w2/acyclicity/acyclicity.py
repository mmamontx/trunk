#Uses python3

import sys

'''
def previsit(x):
    global previs, clock
    previs[x] = clock
    clock += 1

def postvisit(x):
    global postvis, clock
    postvis[x] = clock
    clock += 1
'''

def explore(adj, x):
    global visited
    #print("Exploring", x)
    #print(adj[x])
    visited[x] = 1
    #previsit(x)
    for i in range(len(adj[x])):
        if visited[adj[x][i]] == 0:
            rv = explore(adj, adj[x][i])
            if rv == 1:
                return 1
        else:
            return 1
    visited[x] = 0
    #postvisit(x)
    return 0

def acyclic(adj):
    #global previs, postvis
    global visited
    #print(adj)
    for i in range(len(adj)):
        rv = explore(adj, i)
        if rv == 1:
            return 1
        #else:
        #    print(visited)
        #    for j in range(len(visited)):
        #        visited[j] = 0
    #print(previs)
    #print(postvis)
    return 0

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n, m = data[0:2]
    visited = [0 for i in range(n)]
    #clock = 1
    #previs = [0 for i in range(n)]
    #postvis = [0 for i in range(n)]
    data = data[2:]
    edges = list(zip(data[0:(2 * m):2], data[1:(2 * m):2]))
    adj = [[] for _ in range(n)]
    for (a, b) in edges:
        adj[a - 1].append(b - 1)
    print(acyclic(adj))
