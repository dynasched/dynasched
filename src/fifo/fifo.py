#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

import dsched


fifos = []


@dsched.hookimpl
def queue_init():
    """Setup one basic FIFO queue
    """
    q = {"name": "base", "status": "active", "requests": []}
    fifos.append(q)


@dsched.hookimpl
def queue_status(queues):
    """Cycle across the FIFO queues and
       return the status of each of them
    """
    ans = []
    for q in fifos:
        status = (q['name'], q['status'])
        ans.append(status)
    return ("fifo", ans)


@dsched.hookimpl
def queue_add(req):
    """Add a request to one of our queues
       The request dict contains a range of
       information describing the request.
       It is up to us to decide which queue
       to add it to based on that info
    """
    q = fifos[0]
    q.append(req)
    return

@dsched.hookimpl
def queue_num_pending(queues):
    """Return the number of pending requests in
       the apecified queues. None means return
       all of them
    """
    ans = []
    if queues is None:
        for q in fifos:
            ans.append((q['name'], q['requests'].size))
    else:
        for p in queues:
            for q in fifos:
                if q['name'] == p:
                    ans.append((q['name'], q['requests'].size))
    return ("fifo", ans)


@dsched.hookimpl
def queue_names():
    """Return the names of our queues
    """
    ans = []
    for q in fifos:
        ans.append(q['name'])
    return ("fifo", ans)
