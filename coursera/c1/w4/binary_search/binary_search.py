# Uses python3
import sys

def binary_search(a, x):
    #left, right = 0, len(a)
    left, right = 0, len(a) - 1
    # write your code here
    while left <= right:
        i = left + int((right - left) / 2)
        if x == a[i]:
            return i
        else:
            if x < a[i]:
                right = i - 1
            else:
                left = i + 1
    return -1

def linear_search(a, x):
    for i in range(len(a)):
        if a[i] == x:
            return i
    return -1

if __name__ == '__main__':
    input = sys.stdin.read()
    data = list(map(int, input.split()))
    n = data[0]
    m = data[n + 1]
    a = data[1 : n + 1]
    for x in data[n + 2:]:
        # replace with the call to binary_search when implemented
        #print(linear_search(a, x), end = ' ')
        print(binary_search(a, x), end = ' ')
