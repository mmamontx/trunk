# Uses python3
import sys
from collections import namedtuple

Segment = namedtuple('Segment', 'start end')

def rmmi(segments, mvalue):
    i = 0
    l = len(segments)
    while i < l:
        #print("Checking", segments[i].start, segments[i].end)
        if segments[i].start <= mvalue and mvalue <= segments[i].end:
            del segments[i]
            l -= 1
        else:
            i += 1

    return segments

def findmi(segments):
    mvalue = segments[0].end
    mi = 0
    i = 0
    for s in segments:
        if s.end < mvalue:
            mvalue = s.end
            mi = i
        i += 1

    return mvalue

def optimal_points(segments):
    points = []
    #write your code here
    while len(segments) > 0:
        #for s in segments:
        #    print(s, ": start is ", s.start, ", end is ", s.end)
        mi = findmi(segments)
        points.append(mi)
        #print("Minimum end value is ", mi)
        segments = rmmi(segments, mi)
    #for s in segments:
    #    points.append(s.start)
    #    points.append(s.end)
    return points

if __name__ == '__main__':
    input = sys.stdin.read()
    n, *data = map(int, input.split())
    segments = list(map(lambda x: Segment(x[0], x[1]), zip(data[::2], data[1::2])))
    points = optimal_points(segments)
    print(len(points))
    for p in points:
        print(p, end=' ')
