# python3

import random
import sys

def read_input():
    return (input().rstrip(), input().rstrip())

def print_occurrences(output):
    print(' '.join(map(str, output)))

def _hash_func(s, p, x):
    ans = 0
    for c in reversed(s):
        ans = (ans * x + ord(c)) % p
    #print(s, ans)
    return ans
    #return ans % self.bucket_count

def AreEqual(a, b):
    #print(a, b)
    if len(a) != len(b):
        return False
    for i in range(len(a)):
        if a[i] != b[i]:
            return False
    return True

def PrecomputeHashes(text, patlen, p, x):
    H = [0 for i in range(len(text) - patlen + 1)]
    S = text[len(text) - patlen:len(text)]
    #print(S)
    H[len(text) - patlen] = _hash_func(S, p, x)
    y = 1
    for i in range(1, patlen + 1):
        y = (y * x) % p
    for i in reversed(range(len(text) - patlen)):
        H[i] = (x * H[i + 1] + ord(text[i]) - y * ord(text[i + patlen])) % p
    return H

def get_occurrences(pattern, text):
    p = 1000000007
    #p = sys.maxsize
    #x = 263
    x = random.randint(1, p - 1)
    #x = 1
    result = []
    pHash = _hash_func(pattern, p, x)
    H = PrecomputeHashes(text, len(pattern), p, x)
    #print(H)
    for i in range(len(text) - len(pattern) + 1):
        #tHash = _hash_func(text[i:i + len(pattern)], p, x)
        #if pHash != tHash:
        if pHash != H[i]:
            continue
        #if AreEqual(text[i:i + len(pattern)], pattern):
        if text[i:i + len(pattern)] == pattern:
            result.append(i)
    return result
    #return [
    #    i 
    #    for i in range(len(text) - len(pattern) + 1) 
    #    if text[i:i + len(pattern)] == pattern
    #]

if __name__ == '__main__':
    print_occurrences(get_occurrences(*read_input()))

