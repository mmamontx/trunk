# python3
from sys import stdin
#from sys import maxsize
maxsize = 1000000000
  
class Equation:
    def __init__(self, a, b):
        self.a = a
        self.b = b

class Position:
    def __init__(self, column, row):
        self.column = column
        self.row = row

def SelectPivotElement(a, used_rows, used_columns):
    pivot_element = Position(0, 0)
    while used_rows[pivot_element.row]:
        pivot_element.row += 1
    while used_columns[pivot_element.column]:
        pivot_element.column += 1
    return pivot_element

def SwapLines(a, b, used_rows, pivot_element):
    pe = pivot_element
    if a[pe.row][pe.column] == 0:
        for i in range(pe.row + 1, len(a)):
            if a[i][pe.column] != 0:
                pivot_element.row = i
                break
    a[pivot_element.column], a[pivot_element.row] = a[pivot_element.row], a[pivot_element.column]
    b[pivot_element.column], b[pivot_element.row] = b[pivot_element.row], b[pivot_element.column]
    used_rows[pivot_element.column], used_rows[pivot_element.row] = used_rows[pivot_element.row], used_rows[pivot_element.column]
    pivot_element.row = pivot_element.column;

def scale(a, b, pivot_element):
    pe = pivot_element
    peval = a[pe.row][pe.column]
    if peval == 1:
        return

    for i in range(len(a[pe.row])):
        if i == pe.column:
            continue
        a[pe.row][i] /= peval
    b[pe.row] /= peval

def myabs(x):
    if x < 0:
        x = 0 - x
    return x

def zero(a, b, pivot_element):
    pe = pivot_element
    peval = a[pe.row][pe.column]

    for i in range(len(a)):
        if (i == pe.row) or (a[i][pe.column] == 0):
            continue
        for j in range(pe.column + 1, len(a)):
            a[i][j] -= a[pe.row][j] * a[i][pe.column]
            if myabs(a[i][j]) < 1e-12:
                a[i][j] = 0

        b[i] -= b[pe.row] * a[i][pe.column]
        if myabs(b[i]) < 1e-12:
            b[i] = 0

        a[i][pe.column] = 0

def ProcessPivotElement(a, b, pivot_element):
    scale(a, b, pivot_element)
    zero(a, b, pivot_element)

def MarkPivotElementUsed(pivot_element, used_rows, used_columns):
    used_rows[pivot_element.row] = True
    used_columns[pivot_element.column] = True

def SolveEquation(equation):
    a = equation.a
    b = equation.b
    size = len(a)

    used_columns = [False] * size
    used_rows = [False] * size
    for step in range(size):
        pivot_element = SelectPivotElement(a, used_rows, used_columns)
        SwapLines(a, b, used_rows, pivot_element)
        if a[pivot_element.row][pivot_element.column] == 0:
            return []
        ProcessPivotElement(a, b, pivot_element)
        MarkPivotElementUsed(pivot_element, used_rows, used_columns)
        '''
        print()
        for i in range(len(a)):
            print([float(a[i][j]) for j in range(len(a[i]))], b[i])
        '''
    return b

def find_subsets(so_far, rest, m, s):
    if not rest:
        if len(so_far) == m:
            s.append(set(so_far))
    else:
        s = find_subsets(so_far + [rest[0]], rest[1:], m, s)
        s = find_subsets(so_far, rest[1:], m, s)
    return s

def row(i, m):
  r = [0 for i in range(m)]
  r[i] = -1
  return r

def has_zero_column(A1):
  for j in range(len(A1[0])):
    flag = False
    for i in range(len(A1)):
      if A1[i][j] != 0:
        flag = True
    if not flag:
      return True
  return False

def satisfies(answers, A, b, c):
  s = sorted(answers, key=lambda a: a[0], reverse = True)
  '''
  print("sorted:")
  for i in range(len(s)):
    print(float(s[i][0]), [float(s[i][1][j]) for j in range(len(s[i][1]))], s[i][2])
  '''

  sat = []
  for i in range(len(s)):
    '''
    print()
    print(s[i])
    print()
    '''
    for j in range(len(A)):
      #print("Equation:", A[j], b[j])
      left = 0
      tainted = False
      for k in range(len(A[j])):
        left += s[i][1][k] * A[j][k]
      #print("left =", left, "right =", b[j])
      if left == 0:
        if b[j] < 0:
          #print("Satisfies NOT!")
          tainted = True
      elif left > 0:
        right = b[j] / left
        left = 1
        #print("*left =", left, "right =", right)
        #if left > right: # Should be <=
        if (left - right) > 1e-7: # Should be <=
          #print("Satisfies NOT!")
          tainted = True
      elif left < 0:
        right = b[j] / left
        left = 1
        #print("*left =", left, "right =", right)
        #if left < right: # Should be >=
        if (right - left) > 1e-7: # Should be >=
          #print("Satisfies NOT!")
          tainted = True
      if tainted:
        break
    if not tainted:
      sat.append(s[i])
    if len(sat) > 8: # Over-optimisation: provides a wrong answer if set too small
      break

  '''
  print("Satisfy:")
  for i in range(len(sat)):
    print(float(sat[i][0]), [float(sat[i][1][j]) for j in range(len(sat[i][1]))], sat[i][2])
  '''

  if sat:
    if sat[0][2] == True: # Check if there is a bounded solution that also maximizes the fuction
      for i in range(1, len(sat)):
        if sat[i][2] == False and sat[i][0] == sat[0][0]:
          return [0, sat[i][1], sat[i][2]]
    answer = [0, sat[0][1], sat[0][2]]
  else:
    answer = [-1, [], False]
  return answer

def solve_diet_problem(n, m, A, b, c):  
  # Write your code here
  onesrow = [1 for i in range(m)]
  A.append(onesrow)
  b.append(maxsize)
  n += 1

  '''
  for i in range(len(A)):
    print(A[i], b[i])
  print(c)
  '''
  s = find_subsets([], range(n + m), m, [])
  #print(s)
  answers = []
  for i in range(len(s)):
    A1 = []
    b1 = []
    for j in s[i]:
      if j < n:
        A1.append(list(A[j]))
        b1.append(b[j])
      else:
        r = row(j - n, m)
        A1.append(list(r))
        b1.append(0)
        A.append(list(r))
        b.append(0)

    '''
    print()
    for j in range(len(A1)):
      print(A1[j], b1[j])
    '''

    if has_zero_column(A1):
      continue

    e = Equation(A1, b1)
    roots = SolveEquation(e)
    #print("roots:", [float(roots[k]) for k in range(len(roots))])
    if roots == []:
      continue

    valid = True
    for root in roots:
      if root < 0:
        valid = False
        break
    if not valid:
      continue

    f = 0
    for j in range(len(roots)):
      f += roots[j] * c[j]
    answers.append([f, roots, True if (n - 1) in s[i] else False])

  answer = satisfies(answers, A, b, c)
  if answer[2] == True:
    return [1, []]
  return [answer[0], answer[1]]

n, m = list(map(int, stdin.readline().split()))
A = []
for i in range(n):
  A += [list(map(int, stdin.readline().split()))]
b = list(map(int, stdin.readline().split()))
c = list(map(int, stdin.readline().split()))

anst, ansx = solve_diet_problem(n, m, A, b, c)

if anst == -1:
  print("No solution")
if anst == 0:  
  print("Bounded solution")
  print(' '.join(list(map(lambda x : '%.18f' % x, ansx))))
if anst == 1:
  print("Infinity")
    
