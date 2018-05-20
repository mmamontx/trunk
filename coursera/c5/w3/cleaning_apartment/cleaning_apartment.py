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
    #for edge in edges:
    #    print(edge)

    l = [set() for i in range(n)]
    for edge in edges:
        l[edge[0] - 1].add(edge[1])
        l[edge[1] - 1].add(edge[0])

    lines = 0
    for i in range(n):
        for j in range(i + 1, n):
            if j + 1 in l[i]:
                continue
            for k in range(n):
                for k1 in range(n):
                    if abs(k - k1) == 1:
                        lines += 1

    print(n + ((n - 1) * n // 2) * n \
          + lines + ((n - 1) * n // 2) * n + n, n * n)

    # All vertices must be on the path
    for i in range(n):
        for j in range(n):
            print("%d " % (i * n + j + 1), end='')
        print(0)

    # Each vertex must be visited exactly once
    for k in range(n):
        for i in range(1, n + 1):
            for j in range(i + 1, n + 1):
                print("%d %d " % (0 - (i + k * n), 0 - (j + k * n)), end='')
                print(0)

    '''
    # Two successive vertices must be connected by an edge
    for i in range(len(l)):
        for j in range(n):
            pos = i * n + 1 + j
            p = []
            for k in range(len(l[i])):
                if j > 0:
                    pos1 = (l[i][k] - 1) * n + 1 + j
                    pos1 -= 1
                    p.append(pos1)
                if j < (n - 1):
                    pos2 = (l[i][k] - 1) * n + 1 + j
                    pos2 += 1
                    p.append(pos2)
            print("%d " % (0 - pos), end='')
            for k in range(len(p)):
                print("%d " % p[k], end='')
            print(0)
    '''
    # Nonadjacent nodes cannot be adjacent in the path
    for i in range(n):
        for j in range(i + 1, n):
            if j + 1 in l[i]:
                continue
            for k in range(n):
                for k1 in range(n):
                    if abs(k - k1) == 1:
                        print("%d %d 0" % (0 - (i * n + k + 1), 0 - (j * n + k1 + 1)))

    # There is only one vertex on each position in the path
    for k in range(1, n + 1):
        for i in range(n):
            for j in range(i + 1, n):
                print("%d %d " % (0 - (i * n + k), 0 - (j * n + k)), end='')
                print(0)

    # Every position on the path must be occupied
    for i in range(n):
        for j in range(n):
            val = i + j * n + 1
            print("%d " % val, end='')
        print(0)

printEquisatisfiableSatFormula()
