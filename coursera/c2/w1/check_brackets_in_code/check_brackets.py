# python3

import sys

class Bracket:
    def __init__(self, bracket_type, position):
        self.bracket_type = bracket_type
        self.position = position

    def Match(self, c):
        if self.bracket_type == '[' and c == ']':
            return True
        if self.bracket_type == '{' and c == '}':
            return True
        if self.bracket_type == '(' and c == ')':
            return True
        return False

if __name__ == "__main__":
    text = sys.stdin.read()

    opening_brackets_stack = []
    size = 0
    for i, next in enumerate(text):
        #print(i, next)
        if next == '(' or next == '[' or next == '{':
            # Process opening bracket, write your code here
            #pass
            st = Bracket(next, i + 1)
            #opening_brackets_stack = [st] + opening_brackets_stack
            opening_brackets_stack.insert(size, st)
            size += 1
            #print("size is", size)

        if next == ')' or next == ']' or next == '}':
            # Process closing bracket, write your code here
            #pass
            if not opening_brackets_stack:
                print(i + 1)
                sys.exit()
            del opening_brackets_stack[size - 1]
            size -= 1
            if st.Match(next) == False:
                print(i + 1)
                sys.exit()
            elif opening_brackets_stack:
                #st = opening_brackets_stack[0]
                st = opening_brackets_stack[size - 1]

    # Printing answer, write your code here
    if not opening_brackets_stack:
        print("Success")
    else:
        print(st.position)
