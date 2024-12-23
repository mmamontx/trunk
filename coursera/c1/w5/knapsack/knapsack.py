# Uses python3
import sys

def optimal_weight(W, w):
    # write your code here
    #result = 0
    #for x in w:
    #    if result + x <= W:
    #        result = result + x
    n = len(w)
    value = [[0 for x in range(n + 1)] for x in range(W + 1)]
    w = [0] + w
    for i in range(1, n + 1):
        for j in range(1, W + 1):
            value[j][i] = value[j][i - 1]
            if w[i] <= j:
               val = value[j - w[i]][i - 1] + w[i]
               if value[j][i] < val:
                   value[j][i] = val
    return value[W][n]
    #return result

if __name__ == '__main__':
    input = sys.stdin.read()
    W, n, *w = list(map(int, input.split()))
    print(optimal_weight(W, w))
