# python3
import sys

#def findnth(haystack, needle, n):
#    start = -1
#    while n > 0:
#        start = haystack.find(needle, start + 1)
#        n -= 1
#    return start

#def getn(s, i):
#    start = -1
#    n = 0
#    while 1:
#        start = s.find(s[i], start + 1)
#        n += 1
#        if start == i:
#            break
#    return n

def InverseBWT(bwt):
    # write your code here
    srtd = ''.join(sorted(bwt))
    #print("bwt is", bwt)
    #print("srtd is", srtd)
    #print("count is", bwt)
    #print("srtd is", srtd)
    first = []
    dic = {}
    j = 0
    for i in range(len(bwt)):
        if bwt[i] not in dic:
            dic[bwt[i]] = j
            first.append(srtd.find(bwt[i]))
            j += 1
    #print(dic, len(dic))
    #print("first is", first)
    count = [[0 for j in range(len(dic))] for i in range(len(bwt) + 1)]
    #print(count)
    for i in range(len(bwt)):
        for j in range(len(dic)):
            count[i + 1][j] = count[i][j]
        count[i + 1][dic[bwt[i]]] += 1
    #print(count)
    j = 0
    result = "$"
    for i in range(len(bwt) - 1):
        #n = getn(bwt, j)
        #n = count[j][dic[bwt[j]]]
        #print(bwt[j], "is", n, "-th")
        #j = findnth(srtd, bwt[j], n)
        #j = srtd.find(bwt[j])
        j = first[dic[bwt[j]]] + count[j][dic[bwt[j]]]
        #print("j is", j)
        result += srtd[j]
    return result[::-1]
    #return ""


if __name__ == '__main__':
    bwt = sys.stdin.readline().strip()
    print(InverseBWT(bwt))
