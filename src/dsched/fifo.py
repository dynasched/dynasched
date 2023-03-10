#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

import signal
import os
import threading
import socket

def main():
    fifo = FIFO()
    fifo.queue_init()
    res = fifo.queue_names()
    print("QUEUES:", res)

class FIFO:
    def __init__(self):
        self.fifos = []

    def queue_init(self):
        """Setup one basic FIFO queue
        """
        print("FIFO INIT")
        q = {"name": "base", "status": "active", "requests": []}
        self.fifos.append(q)

    def queue_status(self, queues):
        """Cycle across the FIFO queues and
           return the status of each of them
        """
        ans = []
        for q in self.fifos:
            status = (q['name'], q['status'])
            ans.append(status)
        return ("fifo", ans)


    def queue_add(self, req):
        """Add a request to one of our queues
           The request dict contains a range of
           information describing the request.
           It is up to us to decide which queue
           to add it to based on that info
        """
        q = self.fifos[0]
        q.append(req)
        return

    def queue_num_pending(self, queues):
        """Return the number of pending requests in
           the apecified queues. None means return
           all of them
        """
        ans = []
        if queues is None:
            for q in self.fifos:
                ans.append((q['name'], q['requests'].size))
        else:
            for p in queues:
                for q in self.fifos:
                    if q['name'] == p:
                        ans.append((q['name'], q['requests'].size))
        return ("fifo", ans)


    def queue_names(self):
        """Return the names of our queues
        """
        print("FIFO NAMES")
        ans = []
        for q in self.fifos:
            ans.append(q['name'])
        print("ANS", ans)
        return ("fifo", ans)

if __name__ == "__main__":
    main()
