# python3
from sys import stdin
import numpy as np

PREC = np.array(1e-9, dtype=np.float64)

class Position:
    def __init__(self, column, row):
        self.column = column
        self.row = row

def scale(a, b, pe): # pe stands for pivot element
    peval = a[pe.row][pe.column]
    #if peval == 1:
    if np.absolute(peval - np.array(1, dtype=np.float64)) < PREC:
        return

    a[pe.row] = np.divide(a[pe.row], peval)
    b[pe.row] = np.divide(b[pe.row], peval)

def zero(a, b, pe):
    for i in range(len(a)):
        if i == pe.row:
            continue
        if np.absolute(a[i][pe.column]) < PREC:
            continue

        rel = a[i][pe.column] # Since pivot element is always 1
        a[i] -= a[pe.row] * rel
        b[i] -= b[pe.row] * rel

def ProcessPivotElement(a, b, pivot_element):
    scale(a, b, pivot_element)
    zero(a, b, pivot_element)

def init_table(A, b, c, m):
  for i in range(len(A)):
    A[i] = np.array(A[i], dtype=np.float64)
  b = np.array(b, dtype=np.float64)
  c = np.array(c, dtype=np.float64)

  # 1. Replace by a difference between two nonnegative variables
  # Omitted because all equations are constrained (<=)

  # 2. Change inequalities to equalities, introduce slack vars
  # All vars are slack since all equations are (<=)

  basicv = [] # 0-based indices of basic variables
  slackv = [] # 0-based indices of slack variables
  for i in range(len(A)):
    for j in range(len(A)):
      if j == i:
        A[i] = np.append(A[i], np.array(1.0, dtype=np.float64))
        basicv.append(len(A[i]) - 1)
        if b[i] >= 0:
          slackv.append(len(A[i]) - 1)
      else:
        A[i] = np.append(A[i], np.array(0.0, dtype=np.float64))

  # 3. Multiply equations with a negative righthand side by -1
  artlist = [] # Store indices of equations where art var should be added
  for i in range(len(A)):
    if b[i] < 0:
      b[i] = b[i] * np.array(-1.0, dtype=np.float64)
      A[i] = A[i] * np.array(-1.0, dtype=np.float64)
      artlist.append(i)

  # 4. Add artificial variables
  artv = []
  artsurpv = {}
  for i in artlist:
    for j in range(len(A)):
      if j == i:
        A[j] = np.append(A[j], np.array(1.0, dtype=np.float64))
        artvarindex = len(A[i]) - 1
        basicv[j] = artvarindex
        for k in range(m, len(A[i])): # Find corresponding surplus variable
          if A[i][k] != 0:
            artsurpv[artvarindex] = k
            break
      else:
        #A[j] = np.append(A[j], np.array(1.0, dtype=np.float64))
        A[j] = np.append(A[j], np.array(0.0, dtype=np.float64))
    artv.append(artvarindex)

  # Add z-equation
  while len(c) < len(A[0]):
    c = np.append(c, np.array(0.0, dtype=np.float64))
  A.append(c)
  b = np.append(b, np.array(0.0, dtype=np.float64))

  # Add w-equation (Phase I objective) if artificial variables were added
  if len(artv) > 0:
    w = np.array([0.0 for i in range(len(A[0]))], dtype=np.float64)
    Bw = np.array(0.0, dtype=np.float64)

    for i in artlist:
      #for j in range(len(basicv)):
      for j in range(artv[0]): # FIXME
        w[j] += A[i][j]
      Bw += b[i]

    A.append(w)
    b = np.append(b, Bw)

  return A, b, basicv, artv, slackv, artsurpv

def optimality_test(obj, basicv):
  for i in range(len(obj)):
    if obj[i] > PREC:
      return False
  return True

def select_xs(obj, basicv):
  # Bland's rule #1 is not used
  objs = []
  objvals = []
  for i in range(len(obj)):
    if obj[i] > PREC and i not in basicv:
      #return i # Bland's rule
      objs.append(i)
      objvals.append(obj[i])
  maxval = max(objvals)
  for i in reversed(range(len(objs))):
    if obj[objs[i]] == maxval:
      return objs[i]

def unboundedness_test(A, basicv, xs):
  for i in range(len(basicv)):
    if A[i][xs] > PREC:
      return True
  return False

def select_pivot_row(A, b, basicv, xs):
  # Bland's rule #2
  ratios = []
  for i in range(len(basicv)):
    #if A[i][xs] > PREC or (A[i][xs] < 0 and (np.absolute(b[i]) < PREC)):
    if A[i][xs] > PREC: # According to the rule coef should be greater
      ratios.append((i, np.divide(b[i], A[i][xs])))
  rlist = [r[1] for r in ratios]
  minratio = min(rlist)
  minratios = [] # Indices of variables with minimum ratio
  for r in ratios:
    if r[1] == minratio:
      minratios.append(r[0])
  if len(minratios) > 1:
    minbasic = basicv[minratios[0]]
    for i in range(1, len(minratios)):
      basic = basicv[minratios[i]]
      if basic < minbasic:
        minbasic = basic
    for i in minratios:
      if basicv[i] == minbasic:
        return i
    #print("Achtung!")
  else:
    return minratios[0]

def artv_in_basicv(artv, basicv, ex_artv):
  for i in artv:
    if not excluded(ex_artv, i):
      if i in basicv:
        return True
  return False

def excluded(ex_artv, i):
  for j in range(len(ex_artv)):
    if ex_artv[j][0] == i:
       return True
  return False

def simplex_method(A, b, c, m):
  A, b, basicv, artv, slackv, artsurpv = init_table(A, b, c, m)

  #print("Artificial variables:", artv)

  if len(artv) > 0:
    phase = 1
  else:
    phase = 2

  ex_artv = []
  while True:
    #print()
    #for i in range(len(A)):
    #  print(A[i], b[i])
    #print("basicv:", basicv)

    if optimality_test(A[-1], basicv) == True:
      if phase == 1:
        w = np.array(0.0, dtype=np.float64)
        for i in artv:
          if not excluded(ex_artv, i):
            if i in basicv:
              w -= b[basicv.index(i)]
        #print("w:", w)

        if w < np.array(-1e-7, dtype=np.float64):
          return -1, []

        if artv_in_basicv(artv, basicv, ex_artv) == True:
          for i in artv:
            if i in basicv:
              if excluded(ex_artv, i):
                continue
              index = basicv.index(i)
              nonzeros = []
              for j in range(len(basicv)):
                if np.absolute(A[index][j]) > PREC:
                  nonzeros.append(j)
              if len(nonzeros) == 0:
                # Maintain xi in the basis throughout phase 2
                #print("Achtung zero!")
                #print("A[index][i] =", A[index][i])
                A[index][i] = np.array(1.0, dtype=np.float64)
                ex_artv.append((i, index))
              else:
                pos = artsurpv[i]
                #print("Achtung pivot:", index, pos)

                if pos in basicv:
                  pass # Error

                A[index][pos] -= A[index][i]
                A[index][i] -= A[index][i]

                basicv[index] = pos
                ProcessPivotElement(A, b, Position(pos, index))
                break
          continue
        else:
          for i in range(len(A)):
            for j in artv:
              if not excluded(ex_artv, j):
                A[i] = np.delete(A[i], -1)

          del A[-1]
          phase = 2
          continue
      else: # phase = 2
        break

    # Select variable xs to introduce into the basis
    xs = select_xs(A[-1], basicv)
    #print("xs:", xs)

    if unboundedness_test(A, basicv, xs) == False:
      return 1, []

    r = select_pivot_row(A, b, basicv, xs)
    #print("r:", r)

    basicv[r] = xs
    ProcessPivotElement(A, b, Position(xs, r))

  #print("basicv =", basicv)
  #print("artv =", artv)
  #print("m =", m)
  #for i in range(len(basicv)):
  #  print(i, A[i], b[i])

  roots = [0 for i in range(m)]

  for i in range(m):
    if i in basicv:
      bi = basicv.index(i)
      roots[i] = b[bi]

  for i in basicv:
    if i in artv:
      #print("i:", i)
      #print("index:", basicv.index(i))
      #print("b:", b[basicv.index(i)])
      index = basicv.index(i)
      for j in range(m):
        if A[index][j] != 0:
          roots[j] = b[index]
          break

  '''
  for i in range(len(ex_artv)):
    con = ex_artv[i][1]
    for j in range(m):
      if A[con][j] != 0:
        roots[j] = b[con]
        break
  '''

  return 0, roots

def allocate_ads(n, m, A, b, c):  
  # Write your code here
  #print("n =", n, "m =", m)
  #for i in range(len(A)):
  #  print(A[i], b[i])
  #print(c, "(objective)")
  ans, roots = simplex_method(A, b, c, m)
  return [ans, roots]
  #return [0, [0] * m]

n, m = list(map(int, stdin.readline().split()))
A = []
for i in range(n):
  A += [list(map(int, stdin.readline().split()))]
b = list(map(int, stdin.readline().split()))
c = list(map(int, stdin.readline().split()))

anst, ansx = allocate_ads(n, m, A, b, c)

if anst == -1:
  print("No solution")
if anst == 0:  
  print("Bounded solution")
  print(' '.join(list(map(lambda x : '%.18f' % x, ansx))))
if anst == 1:
  print("Infinity")
    
