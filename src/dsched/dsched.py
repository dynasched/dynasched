#!/usr/bin/env python3
#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

# system imports
import signal
import os
import threading
import socket

# project imports
import server_upcalls
from fifo import *
import pmix

terminate_event = threading.Event()

def handler(signum, frame):
    global terminate_event

    # activate the terminate event
    terminate_event.set()

def main():
    global terminate_event

    try:
        dsched = Scheduler()
    except Exception as error:
        print("EXCEPTION ON INIT")
        exit(1)
    print("Testing version ", dsched.get_version())

    # initialize the queues
    try:
        dsched.queue_init()
        print("QUEUES INIT")
    except Exception as error:
        print("QUEUE INIT FAILED");
        exit(1)

    try:
        queues = dsched.queue_names()
        print("QUEUES:", queues)
    except Exception as error:
        exit(1)

    try:
        dsched.query_allocation()
    except Exception as error:
        print("QUERY FAILED")
        pass
#        dsched.finalize()
#        exit(1)

    # setup the signal handler - we protect each
    # call in case a particular system doesn't
    # have the specified signal
    try:
        signal.signal(signal.SIGTERM, handler)
    except:
        pass
    try:
        signal.signal(signal.SIGINT, handler)
    except:
        pass
    try:
        signal.signal(signal.SIGABRT, handler)
    except:
        pass
    try:
        signal.signal(signal.SIGBREAK, handler)
    except:
        pass

    # wait for someone to shut us down
    flag = terminate_event.wait(timeout = None)

    print("FINALIZING")
    dsched.finalize()


class Scheduler(pmix.PMIxScheduler):
    def __init__(self):
        self.allocation = []
        self.topologies = []
        self.pid = os.getpid()
        try:
            self.hname = socket.gethostname()
        except:
            self.hname = "UNKNOWN-HOST"
        self.fifo = None
        args = [
            {
                "key": pmix.PMIX_TOOL_NSPACE,
                "value": "DynaSched." + self.hname + '.' + str(self.pid),
                "val_type": pmix.PMIX_STRING,
            },
            {
                "key": pmix.PMIX_TOOL_RANK,
                "value": 0,
                "val_type": pmix.PMIX_PROC_RANK
            },
            {
                "key": pmix.PMIX_SERVER_SCHEDULER,
                "value": "T",
                "val_type": pmix.PMIX_BOOL
            },
            {
                "key": pmix.PMIX_CONNECT_TO_SYSTEM,
                "value": "T",
                "val_type": pmix.PMIX_BOOL
            },
            {
                "key": pmix.PMIX_TOOL_CONNECT_OPTIONAL,
                "value": "T",
                "val_type": pmix.PMIX_BOOL
            },
            {
                "key": pmix.PMIX_SERVER_REMOTE_CONNECTIONS,
                "value": "T",
                "val_type": pmix.PMIX_BOOL
            },
        ]
        rc, self.myproc = self.init(args)
        print("Init", self.error_string(rc), self.myproc)
        if pmix.PMIX_SUCCESS != rc:
            raise Exception('PMIx_tool_init failed: ', self.error_string(rc))
        try:
            map = {
                'toolconnected': server_upcalls.toolconnected,
                'query': server_upcalls.clientquery,
                'registerevents': server_upcalls.client_register_events,
                'allocate': server_upcalls.allocation,
                'sessioncontrol': server_upcalls.session_ctrl
            }
        except Exception as error:
            print("FAILED TO DEFDINE MAP")
            return
        try:
            rc = self.set_server_module(map)
        except Exception as error:
            print("FAILED TO MAP")
            return
        if pmix.PMIX_SUCCESS != rc:
            raise Exception('Setting server module failed: ', self.error_string(rc))

    def queue_init(self):
        try:
            self.fifo = FIFO()
            self.fifo.queue_init()
        except Exception as error:
            print('Unable to create FIFO instance')

    def queue_names(self):
        names = self.fifo.queue_names()
        return names

    def query_allocation(self):
        pyq = [{"keys": [pmix.PMIX_QUERY_ALLOCATION], "qualifiers": []}]
        rc, pyresults = self.query(pyq)
        if rc != pmix.PMIX_SUCCESS and rc != pmix.PMIX_OPERATION_SUCCEEDED:
            raise Exception("QUERY ALLOCATION FAILED: ", self.error_string(rc))
        darray = pyresults[0]["value"]["array"]
        for d in darray:
            if pmix.PMIX_NODE_INFO.decode() == d["key"]:
                n = d["value"]["array"]
                self.allocation.append(n)
            elif (
                pmix.PMIX_HWLOC_XML_V2.decode() == d["key"]
                or pmix.PMIX_HWLOC_XML_V1.decode() == d["key"]
            ):
                t = d["value"]
                self.topologies.append(t)
            else:
                print("ALLOCATION CONTAINS UNEXPECTED DATA", d["key"])
        print("ALLOCATION:", self.allocation)


if __name__ == "__main__":
    global killer
    killer = server_upcalls.GracefulKiller()
    main()
