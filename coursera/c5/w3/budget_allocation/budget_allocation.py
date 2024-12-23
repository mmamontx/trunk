# python3

from sys import stdin

n, m = list(map(int, stdin.readline().split()))
A = []
for i in range(n):
  A += [list(map(int, stdin.readline().split()))]
b = list(map(int, stdin.readline().split()))

def print_clause(v, t):
    m = len(v)
    for i in range(len(t)):
        if t[i]:
            continue # We're interested only in false ones
        if i & 1:
            print(0 - v[0][1], end=' ')
        else:
            print(v[0][1], end=' ')
        if m > 1:
            if i & 2:
                print(0 - v[1][1], end=' ')
            else:
                print(v[1][1], end=' ')
        if m > 2:
            if i & 4:
                print(0 - v[2][1], end=' ')
            else:
                print(v[2][1], end=' ')
        print(0)

def table(v, b):
    m = len(v)
    #print("\ntable()")
    #print("v:", v)
    #print("b:", b)
    if m == 0:
        return []
    t = [-1 for i in range(2 ** m)]

    t[0] = 1 if 0 <= b else 0
    t[1] = 1 if v[0][0] <= b else 0
    if m > 1:
        t[2] = 1 if v[1][0] <= b else 0
        t[3] = 1 if (v[0][0] + v[1][0]) <= b else 0
    if m > 2:
        t[4] = 1 if v[2][0] <= b else 0
        t[5] = 1 if (v[0][0] + v[2][0]) <= b else 0
        t[6] = 1 if (v[1][0] + v[2][0]) <= b else 0
        t[7] = 1 if (v[0][0] + v[1][0] + v[2][0]) <= b else 0
    return t

# This solution prints a simple satisfiable formula
# and passes about half of the tests.
# Change this function to solve the problem.
def printEquisatisfiableSatFormula():
    '''
    print("3 2")
    print("1 2 0")
    print("-1 -2 0")
    print("1 -2 0")
    '''
    #print("n =", n, "m =", m)
    #for i in range(len(A)):
    #    print(A[i], b[i])

    numclauses = 0
    tuples = []
    for i in range(len(A)):
        v = []
        for j in range(len(A[i])):
            if A[i][j] != 0:
                v.append((A[i][j], j + 1)) # The latter is the variable index
        t = table(v, b[i]) # The first one is the number of variables in the constraint
        #print("t:", t)
        if len(t) > 0:
            tuples.append((v, t))
            numclauses += t.count(0)

    if numclauses > 0:
        print(numclauses, m)
        for tupl in tuples:
            print_clause(tupl[0], tupl[1])
    else:
        print(1, 1)
        print(1, -1, 0)

printEquisatisfiableSatFormula()
