# python3

import sys
import threading

sys.setrecursionlimit(10**6) # max depth of recursion
threading.stack_size(2**26)  # new thread will get stack of such size

index = 0
S = []
V = []
adj = []

class vertex:
    def __init__(self, index, lowlink, onStack, adjindex):
        self.index = index
        self.lowlink = lowlink
        self.onStack = onStack
        self.adjindex = adjindex

def strongconnect(v):
    global index
    global S
    global V
    global adj
    v.index = index
    v.lowlink = index
    index = index + 1
    v.onStack = True
    S.append(v)

    sccs = []

    a = adj[v.adjindex]
    for windex in a:
        w = V[windex]
        if w.index is -1:
            s = strongconnect(w)
            if s:
                sccs += s
            v.lowlink = min(v.lowlink, w.lowlink)
        elif w.onStack:
            v.lowlink = min(v.lowlink, w.index)

    scc = set()
    if v.lowlink == v.index:
        while True:
            w = S.pop()
            w.onStack = False
            scc.add(w.adjindex)
            if w == v:
                break

    if scc:
        sccs.append(scc)

    return sccs

def tarjan(n):
    global V
    V = [vertex(-1, -1, False, i) for i in range(n * 2)]

    sccs = []
    for v in V:
        if v.index is -1:
            s = strongconnect(v)
            if s:
                sccs += s
    return sccs

def isSatisfiable(clauses, n):
    global adj
    adj = [[] for i in range(n * 2)]
    for c in clauses:
        if c[0] > 0:
            i = (abs(c[0]) - 1) * 2 + 1
            if c[1] > 0:
                j = (abs(c[1]) - 1) * 2
                adj[i].append(j)
                adj[j + 1].append(i - 1)
            else:
                j = (abs(c[1]) - 1) * 2 + 1
                adj[i].append(j)
                adj[j - 1].append(i - 1)
        else:
            i = (abs(c[0]) - 1) * 2
            if c[1] > 0:
                j = (abs(c[1]) - 1) * 2
                adj[i].append(j)
                adj[j + 1].append(i + 1)
            else:
                j = (abs(c[1]) - 1) * 2 + 1
                adj[i].append(j)
                adj[j - 1].append(i + 1)

    sccs = tarjan(n)

    result = [-1 for i in range(n)]
    for scc in sccs:
        for i in scc:
            if result[i // 2] != -1:
                continue
            if i % 2:
                if i - 1 in scc:
                    return None
                result[i // 2] = 1
            else:
                if i + 1 in scc:
                    return None
                result[i // 2] = 0

    return result

# Arguments:
#   * `n` - the number of vertices.
#   * `edges` - list of edges, each edge is a tuple (u, v), 1 <= u, v <= n.
#   * `colors` - list consisting of `n` characters, each belonging to the set {'R', 'G', 'B'}.
# Return value: 
#   * If there exists a proper recoloring, return value is a list containing new colors, similar to the `colors` argument.
#   * Otherwise, return value is None.
def assign_new_colors(n, edges, colors):
    # Insert your code here.
    colors = colors[0]
    clauses = []

    # Must not be of the same color
    for i in range(n):
        if colors[i] == 'R':
            clauses.append([0 - (i * 3 + 1), 0 - (i * 3 + 1)])
        elif colors[i] == 'G':
            clauses.append([0 - (i * 3 + 2), 0 - (i * 3 + 2)])
        else: # colors[i] == 'B'
            clauses.append([0 - (i * 3 + 3), 0 - (i * 3 + 3)])

    # Must be in one of the rest two colors
    for i in range(n):
        if colors[i] == 'R':
            clauses.append([i * 3 + 2, i * 3 + 3])
            clauses.append([0 - (i * 3 + 2), 0 - (i * 3 + 3)])
        elif colors[i] == 'G':
            clauses.append([i * 3 + 1, i * 3 + 3])
            clauses.append([0 - (i * 3 + 1), 0 - (i * 3 + 3)])
        else: # colors[i] == 'B'
            clauses.append([i * 3 + 1, i * 3 + 2])
            clauses.append([0 - (i * 3 + 1), 0 - (i * 3 + 2)])

    # Must not be of the same color as adjacent vertices
    ad = [[] for i in range(n)]
    for e in edges:
        ad[e[0] - 1].append(e[1] - 1)
        ad[e[1] - 1].append(e[0] - 1)
    for i in range(n):
        for a in ad[i]:
            clauses.append([0 - (i * 3 + 1), 0 - (a * 3 + 1)])
            clauses.append([0 - (i * 3 + 2), 0 - (a * 3 + 2)])
            clauses.append([0 - (i * 3 + 3), 0 - (a * 3 + 3)])

    # Extra: each vertex can be of a single color at a time
    for i in range(n):
        clauses.append([0 - (1 + i * 3), 0 - (2 + i * 3)])
        clauses.append([0 - (1 + i * 3), 0 - (3 + i * 3)])
        clauses.append([0 - (2 + i * 3), 0 - (3 + i * 3)])

    result = isSatisfiable(clauses, n * 3)
    '''
    if n % 3 == 0:
        new_colors = []
        for i in range(n):
            new_colors[i].append("RGB"[i % 3])
        return new_colors
    '''
    if result != None:
        s = ''
        for i in range(len(result)):
            if result[i] == 0:
                s = s + "RGB"[i % 3]
        return s
    else:
        return None
    
def main():
    n, m = map(int, input().split())
    colors = input().split()
    edges = []
    for i in range(m):
        u, v = map(int, input().split())
        edges.append((u, v))
    new_colors = assign_new_colors(n, edges, colors)
    if new_colors is None:
        print("Impossible")
    else:
        print(''.join(new_colors))

#main()
threading.Thread(target=main).start()
