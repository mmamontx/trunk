# python3
import sys


def ComputePrefixFunction(P):
  #print("The prefix is", P)
  s = [0 for i in range(len(P))]
  border = 0
  for i in range(1, len(P)):
    while border > 0 and P[i] != P[border]:
      border = s[border - 1]
    if P[i] == P[border]:
      border += 1
    else:
      border = 0
    s[i] = border
  return s

def find_pattern(pattern, text):
  """
  Find all the occurrences of the pattern in the text
  and return a list of all positions in the text
  where the pattern starts in the text.
  """
  result = []
  # Implement this function yourself
  s = ComputePrefixFunction(pattern + '$' + text)
  #print(s)
  for i in range(len(pattern) + 1, len(s)):
      if s[i] == len(pattern):
          result.append(i - 2 * len(pattern))
  return result


if __name__ == '__main__':
  pattern = sys.stdin.readline().strip()
  text = sys.stdin.readline().strip()
  result = find_pattern(pattern, text)
  print(" ".join(map(str, result)))

