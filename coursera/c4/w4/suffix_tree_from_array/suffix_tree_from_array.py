# python3
import sys


def suffix_array_to_suffix_tree(sa, lcp, text):
    """
    Build suffix tree of the string text given its suffix array suffix_array
    and LCP array lcp_array. Return the tree as a mapping from a node ID
    to the list of all outgoing edges of the corresponding node. The edges in the
    list must be sorted in the ascending order by the first character of the edge label.
    Root must have node ID = 0, and all other node IDs must be different
    nonnegative integers. Each edge must be represented by a tuple (node, start, end), where
        * node is the node ID of the ending node of the edge
        * start is the starting position (0-based) of the substring of text corresponding to the edge label
        * end is the first position (0-based) after the end of the substring corresponding to the edge label

    For example, if text = "ACACAA$", an edge with label "$" from root to a node with ID 1
    must be represented by a tuple (1, 6, 7). This edge must be present in the list tree[0]
    (corresponding to the root node), and it should be the first edge in the list (because
    it has the smallest first character of all edges outgoing from the root).
    """
    #tree = {}
    tree = []
    nodes = []
    # Implement this function yourself
    #for i in range(len(sa)):
    #    print("%d:" % i, sa[i], text[sa[i]:])
    #print(lcp)
    tree.append([])
    nodes.append([0, 0]) # [parent, depth]
    lcpPrev = 0
    curNode = 0
    counter = 0
    for i in range(len(text)):
        suffix = sa[i]
        #print("")
        #print(i, "suffix is", text[suffix:])
        #print("lcpPrev =", lcpPrev)
        #print("nodes[%d][1] = %d" % (curNode, nodes[curNode][1]))
        while nodes[curNode][1] > lcpPrev:
            curNode = nodes[curNode][0]
            #print("lcpPrev =", lcpPrev)
            #print("nodes[%d][1] = %d" % (curNode, nodes[curNode][1]))
        if nodes[curNode][1] == lcpPrev:
            tree.append([])
            counter += 1
            tree[curNode].append((counter, suffix + nodes[curNode][1], len(text))) # NOTE: not sure 'bout the last one
            nodes.append([curNode, len(text) - suffix])
            curNode = counter
        else:
            #print("Breaking edge! curNode is", curNode)
            edgeStart = sa[i - 1] + nodes[curNode][1]
            offset = lcpPrev - nodes[curNode][1]

            #print("offset =", offset)
            counter += 1
            for j in range(len(tree[curNode])):
                t = tree[curNode][j]
                #if t[1] == edgeStart:
                if text[t[1]] == text[edgeStart]:
                    tree.append([(t[0], t[1] + offset, t[2])])
                    nodes[t[0]][0] = counter # The selected node becomes a child of the new node
                    del tree[curNode][j]
                    break
            tree[curNode].append((counter, edgeStart, edgeStart + offset))
            nodes.append([curNode, nodes[curNode][1] + offset])
            curNode = counter
            '''
            print("tree after the separation:")
            for j in range(len(tree)):
                print(tree[j])
            print("nodes after the separation:", nodes)
            '''

            tree.append([])
            counter += 1
            tree[curNode].append((counter, suffix + nodes[curNode][1], len(text))) # NOTE: not sure 'bout the last one
            nodes.append([curNode, len(text) - suffix])
            curNode = counter
        if i < (len(text) - 1):
            lcpPrev = lcp[i]
        '''
        print("tree:")
        for j in range(len(tree)):
            print(tree[j])
        print("nodes:", nodes)
        '''
    return tree


if __name__ == '__main__':
    text = sys.stdin.readline().strip()
    sa = list(map(int, sys.stdin.readline().strip().split()))
    lcp = list(map(int, sys.stdin.readline().strip().split()))
    print(text)
    # Build the suffix tree and get a mapping from 
    # suffix tree node ID to the list of outgoing Edges.
    tree = suffix_array_to_suffix_tree(sa, lcp, text)
    """
    Output the edges of the suffix tree in the required order.
    Note that we use here the contract that the root of the tree
    will have node ID = 0 and that each vector of outgoing edges
    will be sorted by the first character of the corresponding edge label.
    
    The following code avoids recursion to avoid stack overflow issues.
    It uses two stacks to convert recursive function to a while loop.
    This code is an equivalent of 
    
        OutputEdges(tree, 0);
    
    for the following _recursive_ function OutputEdges:
    
    def OutputEdges(tree, node_id):
        edges = tree[node_id]
        for edge in edges:
            print("%d %d" % (edge[1], edge[2]))
            OutputEdges(tree, edge[0]);
    
    """
    stack = [(0, 0)]
    result_edges = []
    while len(stack) > 0:
      (node, edge_index) = stack[-1]
      stack.pop()
      #if not node in tree:
      if not tree[node]:
        continue
      edges = tree[node]
      if edge_index + 1 < len(edges):
        stack.append((node, edge_index + 1))
      print("%d %d" % (edges[edge_index][1], edges[edge_index][2]))
      stack.append((edges[edge_index][0], 0))
