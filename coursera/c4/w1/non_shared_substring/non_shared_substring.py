# python3
import sys
from os.path import *

sys.setrecursionlimit(10**6)

def strcmp(a, b):
  if not a or not b: return 0

  if a <= b:
    for i in range(len(a)):
      if a[i] != b[i]:
        return len(a[:i])
    return len(a)
  else:
    for i in range(len(b)):
      if b[i] != a[i]:
        return len(b[:i])
    return len(b)

def build_suffix_tree(text):
  result = []
  tree = []
  tree.append([])
  counter = 0
  for i in range(len(text)):
    pat = text[i:]
    cn, flag = 0, 0
    head = 0
    while not flag:
      cnprev = cn
      for j in range(len(tree[cn])):
        (pos, l, d) = tree[cn][j]
        if text[pos] == pat[0 + head]:
          lab = text[pos:pos + l]
          n = strcmp(pat[head:], lab)
          if n == len(pat) == len(lab):
            flag = 1
          elif n == len(lab) and n != len(pat):
            if d > 0:
              cn = d
              head += len(lab)
            else:
              pass # Undefined behavior
          else:
            counter += 1
            tree[cn][j] = (pos, n, counter)
            cn = counter
            tree.append([])
            tree[cn].append((head + i + n, len(pat[head + n:]), 0))
            tree[cn].append((pos + n, len(lab[n:]), d))
            flag = 1
          break
      if cn == cnprev and not flag:
        tree[cn].append((i + head, len(pat[head:]), 0))
        flag = 1
  #for i in range(len(tree)):
  #  for j in range(len(tree[i])):
  #    (pos, l, d) = tree[i][j]
  #    result.append(text[pos:pos + l])
  #return result
  return tree

def tl_find(cn, path):
    global text
    global tree
    tl = []
    tainted = False
    for i in range(len(tree[cn])):
        (pos, l, d) = tree[cn][i]
        ptn = text[pos:pos + l]
        if ptn[0] == '#':
            #print(cn, "Got a type L leaf with diez:", ptn)
            tl.append(path)
        elif '#' in ptn:
            #print(cn, "Got a type L leaf:", ptn)
            tl.append(path + ptn[0])
        elif ptn[-1] == '$':
            #print(cn, "Got a type R leaf:", ptn)
            tainted = True
        elif d > 0:
            #print(cn, "Got something:", ptn)
            (tl_child, tainted_child) = tl_find(d, path + ptn)
            if tl_child:
                tl += tl_child
            if tainted_child:
                tainted = True
            #print(cn, "Got from something:", tl_child)
        else:
            print("Oops!")
            exit(-1)

    if tainted and (path in tl):
        #print(cn, "REMOVING", path, "'cos it's tainted!")
        tl.remove(path)
    if not tainted:
        tl.append(path)
    #print(cn, "Returning tl that is:", tl)
    return (tl, tainted)

text = ''
tree = {}

def solve (p, q):
	#result = p
	#return result
    global text
    global tree
    text = p + '#' + q + '$'
    tree = build_suffix_tree(text)
    tl = []
    for i in range(len(tree[0])):
        (pos, l, d) = tree[0][i]
        ptn = text[pos:pos + l]
        if ptn[0] == '#': # Don't check the top one
            pass
        elif '#' in ptn:
            tl.append(ptn[0])
        elif ptn[-1] == '$':
            pass
        elif d > 0:
            (tl_child, tainted_child) = tl_find(d, '' + ptn)
            if tl_child:
                tl += tl_child
            if tainted_child:
                tainted = True
        else:
            print("Oops!")
            exit(-1)
    tl.sort(key = len)
    '''
    print(tree)
    for i in range(len(tree)):
        print(i, tree[i])
    print(tl)
    print(p)
    print(q)
    print(text)
    '''
    return tl[0]

p = sys.stdin.readline ().strip ()
q = sys.stdin.readline ().strip ()

ans = solve (p, q)

sys.stdout.write (ans + '\n')
