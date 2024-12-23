# python3

class priority_queue:
    def __init__(self):
        self.size = 0
        self.threads = []

    def SiftDown(self, i):
        while (i * 2 + 1) <= (self.size - 1):
            maxi = i
            l = i * 2 + 1
	    #if l <= (self.size - 1) and self.threads[l][1] < self.threads[maxi][1]:
            if l <= (self.size - 1) and (self.threads[l][1] < self.threads[maxi][1] or (self.threads[l][1] == self.threads[maxi][1] and self.threads[l][0] < self.threads[maxi][0])):
                maxi = l
            r = i * 2 + 2
            #if r <= (self.size - 1) and self.threads[r][1] < self.threads[maxi][1]:
            if r <= (self.size - 1) and (self.threads[r][1] < self.threads[maxi][1] or (self.threads[r][1] == self.threads[maxi][1] and self.threads[r][0] < self.threads[maxi][0])):
                maxi = r
            if maxi != i:
                self.threads[maxi], self.threads[i] = self.threads[i], self.threads[maxi]
                i = maxi
            else:
                break

    def ChangePriority(self, i, p):
        oldp = self.threads[i][1]
        x = self.threads[i][0]
        self.threads[i] = (x, p)
        if p < oldp:
            self.SiftUp(i)
        else:
            self.SiftDown(i)

    def GetMax(self):
        #mini = self.size - 1
        #minp = self.threads[0][1]
        #index = 0
        #for i in range(self.size):
        #    if self.threads[i][1] == minp and self.threads[i][0] < mini:
        #         mini = self.threads[i][0]
        #         index = i
        #return index
        return 0

    #def ExtractMax(self):
    #    result = self.threads[0]
    #    self.threads[0] = self.threads[self.size - 1]
    #    del self.threads[-1]
    #    self.size -= 1
    #    self.SiftDown(0)
    #    return result

    def SiftUp(self, i):
        while i > 0 and self.threads[i // 2][1] > self.threads[i][1]:
            self.threads[i // 2], self.threads[i] = self.threads[i], self.threads[i // 2]
            i = i // 2

    def Insert(self, i, p):
        self.size += 1
        self.threads.append((i, p))
        self.SiftUp(self.size - 1)

class JobQueue:
    def read_data(self):
        self.num_workers, m = map(int, input().split())
        self.jobs = list(map(int, input().split()))
        assert m == len(self.jobs)

    def write_response(self):
        for i in range(len(self.jobs)):
          print(self.assigned_workers[i], self.start_times[i]) 

    def assign_jobs(self):
        # TODO: replace this code with a faster algorithm.
        self.assigned_workers = [None] * len(self.jobs)
        self.start_times = [None] * len(self.jobs)
        #next_free_time = [0] * self.num_workers
        #for i in range(len(self.jobs)):
        #  next_worker = 0
        #  for j in range(self.num_workers):
        #    if next_free_time[j] < next_free_time[next_worker]:
        #      next_worker = j
        #  self.assigned_workers[i] = next_worker
        #  self.start_times[i] = next_free_time[next_worker]
        #  next_free_time[next_worker] += self.jobs[i]
        pq = priority_queue()
        for i in range(self.num_workers):
            pq.Insert(i, 0)

        for i in range(len(self.jobs)):
            gm = pq.GetMax()
            self.assigned_workers[i] = pq.threads[gm][0]
            self.start_times[i] = pq.threads[gm][1]
            pq.ChangePriority(gm, pq.threads[gm][1] + self.jobs[i])

    def solve(self):
        self.read_data()
        self.assign_jobs()
        self.write_response()

if __name__ == '__main__':
    job_queue = JobQueue()
    job_queue.solve()

