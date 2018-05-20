# python3
n, m = map(int, input().split())
edges = [ list(map(int, input().split())) for i in range(m) ]

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
    #print("n = %d m = %d" % (n, m))
    #print("edges:")
    #for edge in edges:
    #    print(edge)

    # Number of clauses and vars
    print(n + m * 3, n * 3)

    # Each vertex should have some color
    for i in range(n):
        print(i * 3 + 1, i * 3 + 2, i * 3 + 3, 0)

    # Connected vertices should have different colors
    for edge in edges:
        for i in range(1, 4):
            print(0 - ((edge[0] - 1) * 3 + i), 0 - ((edge[1] - 1) * 3 + i), 0)

printEquisatisfiableSatFormula()
