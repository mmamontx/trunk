# python3
import sys
from os.path import *


def strcmp(a, b):
  #print("Comparing", a, "and", b)
  #i, n = 0, 0
  #while i < min(len(a), len(b)):
  #  if a[i] == b[i]:
  #    n += 1
  #    i += 1
  #  else:
  #    break
  return len(commonprefix([a, b]))
  #return n

def build_suffix_tree(text):
  """
  Build a suffix tree of the string text and return a list
  with all of the labels of its edges (the corresponding 
  substrings of the text) in any order.
  """
  result = []
  # Implement this function yourself
  #text = text[:-1]
  #print("text is", text)
  tree = []
  tree.append([])
  counter = 0
  for i in range(len(text)):
    pat = text[i:]
    #print()
    #print(i, "pat is", pat)
    cn, flag = 0, 0
    head = 0
    while not flag:
      cnprev = cn
      #print("cn is now", cn)
      for j in range(len(tree[cn])):
        (pos, l, d) = tree[cn][j]
        if text[pos] == pat[0 + head]:
          #print("Match!")
          lab = text[pos:pos + l]
          n = strcmp(pat[head:], lab)
          #print("n is", n)
          if n == len(pat) == len(lab):
            flag = 1
          elif n == len(lab) and n != len(pat):
            if d > 0:
              cn = d
              head += len(lab)
              #print("Moved over", lab)
            else:
              #print("ACHTUNG!")
              pass # Undefined behavior
          else:
            #print("Common part is", pat[head:head + n])
            counter += 1
            tree[cn][j] = (pos, n, counter)
            cn = counter
            tree.append([])
            tree[cn].append((head + i + n, len(pat[head + n:]), 0))
            tree[cn].append((pos + n, len(lab[n:]), d))
            #print("First part is", pat[head + n:])
            #print("Second part is", lab[n:])
            flag = 1
          break
      if cn == cnprev and not flag:
        tree[cn].append((i + head, len(pat[head:]), 0))
        flag = 1
    #print(i, "tree is", tree)
  #tree[0].append((len(text), 0, 0))
  for i in range(len(tree)):
    for j in range(len(tree[i])):
      (pos, l, d) = tree[i][j]
      result.append(text[pos:pos + l])
  return result


if __name__ == '__main__':
  text = sys.stdin.readline().strip()
  result = build_suffix_tree(text)
  print("\n".join(result))
