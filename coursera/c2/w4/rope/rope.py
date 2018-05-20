# python3

import sys

# Splay tree implementation

# Vertex of a splay tree
class Vertex:
  #def __init__(self, key, sum, left, right, parent):
  def __init__(self, key, ch, left, right, parent):
    #(self.key, self.sum, self.left, self.right, self.parent) = (key, sum, left, right, parent)
    (self.key, self.ch, self.left, self.right, self.parent) = (key, ch, left, right, parent)

def update(v):
  if v == None:
    return
  #v.sum = v.key + (v.left.sum if v.left != None else 0) + (v.right.sum if v.right != None else 0)
  v.key = 1 + (v.left.key if v.left != None else 0) + (v.right.key if v.right != None else 0)
  if v.left != None:
    v.left.parent = v
  if v.right != None:
    v.right.parent = v

def smallRotation(v):
  parent = v.parent
  if parent == None:
    return
  grandparent = v.parent.parent
  if parent.left == v:
    m = v.right
    v.right = parent
    parent.left = m
  else:
    m = v.left
    v.left = parent
    parent.right = m
  update(parent)
  update(v)
  v.parent = grandparent
  if grandparent != None:
    if grandparent.left == parent:
      grandparent.left = v
    else: 
      grandparent.right = v

def bigRotation(v):
  if v.parent.left == v and v.parent.parent.left == v.parent:
    # Zig-zig
    smallRotation(v.parent)
    smallRotation(v)
  elif v.parent.right == v and v.parent.parent.right == v.parent:
    # Zig-zig
    smallRotation(v.parent)
    smallRotation(v)    
  else: 
    # Zig-zag
    smallRotation(v);
    smallRotation(v);

# Makes splay of the given vertex and makes
# it the new root.
def splay(v):
  if v == None:
    return None
  while v.parent != None:
    if v.parent.parent == None:
      smallRotation(v)
      break
    bigRotation(v)
  return v

# Searches for the given key in the tree with the given root
# and calls splay for the deepest visited node after that.
# Returns pair of the result and the new root.
# If found, result is a pointer to the node with the given key.
# Otherwise, result is a pointer to the node with the smallest
# bigger key (next value in the order).
# If the key is bigger than all keys in the tree,
# then result is None.
def find(root, key): 
  v = root
  last = root
  next = None
  base = 0
  while v != None:
    if (v.key - 1 + base) >= key and (next == None or v.key < next.key):
      next = v    
    last = v
    #if key >= (v.key - 1 + base) or base == key:
    if key > (v.key - 1 + base) or ((v.left.key if v.left != None else 0) + base) == key:
      #print("Breaking!")
      break
    if v.left != None and (v.left.key - 1 + base) >= key:
        #print(v.left.key - 1 + base, ">=", key, "so going left...")
        v = v.left
    else:
      base += (v.left.key if v.left != None else 0) + 1
      v = v.right
    #if v.key >= key and (next == None or v.key < next.key):
    #  next = v    
    #last = v
    #if v.key == key:
    #  break    
    #if v.key < key:
    #  v = v.right
    #else: 
    #  v = v.left      
  root = splay(last)
  return (next, root)

def split(root, key):  
  #print("find() key =", key)
  (result, root) = find(root, key)  
  #print("!find()")
  if result == None:    
    return (root, None)  
  right = splay(result)
  left = right.left
  right.left = None
  if left != None:
    left.parent = None
  update(left)
  update(right)
  return (left, right)

  
def merge(left, right):
  if left == None:
    return right
  if right == None:
    return left
  while right.left != None:
    right = right.left
  right = splay(right)
  right.left = left
  update(right)
  return right

  
# Code that uses splay tree to solve the problem
                                    
root = None

def inord(x, how):
  if x == None:
    return

  inord(x.left, how)
  if how == 0:
    print(x.ch, end='')
  else:
    print(x.ch, x.key, end=' ')
    if x.left != None:
      print("left is", x.left.ch, end=' ')
    if x.right != None:
      print("right is", x.right.ch, end=' ')
    print()
  inord(x.right, how)

def inord_plus(x, val):
  if x == None or val == 0:
    return

  inord_plus(x.left, val)
  x.key += val
  inord_plus(x.right, val)

#def insert(x):
def insert(ch):
  global root
  #(left, right) = split(root, x)
  x = (root.key if root != None else 0) # This way 'x' will always be a next possible index
  (left, right) = split(root, x)
  new_vertex = None
  #if right == None or right.key != x:
  if right == None or (right.key - 1) != x:
    #new_vertex = Vertex(x, x, None, None, None)  
    new_vertex = Vertex(x + 1, ch, None, None, None)  
  root = merge(merge(left, new_vertex), right)
  
def cut(i, j):
	global root
	#print("cut()")
	(left, offcut) = split(root, i)
	#print("left:"); inord(left, 1)
	#(offcut, right) = split(offcut, j + 1)
	(offcut, right) = split(offcut, j - i + 1)
	#print("offcut:"); inord(offcut, 1)
	#print("right:"); inord(right, 1)
	#inord_plus(right, -(j - i + 1))
	root = merge(left, right)
	return offcut

#def paste(offcut, k, length):
def paste(offcut, k):
	global root
	#print("paste()")
	(left, right) = split(root, k)
	#inord_plus(right, length)
	root = merge(left, offcut)
	root = merge(root, right)

class Rope:
	def __init__(self, s):
		self.s = s
		for i in range(len(s)):
			insert(s[i])
			#insert(i, s[i])
	def inord(self, x):
		s = []
		while True:
			if x is not None:
				s.append(x)
				x = x.left
			else:
				if (len(s) > 0):
					x = s.pop()
					self.s += x.ch
					x = x.right
				else:
					break
	def result(self):
		self.s = ''
		self.inord(root)
		return self.s
	def process(self, i, j, k):
                # Write your code here
		#print(i, j, k)
		#global root
		#inord(root, 1)
		offcut = cut(i, j)
		#inord_plus(offcut, k - i)
		#paste(offcut, k, j - i + 1)
		paste(offcut, k)
		#pass
                

rope = Rope(sys.stdin.readline().strip())
q = int(sys.stdin.readline())
for _ in range(q):
	i, j, k = map(int, sys.stdin.readline().strip().split())
	rope.process(i, j, k)
print(rope.result())
