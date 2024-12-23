# python3

from collections import deque

class Request:
    def __init__(self, arrival_time, process_time):
        self.arrival_time = arrival_time
        self.process_time = process_time

class Response:
    def __init__(self, dropped, start_time):
        self.dropped = dropped
        self.start_time = start_time

class Buffer:
    def __init__(self, size):
        self.size = size
        self.finish_time_ = []
        self.ft = deque()

    def Process(self, request):
        # write your code here
        #print("Process() request.arrival_time =", request.arrival_time, "request.process_time =", request.process_time)
        #print("self.ft", list(self.ft), "len(self.ft) =", len(self.ft))
        while len(self.ft) != 0:
            if self.ft[0] <= request.arrival_time:
                self.ft.popleft()
            else:
                break
        if len(self.ft) >= self.size:
            return Response(True, -1)
        if len(self.ft) > 0:
            self.ft.append(self.ft[-1] + request.process_time)
            return Response(False, self.ft[-2])
        else:
            self.ft.append(request.arrival_time + request.process_time)
            return Response(False, request.arrival_time)
        #return Response(False, -1)

def ReadRequests(count):
    requests = []
    for i in range(count):
        arrival_time, process_time = map(int, input().strip().split())
        requests.append(Request(arrival_time, process_time))
    return requests

def ProcessRequests(requests, buffer):
    responses = []
    for request in requests:
        responses.append(buffer.Process(request))
    return responses

def PrintResponses(responses):
    for response in responses:
        print(response.start_time if not response.dropped else -1)

if __name__ == "__main__":
    size, count = map(int, input().strip().split())
    requests = ReadRequests(count)

    buffer = Buffer(size)
    responses = ProcessRequests(requests, buffer)

    PrintResponses(responses)
