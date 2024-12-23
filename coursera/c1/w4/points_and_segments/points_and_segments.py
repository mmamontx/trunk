# Uses python3
import sys
import random

def partition3(a, l, r):
    x = a[l]
    j = l - 1
    m1 = m2 = l
    for i in range(l + 1, r + 1):
        if a[i] == x:
           m2 += 1
           a[m2], a[i] = a[i], a[m2]
        if a[i] < x:
            j += 1
            a[i], a[j] = a[j], a[i]
            m1 += 1
            m2 += 1
            a[i], a[m2] = a[m2], a[i]
    return m1, m2

def randomized_quick_sort(a, l, r):
    if l >= r:
        return
    k = random.randint(l, r)
    a[l], a[k] = a[k], a[l]
    m1, m2 = partition3(a, l, r)
    randomized_quick_sort(a, l, m1 - 1)
    randomized_quick_sort(a, m2 + 1, r)

def fast_count_segments(starts, ends, points):
    cnt = [0] * len(points)
    #write your code here
    pairs = []
    for i in range(len(starts)):
        pairs.append((starts[i], 'l'))
    for i in range(len(points)):
        pairs.append((points[i], 'p', i))
    for i in range(len(ends)):
        pairs.append((ends[i], 'r'))
    #print(pairs)
    randomized_quick_sort(pairs, 0, len(pairs) - 1)
    #print(pairs)
    count = 0
    for i in range(len(pairs)):
        if pairs[i][1] == 'l':
            count += 1
        if pairs[i][1] == 'p':
            cnt[pairs[i][2]] = count
        if pairs[i][1] == 'r':
            count -= 1
    return cnt

def naive_count_segments(starts, ends, points):
    cnt = [0] * len(points)
    for i in range(len(points)):
        for j in range(len(starts)):
            if starts[j] <= points[i] <= ends[j]:
                cnt[i] += 1
    return cnt

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n = data[0]
    m = data[1]
    starts = data[2:2 * n + 2:2]
    ends   = data[3:2 * n + 2:2]
    points = data[2 * n + 2:]
    #use fast_count_segments
    #cnt = naive_count_segments(starts, ends, points)
    cnt = fast_count_segments(starts, ends, points)
    for x in cnt:
        print(x, end=' ')
