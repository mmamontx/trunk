# python3

import queue

class Edge:

    def __init__(self, u, v, capacity):
        self.u = u
        self.v = v
        self.capacity = capacity
        self.flow = 0

# This class implements a bit unusual scheme for storing edges of the graph,
# in order to retrieve the backward edge for a given edge quickly.
class FlowGraph:

    def __init__(self, n):
        # List of all - forward and backward - edges
        self.edges = []
        # These adjacency lists store only indices of edges in the edges list
        self.graph = [[] for _ in range(n)]

    def add_edge(self, from_, to, capacity):
        # Note that we first append a forward edge and then a backward edge,
        # so all forward edges are stored at even indices (starting from 0),
        # whereas backward edges are stored at odd indices.
        forward_edge = Edge(from_, to, capacity)
        #backward_edge = Edge(to, from_, 0)
        backward_edge = Edge(to, from_, capacity)
        backward_edge.flow = capacity
        self.graph[from_].append(len(self.edges))
        self.edges.append(forward_edge)
        self.graph[to].append(len(self.edges))
        self.edges.append(backward_edge)

    def size(self):
        return len(self.graph)

    def get_ids(self, from_):
        return self.graph[from_]

    def get_edge(self, id):
        return self.edges[id]

    def add_flow(self, id, flow):
        # To get a backward edge for a true forward edge (i.e id is even), we should get id + 1
        # due to the described above scheme. On the other hand, when we have to get a "backward"
        # edge for a backward edge (i.e. get a forward edge for backward - id is odd), id - 1
        # should be taken.
        #
        # It turns out that id ^ 1 works for both cases. Think this through!
        self.edges[id].flow += flow
        self.edges[id ^ 1].flow -= flow


def read_data():
    vertex_count, edge_count = map(int, input().split())
    graph = FlowGraph(vertex_count)
    for _ in range(edge_count):
        u, v, capacity = map(int, input().split())
        graph.add_edge(u - 1, v - 1, capacity)
    return graph

def bfs(graph, from_, to):
    '''
    for i in range(len(graph.graph)):
        print('')
        for j in range(len(graph.graph[i])):
            eindex = graph.graph[i][j]
            print(graph.edges[eindex].u, graph.edges[eindex].v)
    '''
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
            abs(graph.edges[eindex].flow) < graph.edges[eindex - (eindex % 2)].capacity: # Not sure 'bout the one in the right
                d.put(v)
                visited[v] = True
                #parent[v] = u
                parent[v] = eindex
                #print(eindex)
    #print(visited)
    #print(parent)
    if visited[to] is False:
        return [] # No path
    path = []
    u = to
    while u != 0:
        path.append(parent[u])
        u = graph.edges[parent[u]].u
    #print(path)
    return list(reversed(path)) # It isn't necessary

def add_minflow(graph, path):
    dfmin = graph.edges[path[0]].capacity - graph.edges[path[0]].flow
    #print("df =", dfmin)
    for i in range(1, len(path)):
        df = graph.edges[path[i]].capacity - graph.edges[path[i]].flow
        #print("df =", df)
        if df < dfmin:
            dfmin = df
    #print("dfmin =", dfmin)
    for i in range(len(path)):
        graph.add_flow(path[i], dfmin)
    return dfmin

def max_flow(graph, from_, to):
    flow = 0
    # your code goes here
    #print("from =", from_, "to =", to)
    '''
    maxits = len(graph.graph) * (len(graph.edges) // 2)
    print("maxits =", maxits)
    its = 0
    '''
    while True:
        '''
        print("Before iteration %d:" % (its + 1))
        for i in range(len(graph.edges)):
            if not i % 2:
                print('')
            print(graph.edges[i].u, graph.edges[i].v, "%d/%d" % (graph.edges[i].flow, graph.edges[i].capacity))
        '''
        path = bfs(graph, from_, to)
        if not path:
            return flow
        '''
        print("The path is:")
        for i in range(len(path)):
            print(graph.edges[path[i]].u, graph.edges[path[i]].v)
        '''
        flow += add_minflow(graph, path)
        '''
        its += 1
        if its == maxits:
            print("Reached the maximum number of iterations!")
            return flow
        '''
    return flow


if __name__ == '__main__':
    graph = read_data()
    print(max_flow(graph, 0, graph.size() - 1))
