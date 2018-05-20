# Uses python3
import sys

def get_change(n):
    #write your code here
    coins = [10, 5, 1]

    for i in range(0, 3):
        if (n >= coins[i]):
            count = (int)(n / coins[i])
            return count + get_change(n - coins[i] * count)

    #return n
    return 0

if __name__ == '__main__':
    n = int(sys.stdin.read())
    print(get_change(n))
