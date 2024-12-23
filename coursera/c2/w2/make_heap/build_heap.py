# python3

class HeapBuilder:
  def __init__(self):
    self._swaps = []
    self._data = []

  def ReadData(self):
    n = int(input())
    self._data = [int(s) for s in input().split()]
    assert n == len(self._data)

  def WriteResponse(self):
    print(len(self._swaps))
    for swap in self._swaps:
      print(swap[0], swap[1])

  def Swap(self, i, j):
    self._data[i], self._data[j] = self._data[j], self._data[i]
    self._swaps.append((i, j))

  def SiftDown(self, i):
    #print("SiftDown() i is", i)
    #print("Before:", self._data)
    while ((i * 2 + 1) <= len(self._data) - 1):
      if ((i * 2 + 2) <= len(self._data) - 1):
        if self._data[i * 2 + 1] < self._data[i * 2 + 2]:
          if self._data[i * 2 + 1] < self._data[i]:
            self.Swap(i, i * 2 + 1)
            i = i * 2 + 1
          else:
            return
        else:
          if self._data[i * 2 + 2] < self._data[i]:
            self.Swap(i, i * 2 + 2)
            i = i * 2 + 2
          else:
            return
      else:
        if self._data[i * 2 + 1] < self._data[i]:
          self.Swap(i, i * 2 + 1)
        return
    #print("After:", self._data)

  def GenerateSwaps(self):
    # The following naive implementation just sorts 
    # the given sequence using selection sort algorithm
    # and saves the resulting sequence of swaps.
    # This turns the given array into a heap, 
    # but in the worst case gives a quadratic number of swaps.
    #
    # TODO: replace by a more efficient implementation
    #print("len is", len(self._data))
    i = len(self._data) // 2
    i -= 1 # 0-based
    #print("i is", i)
    while i >= 0:
        self.SiftDown(i)
        i -= 1
    #print(self._data)
    #for i in range(len(self._data)):
    #  for j in range(i + 1, len(self._data)):
    #    if self._data[i] > self._data[j]:
    #      self._swaps.append((i, j))
    #      self._data[i], self._data[j] = self._data[j], self._data[i]

  def Solve(self):
    self.ReadData()
    self.GenerateSwaps()
    self.WriteResponse()

if __name__ == '__main__':
    heap_builder = HeapBuilder()
    heap_builder.Solve()
