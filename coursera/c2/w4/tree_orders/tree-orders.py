# python3

import sys, threading
sys.setrecursionlimit(10**6) # max depth of recursion
threading.stack_size(2**25)  # new thread will get stack of such size

class TreeOrders:
  def read(self):
    self.n = int(sys.stdin.readline())
    self.key = [0 for i in range(self.n)]
    self.left = [0 for i in range(self.n)]
    self.right = [0 for i in range(self.n)]
    for i in range(self.n):
      [a, b, c] = map(int, sys.stdin.readline().split())
      self.key[i] = a
      self.left[i] = b
      self.right[i] = c

  def inord(self, i):
    #self.result = []
    if self.left[i] != -1:
       #self.result += self.inord(self.left[i])
       self.inord(self.left[i])
    self.result.append(self.key[i])
    if self.right[i] != -1:
       #self.result += self.inord(self.right[i])
       self.inord(self.right[i])
    #return self.result

  def inOrder(self):
    self.result = []
    # Finish the implementation
    # You may need to add a new recursive method to do that
    #self.result += self.inord(0)
    self.inord(0)
                
    return self.result

  def preord(self, i):
    #self.result = []
    self.result.append(self.key[i])
    if self.left[i] != -1:
       #self.result += self.preord(self.left[i])
       self.preord(self.left[i])
    if self.right[i] != -1:
       #self.result += self.preord(self.right[i])
       self.preord(self.right[i])
    #return self.result

  def preOrder(self):
    self.result = []
    # Finish the implementation
    # You may need to add a new recursive method to do that
    #self.result += self.preord(0)
    self.preord(0)
                
    return self.result

  def postord(self, i):
    #self.result = []
    if self.left[i] != -1:
       #self.result += self.postord(self.left[i])
       self.postord(self.left[i])
    if self.right[i] != -1:
       #self.result += self.postord(self.right[i])
       self.postord(self.right[i])
    self.result.append(self.key[i])
    #return self.result

  def postOrder(self):
    self.result = []
    # Finish the implementation
    # You may need to add a new recursive method to do that
    #self.result += self.postord(0)
    self.postord(0)
                
    return self.result

def main():
	tree = TreeOrders()
	tree.read()
	print(" ".join(str(x) for x in tree.inOrder()))
	print(" ".join(str(x) for x in tree.preOrder()))
	print(" ".join(str(x) for x in tree.postOrder()))

threading.Thread(target=main).start()
