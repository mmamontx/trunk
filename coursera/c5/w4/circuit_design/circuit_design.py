# python3

import sys
import threading

sys.setrecursionlimit(10**6) # max depth of recursion
threading.stack_size(2**26)  # new thread will get stack of such size

n, m = map(int, input().split())
clauses = [ list(map(int, input().split())) for i in range(m) ]

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

def tarjan():
    global V
    V = [vertex(-1, -1, False, i) for i in range(n * 2)]

    sccs = []
    for v in V:
        if v.index is -1:
            s = strongconnect(v)
            if s:
                sccs += s
    return sccs

# This solution tries all possible 2^n variable assignments.
# It is too slow to pass the problem.
# Implement a more efficient algorithm here.
def isSatisfiable():
    '''
    for mask in range(1<<n):
        result = [ (mask >> i) & 1 for i in range(n) ]
        formulaIsSatisfied = True
        for clause in clauses:
            clauseIsSatisfied = False
            if result[abs(clause[0]) - 1] == (clause[0] < 0):
                clauseIsSatisfied = True
            if result[abs(clause[1]) - 1] == (clause[1] < 0):
                clauseIsSatisfied = True
            if not clauseIsSatisfied:
                formulaIsSatisfied = False
                break
        if formulaIsSatisfied:
            return result
    '''
    global adj
    adj = [[] for i in range(n * 2)]
    for c in clauses:
        if c[0] > 0:
            i = (abs(c[0]) - 1) * 2 + 1
            if c[1] > 0:
                j = (abs(c[1]) - 1) * 2
                adj[i].append(j)
                adj[j + 1].append(i - 1) # Skew-symmetry
            else:
                j = (abs(c[1]) - 1) * 2 + 1
                adj[i].append(j)
                adj[j - 1].append(i - 1) # Skew-symmetry
        else:
            i = (abs(c[0]) - 1) * 2
            if c[1] > 0:
                j = (abs(c[1]) - 1) * 2
                adj[i].append(j)
                adj[j + 1].append(i + 1) # Skew-symmetry
            else:
                j = (abs(c[1]) - 1) * 2 + 1
                adj[i].append(j)
                adj[j - 1].append(i + 1) # Skew-symmetry

    sccs = tarjan()
    '''
    for scc in sccs:
        print("An another SCC:")
        for i in scc:
            print(i)
    '''
    #for scc in sccs:
    #    for i in range(0, n * 2, 2):
    #        if (i in scc) and (i + 1 in scc):
    #            return None

    result = [-1 for i in range(n)]
    #for scc in reversed(sccs):
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

    #return None
    return result

def main():
    result = isSatisfiable()
    if result is None:
        print("UNSATISFIABLE")
    else:
        print("SATISFIABLE");
        print(" ".join(str(-i-1 if result[i] else i+1) for i in range(n)))

threading.Thread(target=main).start()
