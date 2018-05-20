# python3

import sys, threading
sys.setrecursionlimit(10**7) # max depth of recursion
threading.stack_size(2**25)  # new thread will get stack of such size

class node:
        def __init__(self, index):
                 self.index = index
                 self.children = []

        def add(self, node):
                 self.children.append(node)

class TreeHeight:
        def read(self):
                self.n = int(sys.stdin.readline())
                self.parent = list(map(int, sys.stdin.readline().split()))

        #def ch(self, i):
        #        maxn = n = 0
        #        for j in range(self.n):
        #                if self.parent[j] == i:
        #                        n = self.ch(j)
        #                        if n > maxn:
        #                                maxn = n
        #        return maxn + 1

        def ch(self, i):
                maxn = n = 0
                for j in range(len(self.nodes[i].children)):
                         n = self.ch(self.nodes[i].children[j])
                         if n > maxn:
                                 maxn = n
                return maxn + 1

        def compute_height(self):
                # Replace this code with a faster implementation
                self.nodes = []
                for i in range(self.n):
                        n = node(i)
                        self.nodes.append(n)
                for i in range(self.n):
                        p = self.parent[i]
                        if p != -1:
                                self.nodes[p].add(i)
                #for i in range(self.n):
                #        print("Node", i, "children:", self.nodes[i].children)
                #print("Root is", self.parent.index(-1))
                return self.ch(self.parent.index(-1))

                #maxHeight = 0
                #for vertex in range(self.n):
                #        height = 0
                #        i = vertex
                #        while i != -1:
                #                height += 1
                #                i = self.parent[i]
                #        maxHeight = max(maxHeight, height);
                #return maxHeight;

def main():
  tree = TreeHeight()
  tree.read()
  print(tree.compute_height())

threading.Thread(target=main).start()
