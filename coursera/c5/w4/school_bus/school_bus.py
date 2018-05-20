# python3
from math import log
from itertools import permutations
INF = 10 ** 9

table = [0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4]

def select(S):
    for i in range(32):
        val = 1 << i
        if S & val:
            return 1 << i
    return 0

def numbits(value):
    num = 0
    while value > 0:
        v = value & 0xf
        num += table[v]
        value = value >> 4
    return num

def read_data():
    n, m = map(int, input().split())
    graph = [[INF] * n for _ in range(n)]
    for _ in range(m):
        u, v, weight = map(int, input().split())
        u -= 1
        v -= 1
        graph[u][v] = graph[v][u] = weight
    return graph

def print_answer(path_weight, path):
    print(path_weight)
    if path_weight == -1:
        return
    print(' '.join(map(str, path)))

def optimal_path(graph):
    # This solution tries all the possible sequences of stops.
    # It is too slow to pass the problem.
    # Implement a more efficient algorithm here.
    n = len(graph)
    best_ans = INF
    best_path = []

    '''
    for p in permutations(range(n)):
        cur_sum = 0
        for i in range(1, n):
            if graph[p[i - 1]][p[i]] == INF:
                break
            cur_sum += graph[p[i - 1]][p[i]]
        else:
            if graph[p[-1]][p[0]] == INF:
                continue
            cur_sum += graph[p[-1]][p[0]]
            if cur_sum < best_ans:
                best_ans = cur_sum
                best_path = list(p)
    '''
    size = 1 << n
    C = [dict() for i in range(size)]
    bt = {}
    C[1][1] = 0

    # TODO: Pre-compute numbits()
    for s in range(2, n + 1):
        #print("s =", s)
        for S in range(1, size):
            num = numbits(S)
            if num == s:
                C[S][1] = INF
                #print("S = 0x%x" % S)
                Si = S & 0xfffffffe # TODO: Fix
                while Si != 0:
                    i = select(Si)
                    #print("i = 0x%x obtained from Si = 0x%x" % (i, Si))
                    Sj = S ^ i
                    #print("Sj is 0x%x" % Sj)
                    while Sj != 0:
                        j = select(Sj)
                        #print("j = 0x%x obtained from Sj = 0x%x" % (j, Sj))
                        iindex = int(log(i, 2)) + 1
                        jindex = int(log(j, 2)) + 1

                        if jindex not in C[S ^ i]:
                            C[S ^ i][jindex] = INF
                        val = C[S ^ i][jindex] + graph[jindex - 1][iindex - 1]

                        if iindex not in C[S]:
                            C[S][iindex] = INF
                        if val < C[S][iindex]:
                            #print("In set 0x%x distance to vertex %d is %d" % (S, iindex, val))
                            #print("Based on set 0x%x from vertex %d" % (S ^ i, jindex))
                            #print()
                            C[S][iindex] = val
                            bt[(S, iindex)] = (S ^ i, jindex)
                        Sj ^= j
                    Si ^= i

    maxset = -1
    for i in range(1, n + 1):
        #print("0x%x = %d graph = %d best_ans = %d" % (size - 1, C[size - 1][i], graph[i - 1][0], best_ans))
        if (C[size - 1][i] + graph[i - 1][0]) < best_ans: # TODO: Check if size - 1 should be used
        #if (C[(size - 1) ^ i][i] + graph[i - 1][0]) < best_ans: # TODO: Check if size - 1 should be used
            best_ans = C[size - 1][i] + graph[i - 1][0]
            #best_ans = C[(size - 1) ^ i][i] + graph[i - 1][0]
            maxset = i

    if best_ans == INF:
        return (-1, [])
    else:
        best_path.append(1)
        best_path.append(maxset)

        tup = bt[(size - 1, maxset)]
        tupval = C[tup[0]][tup[1]]

        while tupval != 0:
            best_path.append(tup[1])

            tup = bt[(tup[0], tup[1])]
            tupval = C[tup[0]][tup[1]]

    return (best_ans, best_path)
    #return (best_ans, [x + 1 for x in best_path])


if __name__ == '__main__':
    print_answer(*optimal_path(read_data()))
