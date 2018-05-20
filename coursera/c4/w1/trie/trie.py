#Uses python3
import sys

# Return the trie built from patterns
# in the form of a dictionary of dictionaries,
# e.g. {0:{'A':1,'T':2},1:{'C':3}}
# where the key of the external dictionary is
# the node ID (integer), and the internal dictionary
# contains all the trie edges outgoing from the corresponding
# node, and the keys are the letters on those edges, and the
# values are the node IDs to which these edges lead.
def build_trie(patterns):
    tree = dict()
    # write your code here
    tree[0] = {}
    counter = 0
    #print(patterns)
    #print(tree)
    for pat in patterns:
        cn = 0
        #print(pat)
        for i in range(len(pat)):
            #print(i, pat[i])
            if pat[i] in tree[cn]:
                cn = tree[cn][pat[i]]
                #print('Obtained', cn)
            else:
                counter += 1
                tree[cn][pat[i]] = counter
                cn = counter
                tree[cn] = {}
                #print('Increased to', cn)
    return tree


if __name__ == '__main__':
    patterns = sys.stdin.read().split()[1:]
    tree = build_trie(patterns)
    for node in tree:
        for c in tree[node]:
            print("{}->{}:{}".format(node, tree[node][c], c))
