# python3
import sys

def BWT(text):
    l = []
    for i in range(len(text)):
        l.append(text[i:] + text[:i])
    #print(l)
    l.sort()
    #print(l)
    result = ""
    for i in range(len(l)):
        result += l[i][-1]
    #print(result)
    return result
    #return ""

if __name__ == '__main__':
    text = sys.stdin.readline().strip()
    print(BWT(text))
