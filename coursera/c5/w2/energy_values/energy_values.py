# python3

EPS = 1e-6
PRECISION = 20

class Equation:
    def __init__(self, a, b):
        self.a = a
        self.b = b

class Position:
    def __init__(self, column, row):
        self.column = column
        self.row = row

def ReadEquation():
    size = int(input())
    a = []
    b = []
    for row in range(size):
        line = list(map(float, input().split()))
        a.append(line[:size])
        b.append(line[size])
    return Equation(a, b)

def SelectPivotElement(a, used_rows, used_columns):
    # This algorithm selects the first free element.
    # You'll need to improve it to pass the problem.
    pivot_element = Position(0, 0)
    while used_rows[pivot_element.row]:
        pivot_element.row += 1
    while used_columns[pivot_element.column]:
        pivot_element.column += 1
    return pivot_element

def SwapLines(a, b, used_rows, pivot_element):
    pe = pivot_element
    #for i in range(len(a)):
    #    print(a[i], b[i])
    #print(pe.row, pe.column)
    if a[pe.row][pe.column] == 0:
        for i in range(pe.row + 1, len(a)):
            #print(i)
            if a[i][pe.column] != 0:
                #print("Swapping row", pivot_element.row, "with", i)
                pivot_element.row = i
                break
    a[pivot_element.column], a[pivot_element.row] = a[pivot_element.row], a[pivot_element.column]
    b[pivot_element.column], b[pivot_element.row] = b[pivot_element.row], b[pivot_element.column]
    used_rows[pivot_element.column], used_rows[pivot_element.row] = used_rows[pivot_element.row], used_rows[pivot_element.column]
    pivot_element.row = pivot_element.column;
    #for i in range(len(a)):
    #    print(a[i], b[i])
    return (a, b)

def scale(a, b, pivot_element):
    pe = pivot_element
    peval = a[pe.row][pe.column]
    #print(pe.row, pe.column, peval)
    if peval == 1:
        return (a, b)

    for i in range(len(a[pe.row])):
        a[pe.row][i] /= peval
    b[pe.row] /= peval
    return (a, b)

def zero(a, b, pivot_element):
    pe = pivot_element
    peval = a[pe.row][pe.column]
    if peval != 1:
        # TODO: error
        print("pe != 1")
        exit(-1)
        return (a, b)

    for i in range(pe.row + 1, len(a)):
        val = a[i][pe.column]
        x = 0 - val
        #print("val:", val)
        #print("x:", x)
        for j in range(pe.column, len(a)):
            a[i][j] += a[pe.row][j] * x
        b[i] += b[pe.row] * x
    return (a, b)

def ProcessPivotElement(a, b, pivot_element):
    # Write your code here
    #pass
    pe = pivot_element

    #print()
    #for i in range(len(a)):
    #    print(a[i], b[i])
    #print("A(%d, %d) =" % (pe.row, pe.column), a[pe.row][pe.column])

    #print("Scaling:")
    (a, b) = scale(a, b, pivot_element)
    #for i in range(len(a)):
    #    print(a[i], b[i])

    #print("Zeroeing:")
    (a, b) = zero(a, b, pivot_element)
    #for i in range(len(a)):
    #    print(a[i], b[i])

def MarkPivotElementUsed(pivot_element, used_rows, used_columns):
    used_rows[pivot_element.row] = True
    used_columns[pivot_element.column] = True

def SolveEquation(equation):
    a = equation.a
    b = equation.b
    size = len(a)
    #print("size:", size)

    used_columns = [False] * size
    used_rows = [False] * size
    for step in range(size):
        pivot_element = SelectPivotElement(a, used_rows, used_columns)
        #SwapLines(a, b, used_rows, pivot_element)
        (a, b) = SwapLines(a, b, used_rows, pivot_element)
        #print("!a:", a)
        #print("!b:", b)
        ProcessPivotElement(a, b, pivot_element)
        MarkPivotElementUsed(pivot_element, used_rows, used_columns)
    #print()
    for i in reversed(range(size - 1)):
        #print(i)
        leftsum = 0
        for j in range(i + 1, len(a)):
            #print("j:", j)
            leftsum += a[i][j] * b[j]
        #print("leftsum:", leftsum)
        b[i] -= leftsum
        #print("x%d =" % (i + 1), b[i])

    return b

def PrintColumn(column):
    size = len(column)
    for row in range(size):
        print("%.20lf" % column[row])

if __name__ == "__main__":
    equation = ReadEquation()
    solution = SolveEquation(equation)
    PrintColumn(solution)
    exit(0)
