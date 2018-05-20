# Uses python3
def edit_distance(s, t):
    #write your code here
    n = len(s) + 1
    m = len(t) + 1
    d = [[0 for j in range(m)] for i in range(n)]
    for i in range(n):
        d[i][0] = i
    for j in range(m):
        d[0][j] = j
    for j in range(1, m):
        for i in range(1, n):
            ins = d[i][j - 1] + 1
            de = d[i - 1][j] + 1
            if s[i - 1] == t[j - 1]:
                mat = d[i - 1][j - 1]
                d[i][j] = min([ins, de, mat])
            else:
                mis = d[i - 1][j - 1] + 1
                d[i][j] = min([ins, de, mis])
    #return 0
    return d[i][j]

if __name__ == "__main__":
    print(edit_distance(input(), input()))
