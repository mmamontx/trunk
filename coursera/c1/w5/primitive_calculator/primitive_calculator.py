# Uses python3
import sys

def optimal_sequence(n):
    sequence = []
    #while n >= 1:
    #    sequence.append(n)
    #    if n % 3 == 0:
    #        n = n // 3
    #    elif n % 2 == 0:
    #        n = n // 2
    #    else:
    #        n = n - 1
    s = [0, 0]
    for i in range(2, n + 1):
        m = []
        m.append(s[i - 1] + 1)
        if i % 2 == 0:
            m.append(s[i // 2] + 1)
        if i % 3 == 0:
            m.append(s[i // 3] + 1)
        s.insert(i, min(m))
        del m
    #s.pop(0)
    while n >= 1:
        sequence.append(n)
        m = s[n - 1]
        mi = 0
        if n % 2 == 0:
            if (s[n // 2] < m):
                mi = 1
        if n % 3 == 0:
            if (s[n // 3] < m):
                mi = 2
        if mi == 0:
            n = n - 1
        elif mi == 1:
            n = n // 2
        else: # mi is 2
            n = n // 3
    return reversed(sequence)

input = sys.stdin.read()
n = int(input)
sequence = list(optimal_sequence(n))
print(len(sequence) - 1)
for x in sequence:
    print(x, end=' ')
