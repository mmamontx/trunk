# python3
import sys

dic = {}

def PreprocessBWT(bwt):
  """
  Preprocess the Burrows-Wheeler Transform bwt of some text
  and compute as a result:
    * starts - for each character C in bwt, starts[C] is the first position 
        of this character in the sorted array of 
        all characters of the text.
    * occ_count_before - for each character C in bwt and each position P in bwt,
        occ_count_before[C][P] is the number of occurrences of character C in bwt
        from position 0 to position P inclusive.
  """
  # Implement this function yourself
  srtd = ''.join(sorted(bwt))
  starts = []
  #dic = {}
  global dic
  j = 0
  for i in range(len(bwt)):
    if bwt[i] not in dic:
      dic[bwt[i]] = j
      starts.append(srtd.find(bwt[i]))
      j += 1
  occ_count_before = [[0 for j in range(len(dic))] for i in range(len(bwt) + 1)]
  for i in range(len(bwt)):
    for j in range(len(dic)):
      occ_count_before[i + 1][j] = occ_count_before[i][j]
    occ_count_before[i + 1][dic[bwt[i]]] += 1
  return starts, occ_count_before
  #pass


def CountOccurrences(pattern, bwt, starts, occ_counts_before):
  """
  Compute the number of occurrences of string pattern in the text
  given only Burrows-Wheeler Transform bwt of the text and additional
  information we get from the preprocessing stage - starts and occ_counts_before.
  """
  # Implement this function yourself
  global dic
  top = 0
  bottom = len(bwt) - 1
  while top <= bottom:
    if pattern:
      symbol = pattern[-1]
      pattern = pattern[:-1]
      if symbol in dic:
        top = starts[dic[symbol]] + occ_counts_before[top][dic[symbol]]
        bottom = starts[dic[symbol]] + occ_counts_before[bottom + 1][dic[symbol]] - 1
      else:
        return 0
    else:
      return bottom - top + 1
  return 0
     


if __name__ == '__main__':
  bwt = sys.stdin.readline().strip()
  pattern_count = int(sys.stdin.readline().strip())
  patterns = sys.stdin.readline().strip().split()
  # Preprocess the BWT once to get starts and occ_count_before.
  # For each pattern, we will then use these precomputed values and
  # spend only O(|pattern|) to find all occurrences of the pattern
  # in the text instead of O(|pattern| + |text|).  
  starts, occ_counts_before = PreprocessBWT(bwt)
  #print("starts are", starts)
  #print("occ_counts_before are", occ_counts_before)
  occurrence_counts = []
  for pattern in patterns:
    occurrence_counts.append(CountOccurrences(pattern, bwt, starts, occ_counts_before))
  print(' '.join(map(str, occurrence_counts)))
