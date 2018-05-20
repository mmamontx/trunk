# Uses python3
import sys

def get_majority_element(a, left, right):
    if left == right:
        return -1
    if left + 1 == right:
        return a[left]
    #write your code here
    k = left + int((right - left) / 2)
    l = get_majority_element(a, left, k)
    r = get_majority_element(a, k, right)
    if l != -1 and r != -1:
        if l == r:
            return l
        if left > 0:
            arr = a[left - 1:right]
        else:
            arr = a[left:right]
        lc = arr.count(l)
        #rc = arr.count(r)
        #if lc == rc:
        #    return -1
        if lc > int((right - left) / 2):
            return l
        #if rc > int((right - left) / 2):
        #    return r
        #return -1
        else:
            return r
    if l != -1:
        arr = a[left:right]
        lc = arr.count(l)
        if lc > int((right - left) / 2):
            return l
        else:
            return -1
    if r != -1:
        arr = a[left:right]
        rc = arr.count(r)
        if rc > int((right - left) / 2):
            return r
        else:
            return -1
    return -1

if __name__ == '__main__':
    input = sys.stdin.read()
    n, *a = list(map(int, input.split()))
    if get_majority_element(a, 0, n) != -1:
        print(1)
    else:
        print(0)
