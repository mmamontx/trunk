#Uses python3

import sys
import queue


def bfs(adj, s):
    dist = [-1 for i in range(len(adj))]
    d = queue.Queue()
    for i in s:
        dist[i] = 0
        d.put(i)
    while not d.empty():
        u = d.get()
        for i in range(len(adj[u])):
            if dist[adj[u][i]] == -1:
                 d.put(adj[u][i])
                 dist[adj[u][i]] = dist[u] + 1
    a = set()
    for i in range(len(dist)):
        if dist[i] != -1:
            a.add(i)
    return a

def shortet_paths(adj, cost, s, distance, reachable, shortest):
    #write your code here
    distance[s] = 0

    for i in range(len(adj) - 1):
        flag = 0
        for j in range(len(adj)):
            for k in range(len(adj[j])):
                v = adj[j][k]
                if distance[v] > distance[j] + cost[j][k]:
                    distance[v] = distance[j] + cost[j][k]
                    flag = 1
        if not flag:
            break

    # The additional iteration
    a = set()
    if flag:
        i = len(adj) - 1
        for j in range(len(adj)):
            for k in range(len(adj[j])):
                v = adj[j][k]
                if distance[v] > distance[j] + cost[j][k]:
                    distance[v] = distance[j] + cost[j][k]
                    a.add(v)

    b = bfs(adj, a)

    sets = set()
    sets.add(s)
    c = bfs(adj, sets)

    for i in range(len(adj)):
        if i in c:
            reachable[i] = 1
        if i in b | a:
            shortest[i] = 0
    #pass


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
    s = data[0]
    s -= 1
    #distance = [10**19] * n
    distance = [float('inf')] * n # NOTE: Not sure if it is needed
    reachable = [0] * n
    shortest = [1] * n
    shortet_paths(adj, cost, s, distance, reachable, shortest)
    for x in range(n):
        if reachable[x] == 0:
            print('*')
        elif shortest[x] == 0:
            print('-')
        else:
            print(distance[x])

