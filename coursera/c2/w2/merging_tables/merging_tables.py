# python3

import sys

n, m = map(int, sys.stdin.readline().split())
lines = list(map(int, sys.stdin.readline().split()))
rank = [1] * n
parent = list(range(0, n))
ans = max(lines)

def getParent(table):
    # find parent and compress path
    global parent
    while table != parent[table]:
        table = parent[table]
    return table
    #return parent[table]

def Union(i, j):
    global lines, rank, parent, ans
    i_id = getParent(i)
    j_id = getParent(j)
    if i_id == j_id:
        return
    if rank[i_id] > rank[j_id]:
        parent[j_id] = i_id
        lines[i_id] += lines[j_id]
        lines[j_id] = 0
    else:
        parent[i_id] = j_id
        lines[j_id] += lines[i_id]
        lines[i_id] = 0
        if rank[i_id] == rank[j_id]:
            rank[j_id] += 1
    ans = max(ans, lines[i_id] + lines[j_id])

def merge(destination, source):
    realDestination, realSource = getParent(destination), getParent(source)

    if realDestination == realSource:
        return False

    # merge two components
    # use union by rank heuristic 
    # update ans with the new maximum table size
    Union(realDestination, realSource)
    
    return True

for i in range(m):
    destination, source = map(int, sys.stdin.readline().split())
    merge(destination - 1, source - 1)
    print(ans)
    
#print(parent)
#print(rank)
