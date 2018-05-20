# python3
import sys


def SortCharacters(S):
  order = [0 for i in range(len(S))]
  d = {'$' : 0, 'A' : 1, 'C' : 2, 'G' : 3, 'T' : 4}
  #print(len(d))
  count = [0 for i in range(len(d))]
  for i in range(len(S)):
    count[d[S[i]]] += 1
  #print(count)
  for j in range(1, len(count)):
    count[j] = count[j] + count[j - 1]
  #print(count)
  for i in reversed(range(len(S))):
    c = S[i]
    count[d[c]] -= 1
    order[count[d[c]]] = i
  #print(order)
  return order

def ComputeCharClasses(S, order):
  clas = [0 for i in range(len(S))]
  clas[order[0]] = 0
  for i in range(1, len(S)):
    if S[order[i]] != S[order[i - 1]]:
      clas[order[i]] = clas[order[i - 1]] + 1
    else:
      clas[order[i]] = clas[order[i - 1]]
  return clas

def SortDoubled(S, L, order, clas):
  count = [0 for i in range(len(S))]
  newOrder = [0 for i in range(len(S))]
  for i in range(len(S)):
    count[clas[i]] += 1
  for j in range(1, len(S)):
    count[j] = count[j] + count[j - 1]
  for i in reversed(range(len(S))):
    start = (order[i] - L) % len(S) # NOTE: do I need to add len(S) in the brackets?
    cl = clas[start]
    count[cl] -= 1
    newOrder[count[cl]] = start
  return newOrder

def UpdateClasses(newOrder, clas, L):
  n = len(newOrder)
  newClass = [0 for i in range(n)]
  newClass[newOrder[0]] = 0
  for i in range(1, n):
    cur = newOrder[i]
    prev = newOrder[i - 1]
    mid = (cur + L) % n
    midPrev = (prev + L) % n
    if clas[cur] != clas[prev] or clas[mid] != clas[midPrev]:
      newClass[cur] = newClass[prev] + 1
    else:
      newClass[cur] = newClass[prev]
  return newClass

def build_suffix_array(text):
  """
  Build suffix array of the string text and
  return a list result of the same length as the text
  such that the value result[i] is the index (0-based)
  in text where the i-th lexicographically smallest
  suffix of text starts.
  """
  result = []
  # Implement this function yourself
  order = SortCharacters(text)
  #print(order)
  clas = ComputeCharClasses(text, order)
  #print(clas)
  L = 1
  while L < len(text):
    order = SortDoubled(text, L, order, clas)
    clas = UpdateClasses(order, clas, L)
    L *= 2
  return order
  #return result


if __name__ == '__main__':
  text = sys.stdin.readline().strip()
  print(" ".join(map(str, build_suffix_array(text))))
