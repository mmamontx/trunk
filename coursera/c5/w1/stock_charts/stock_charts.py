# python3
import queue

def add_minflow(graph, path):
    dfmin = graph.edges[path[0]].capacity - graph.edges[path[0]].flow
    for i in range(1, len(path)):
        df = graph.edges[path[i]].capacity - graph.edges[path[i]].flow
        if df < dfmin:
            dfmin = df
    for i in range(len(path)):
        graph.add_flow(path[i], dfmin)
    return dfmin

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
            abs(graph.edges[eindex].flow) < graph.edges[eindex - (eindex % 2)].capacity: # Not sure 'bout the one in the right
                d.put(v)
                visited[v] = True
                parent[v] = eindex
    if visited[to] is False:
        return [] # No path
    path = []
    u = to
    while u != 0:
        path.append(parent[u])
        u = graph.edges[parent[u]].u
    return list(reversed(path)) # It isn't necessary

def max_flow(graph, from_, to):
    flow = 0
    while True:
        path = bfs(graph, from_, to)
        if not path:
            return flow
        flow += add_minflow(graph, path)
    return flow

class Edge:
    def __init__(self, u, v, capacity):
        self.u = u
        self.v = v
        self.capacity = capacity
        self.flow = 0

class FlowGraph:
    def __init__(self, n):
        self.edges = []
        self.graph = [[] for _ in range(n)]

    def add_edge(self, from_, to, capacity):
        forward_edge = Edge(from_, to, capacity)
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
        self.edges[id].flow += flow
        self.edges[id ^ 1].flow -= flow

class StockCharts:
    def read_data(self):
        n, k = map(int, input().split())
        stock_data = [list(map(int, input().split())) for i in range(n)]
        return stock_data

    def write_response(self, result):
        print(result)

    def min_charts(self, stock_data):
        # Replace this incorrect greedy algorithm with an
        # algorithm that correctly finds the minimum number
        # of charts on which we can put all the stock data
        # without intersections of graphs on one chart.
        n = len(stock_data)
        k = len(stock_data[0])
        vertex_count = n * 2 + 2 # 2 for each stock + source and sink
        graph = FlowGraph(vertex_count)
        #print("n =", n, "k =", k)
        charts = []
        for i in range(len(stock_data)):
            for j in range(i + 1, len(stock_data)):
                #print("Stock", i, "=", stock_data[i])
                #print("Stock", j, "=", stock_data[j])
                above = all([x > y for x, y in zip(stock_data[i], stock_data[j])])
                below = all([x < y for x, y in zip(stock_data[i], stock_data[j])])
                if above:
                    #print("above =", above)
                    graph.add_edge(1 + i, int(vertex_count / 2) + j, 1)
                elif below:
                    #print("below =", below)
                    graph.add_edge(1 + j, int(vertex_count / 2) + i, 1)
                #else:
                #    print("Charts doesn't fit.")
        for i in range(n):
            # Add edge from source to each vertex in left group with capacity 1
            graph.add_edge(0, 1 + i, 1)
            # Add edge from each vertex in right group to sink with capacity 1
            graph.add_edge(int(vertex_count / 2) + i, vertex_count - 1, 1)
        mf = max_flow(graph, 0, graph.size() - 1)
        #for edge in graph.edges:
        #    print(edge.u, edge.v, edge.flow)
        #print("mf =", mf)
        '''
        for new_stock in stock_data:
            added = False
            for chart in charts:
                fits = True
                for stock in chart:
                    above = all([x > y for x, y in zip(new_stock, stock)])
                    below = all([x < y for x, y in zip(new_stock, stock)])
                    if (not above) and (not below):
                        fits = False
                        break
                if fits:
                    added = True
                    chart.append(new_stock)
                    break
            if not added:
                charts.append([new_stock])
        '''
        return n - mf
        #return len(charts)

    def solve(self):
        stock_data = self.read_data()
        result = self.min_charts(stock_data)
        self.write_response(result)

if __name__ == '__main__':
    stock_charts = StockCharts()
    stock_charts.solve()
