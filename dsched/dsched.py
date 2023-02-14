#!/usr/bin/env python3
#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

# system imports
import os

# package imports
import pluggy

# project imports
import hookspecs
import server_upcalls
import pmix


def main():

    # get a plugin manager for queues - we don't really
    # know how people might want to organize queues, so
    # provide flexibility for the future
    qpm = get_plugin_manager()

    dsched = Scheduler(qpm.hook)
    print("Testing version ", dsched.get_version())

    # initialize the queues
    dsched.queue_init()
    qs = dsched.queue_names()
    print("QUEUES: ", qs)

    dsched.query_allocation()

    print("FINALIZING")
    dsched.finalize()


def get_plugin_manager():
    pm = pluggy.PluginManager("dsched")
    pm.add_hookspecs(hookspecs)
    pm.load_setuptools_entrypoints("dsched")
    return pm


class Scheduler(pmix.PMIxTool):
    def __init__(self, hook):
        self.hook = hook
        self.allocation = []
        self.topologies = []
        self.pid = os.getpid()
        args = [
            {
                "key": pmix.PMIX_TOOL_NSPACE,
                "value": "DynaSched." + str(self.pid),
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
        ]
        rc, self.myproc = self.init(args)
        print("Init", self.error_string(rc), self.myproc)
        map = {
            "query": server_upcalls.clientquery,
        }
        rc = self.set_server_module(map)
        if pmix.PMIX_SUCCESS != rc:
            print("Failed to set server module")
            exit(1)

    def queue_init(self):
        self.hook.queue_init()

    def queue_names(self):
        names = self.hook.queue_names()
        return names

    def query_allocation(self):
        pyq = [{"keys": [pmix.PMIX_QUERY_ALLOCATION], "qualifiers": []}]
        rc, pyresults = self.query(pyq)
        if rc != pmix.PMIX_SUCCESS and rc != pmix.PMIX_OPERATION_SUCCEEDED:
            print("QUERY ALLOCATION FAILED: ", self.error_string(rc))
            exit(1)
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
        print("ALLOCATION", self.allocation)
        print("TOPOLOGIES", self.topologies)


if __name__ == "__main__":
    global killer
    killer = server_upcalls.GracefulKiller()
    main()
