# python3

import queue

class edge:
    def __init__(self, u, v, capacity, flow):
        self.u = u
        self.v = v
        self.capacity = capacity
        self.flow = flow

class graph:
    def __init__(self, n):
        self.edges = []
        self.graph = [[] for i in range(n)]

    def add_edge(self, u, v):
        e = edge(u, v, 1, 0)
        be = edge(v, u, 1, 1)
        self.graph[u].append(len(self.edges))
        self.graph[v].append(len(self.edges) + 1)
        self.edges += [e, be]

    def add_flow(self, id, flow):
        self.edges[id].flow += flow
        self.edges[id ^ 1].flow -= flow

def bfs(graph, from_, to):
    visited = [False for i in range(len(graph.graph))]
    parent = [-1 for i in range(len(graph.graph))]
    visited[from_] = True
    d = queue.Queue()
    d.put(from_)
    while not d.empty():
        u = d.get()
        for i in range(len(graph.graph[u])):
            eindex = graph.graph[u][i]
            v = graph.edges[eindex].v
            if visited[v] is False and \
            graph.edges[eindex].flow < graph.edges[eindex].capacity:
            #abs(graph.edges[eindex].flow) < graph.edges[eindex - (eindex % 2)].capacity:
                d.put(v)
                visited[v] = True
                parent[v] = eindex
    if visited[to] is False:
        return []
    path = []
    u = to
    #while u != 0:
    while u != from_:
        path.append(parent[u])
        u = graph.edges[parent[u]].u
    #return list(reversed(path))
    return path

def add_minflow(graph, path):
    dfmin = graph.edges[path[0]].capacity - graph.edges[path[0]].flow
    for i in range(1, len(path)):
        df = graph.edges[path[i]].capacity - graph.edges[path[i]].flow
        if df < dfmin:
            dfmin = df
    for i in range(len(path)):
        graph.add_flow(path[i], dfmin)
    return dfmin

def max_flow(graph, from_, to):
    flow = 0
    while True:
        path = bfs(graph, from_, to)
        if not path:
            return flow
        flow += add_minflow(graph, path)
    return flow

class MaxMatching:
    def read_data(self):
        n, m = map(int, input().split())
        adj_matrix = [list(map(int, input().split())) for i in range(n)]
        return adj_matrix

    def write_response(self, matching):
        line = [str(-1 if x == -1 else x + 1) for x in matching]
        print(' '.join(line))

    def find_matching(self, adj_matrix):
        # Replace this code with an algorithm that finds the maximum
        # matching correctly in all cases.
        n = len(adj_matrix)
        m = len(adj_matrix[0])
        matching = [-1] * n
        '''
        busy_right = [False] * m
        for i in range(n):
            for j in range(m):
                if adj_matrix[i][j] and matching[i] == -1 and (not busy_right[j]):
                    matching[i] = j
                    busy_right[j] = True
        '''
        g = graph(n + m + 2) # + source & sink vertices
        for i in range(n):
            for j in range(m):
                if adj_matrix[i][j]:
                    g.add_edge(i, j + n) # + n is to make numeration of the flights and crews continuous
        num_main_edges = len(g.edges)
        src = n + m
        sink = n + m + 1
        for i in range(n):
            g.add_edge(src, i)
        for i in range(n, m + n):
            g.add_edge(i, sink)
        mf = max_flow(g, src, sink)
        for i in range(num_main_edges):
            if (not (i % 2)) and g.edges[i].flow > 0:
                #print("%d)" % i, g.edges[i].u, g.edges[i].v, "%d/%d" % (g.edges[i].flow, g.edges[i].capacity))
                matching[g.edges[i].u] = g.edges[i].v - n
        return matching

    def solve(self):
        adj_matrix = self.read_data()
        matching = self.find_matching(adj_matrix)
        self.write_response(matching)

if __name__ == '__main__':
    max_matching = MaxMatching()
    max_matching.solve()
