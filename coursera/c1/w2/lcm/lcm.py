# Uses python3
import sys

def gcd(a, b):
    if b == 0:
        return a

    a_ = a % b

    return gcd(b, a_)

def lcm(a, b):
    #write your code here
    #return a*b
    #return int(a*b / gcd(a,b))
    return int(b / gcd(a,b)) * a

if __name__ == '__main__':
    input = sys.stdin.read()
    a, b = map(int, input.split())
    print(lcm(a, b))

