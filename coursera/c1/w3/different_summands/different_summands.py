# Uses python3
import sys

def pwds(k, l, s):
    if k <= (2 * l):
        s.append(k)
        return s
    else:
        s.append(l)
        s = pwds(k - l, l + 1, s)
        return s

def optimal_summands(n):
    summands = []
    l = 1
    while n > 0:
        if n <= (2 * l):
            summands.append(n)
            n = 0
        else:
            summands.append(l)
            n -= l
            l += 1
    #summands = pwds(n, 1, summands)
    return summands

if __name__ == '__main__':
    input = sys.stdin.read()
    n = int(input)
    summands = optimal_summands(n)
    print(len(summands))
    for x in summands:
        print(x, end=' ')
