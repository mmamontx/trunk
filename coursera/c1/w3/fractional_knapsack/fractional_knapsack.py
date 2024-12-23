# Uses python3
import sys

def findmi(weights, values, n):
    value = mvalue = 0.
    mi = 0
    for i in range(0, n):
        if weights[i] > 0:
            value = values[i] / weights[i]
            if value > mvalue:
                mvalue = value
                mi = i

    return mi

#def get_optimal_value(capacity, weights, values):
def get_optimal_value(capacity, weights, values, n):
    value = 0.
    # write your code here
    for i in range(0, n):
        if capacity == 0:
            return value
        mi = findmi(weights, values, n)
        a = weights[mi] if weights[mi] < capacity else capacity
        value += a * values[mi] / weights[mi]
        weights[mi] -= a
        capacity -= a

    return value


if __name__ == "__main__":
    data = list(map(int, sys.stdin.read().split()))
    n, capacity = data[0:2]
    values = data[2:(2 * n + 2):2]
    weights = data[3:(2 * n + 2):2]
    #opt_value = get_optimal_value(capacity, weights, values)
    opt_value = get_optimal_value(capacity, weights, values, n)
    print("{:.10f}".format(opt_value))
