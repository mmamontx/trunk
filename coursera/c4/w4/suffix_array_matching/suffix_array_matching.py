# python3
import sys
import random

sys.setrecursionlimit(10**6)

def SortCharacters(S):
  order = [0 for i in range(len(S))]
  d = {'$' : 0, 'A' : 1, 'C' : 2, 'G' : 3, 'T' : 4}
  count = [0 for i in range(len(d))]
  for i in range(len(S)):
    count[d[S[i]]] += 1
  for j in range(1, len(count)):
    count[j] = count[j] + count[j - 1]
  for i in reversed(range(len(S))):
    c = S[i]
    count[d[c]] -= 1
    order[count[d[c]]] = i
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
    start = (order[i] - L) % len(S)
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
  order = SortCharacters(text)
  clas = ComputeCharClasses(text, order)
  L = 1
  while L < len(text):
    order = SortDoubled(text, L, order, clas)
    clas = UpdateClasses(order, clas, L)
    L *= 2
  return order

def cmp(s1, s2):
    minlen = min(len(s1), len(s2))
    for i in range(minlen):
        if s1[i] > s2[i]:
            return 1
        elif s1[i] < s2[i]:
            return -1
        else:
            pass

    if len(s1) > len(s2):
        return 1
    elif len(s1) < len(s2):
        return -1
    else:
        return 0

def PatternMatchingWithSuffixArray(text, pattern, order):
    #print("PatternMatchingWithSuffixArray() pattern =", pattern)
    minIndex = 0
    maxIndex = len(text) - 1
    while minIndex < maxIndex:
        midIndex = (minIndex + maxIndex) // 2
        #print("minIndex =", minIndex, "maxIndex =", maxIndex, "midIndex =", midIndex)
        if cmp(pattern, text[order[midIndex]:order[midIndex] + len(pattern)]) > 0:
            #print(pattern, "is greater than", text[order[midIndex]:order[midIndex] + len(pattern)])
            minIndex = midIndex + 1
        else:
            #print(pattern, "is less than", text[order[midIndex]:order[midIndex] + len(pattern)])
            maxIndex = midIndex
    start = minIndex
    maxIndex = len(text) - 1
    while minIndex < maxIndex:
        midIndex = (minIndex + maxIndex) // 2
        if cmp(pattern, text[order[midIndex]:order[midIndex] + len(pattern)]) < 0:
            maxIndex = midIndex
        else:
            minIndex = midIndex + 1
    end = maxIndex
    #print("start", start)
    #print("end", end)
    return (start, end)

def findsub(s, p):
    occs = set()
    start = 0
    while True:
        pos = s.find(p, start)
        if pos != -1:
            occs.add(pos)
            start += 1
        else:
            return occs

def genstr(n):
    s = ''
    for i in range(n):
        r = random.randint(0, 3)
        if r == 0:
            s += 'A'
        elif r == 1:
            s += 'C'
        elif r == 2:
            s += 'G'
        elif r == 3:
            s += 'T'
    return s

def find_occurrences(text, patterns):
    #occs = set()

    #// write your code here
    #while True:
    if True:
        occs = set()
        '''
        text = genstr(10)
        patterns = []
        for i in range(5):
            p = genstr(random.randint(1, 5))
            patterns.append(p)
        testset = set()
        for i in range(5):
            t = findsub(text, patterns[i])
            testset |= t
        '''

        order = build_suffix_array(text + '$')
        #print("order is", order)
        order.pop(0)
        #for i in range(len(order)):
        #    print(i, text[order[i]:])
        for i in range(len(patterns)):
            (start, end) = PatternMatchingWithSuffixArray(text, patterns[i], order)
            while start <= end and text[order[start]:order[start] + len(patterns[i])] == patterns[i]:
                occs.add(order[start])
                start += 1

        '''
        print("0123456789")
        print(text)
        print(patterns)
        print(testset)
        print(occs)
        if occs == testset:
            print("OK!")
        else:
            print("FAILURE!")
            return occs
        '''

    return occs

if __name__ == '__main__':
    text = sys.stdin.readline().strip()
    pattern_count = int(sys.stdin.readline().strip())
    patterns = sys.stdin.readline().strip().split()
    occs = find_occurrences(text, patterns)
    print(" ".join(map(str, occs)))
