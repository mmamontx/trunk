# python3
import sys

NA = -1

class Node:
	def __init__ (self):
		self.next = [NA] * 4
		self.patternEnd = False

def build_trie(patterns):
    tree = dict()
    tree[0] = {}
    counter = 0
    for pat in patterns:
        cn = 0
        for i in range(len(pat)):
            if pat[i] in tree[cn]:
                cn = tree[cn][pat[i]]
            else:
                counter += 1
                tree[cn][pat[i]] = counter
                cn = counter
                tree[cn] = {}
        #print("cn is", cn)
        tree[cn] = {'flag': 1}
    return tree

def PrefixTrieMatching(text, trie):
	s = ""
	v = 0
	i = 0
	symbol = text[i]
	while 1:
		if not trie[v] or 'flag' in trie[v]:
			return s
		elif i >= len(text):
			return ""
		if text[i] in trie[v]:
			v = trie[v][text[i]]
			s += text[i]
			i += 1
		else:
			return ""

def TrieMatching(text, trie):
	result = []
	for i in range(len(text)):
		s = PrefixTrieMatching(text[i:], trie)
		if s:
			result.append(i)
	return result

def solve (text, n, patterns):
	result = []

	#// write your code here
	trie = build_trie(patterns)
	#print(trie)
	result = TrieMatching(text, trie)

	return result

text = sys.stdin.readline ().strip ()
n = int (sys.stdin.readline ().strip ())
patterns = []
for i in range (n):
	patterns += [sys.stdin.readline ().strip ()]

ans = solve (text, n, patterns)

sys.stdout.write (' '.join (map (str, ans)) + '\n')
