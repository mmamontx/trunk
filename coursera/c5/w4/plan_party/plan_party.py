#uses python3

import sys
import threading

# This code is used to avoid stack overflow issues
sys.setrecursionlimit(10**6) # max depth of recursion
threading.stack_size(2**26)  # new thread will get stack of such size


class Vertex:
    def __init__(self, weight):
        self.weight = weight
        self.children = []


def ReadTree():
    size = int(input())
    global tree
    tree = [Vertex(w) for w in map(int, input().split())]
    for i in range(1, size):
        a, b = list(map(int, input().split()))
        tree[a - 1].children.append(b - 1)
        tree[b - 1].children.append(a - 1)
    return tree


tree = []
D = []

def FunParty(v, parent):
    global D
    global tree
    if D[v] == -1:
        if (len(tree[v].children) == 1) and (tree[v].children[0] == parent):
            D[v] = tree[v].weight
        else:
            m1 = tree[v].weight
            for u in tree[v].children:
                if u == parent:
                    continue
                for w in tree[u].children:
                    if w == v:
                        continue
                    m1 = m1 + FunParty(w, u)
            m0 = 0
            for u in tree[v].children:
                if u == parent:
                    continue
                m0 = m0 + FunParty(u, v)
            D[v] = max(m1, m0)
    return D[v]


def dfs(tree, vertex, parent):
    for child in tree[vertex].children:
        if child != parent:
            dfs(tree, child, vertex)

    # This is a template function for processing a tree using depth-first search.
    # Write your code here.
    # You may need to add more parameters to this function for child processing.
    return FunParty(vertex, parent)


def MaxWeightIndependentTreeSubset(tree):
    size = len(tree)
    if size == 0:
        return 0
    global D
    D = [-1 for i in range(size)]
    #dfs(tree, 0, -1)
    # You must decide what to return.
    #return 0
    return dfs(tree, 0, -1)


def main():
    tree = ReadTree();
    weight = MaxWeightIndependentTreeSubset(tree);
    print(weight)


# This is to avoid stack overflow issues
threading.Thread(target=main).start()
