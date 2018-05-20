# Uses python3
import sys

def merge_sort(a, left, right):
    #print("merge_sort() left:", left, "right:", right)
    n = 0
    if (right - left) <= 1:
        return a, n
    ave = (left + right) // 2
    a, n1 = merge_sort(a, left, ave)
    n += n1
    a, n1 = merge_sort(a, ave, right)
    n += n1
    b = a[left:ave]
    c = a[ave:right]
    d, n1 = merge(b, c)

    #print("Before:", a)
    #print("n is", n)
    i = left
    while i < right:
        a[i] = d.pop(0)
        i += 1

    n += n1
    #print("After:", a)
    #print("n is", n)
    return a, n

def merge(b, c):
    d = []
    n = 0
    #for i in range(0, len(b)):
    #    for j in range(0, len(c)):
    #        if b[i] > c[j]:
    #            n += 1
    while b and c:
        if b[0] <= c[0]:
            d.append(b[0])
            b.pop(0)
        else:
            n += len(b)
            d.append(c[0])
            c.pop(0)
    if not b:
        d += c
    if not c:
        d += b
    return d, n

def get_number_of_inversions(a, b, left, right):
    number_of_inversions = 0
    if right - left <= 1:
        return number_of_inversions
    #ave = (left + right) // 2
    #number_of_inversions += get_number_of_inversions(a, b, left, ave)
    #number_of_inversions += get_number_of_inversions(a, b, ave, right)
    #write your code here
    for i in range(10, 0):
        print(i)
    a, number_of_inversions = merge_sort(a, left, right)
    return number_of_inversions

if __name__ == '__main__':
    input = sys.stdin.read()
    n, *a = list(map(int, input.split()))
    b = n * [0]
    print(get_number_of_inversions(a, b, 0, len(a)))
