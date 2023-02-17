#!/usr/bin/env python3
#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

from server_upcalls import *
import os
import select
import subprocess

def main():
    try:
        foo = PMIxTool()
    except:
        print("FAILED TO CREATE SERVER")
        exit(1)
    print("Testing tool version ", foo.get_version())
    args = [{'key':PMIX_SERVER_SCHEDULER, 'value':'T', 'val_type':PMIX_BOOL},
            {'key':PMIX_LAUNCHER, 'value':'T', 'val_type':PMIX_BOOL},
            {'key':PMIX_CONNECT_TO_SYSTEM, 'value':'T', 'val_type':PMIX_BOOL},
            {'key':PMIX_TOOL_CONNECT_OPTIONAL, 'value':'T', 'val_type':PMIX_BOOL}]
    my_result = foo.init(args)
    print("Init", foo.error_string(my_result[0]))
    map = {'clientconnected': clientconnected,
           'clientfinalized': clientfinalized,
           'fencenb': clientfence,
           'publish': clientpublish,
           'unpublish': clientunpublish,
           'lookup': clientlookup,
           'query': clientquery}
    my_result = foo.set_server_module(map)
    print("Testing PMIx_Initialized")
    rc = foo.initialized()
    print("Initialized: ", rc)
    vers = foo.get_version()
    print("Version: ", vers)
    print("Testing PMIx_tool_is_connected")
    rc = foo.is_connected()
    print("Connected: ", foo.error_string(rc))

    # Register a fabric
    rc,fab = foo.fabric_register(None)
    print("Fabric registered: ", foo.error_string(rc))

    # setup the application
    (rc, regex) = foo.generate_regex(["test000","test001","test002"])
    print("Node regex, rc: ", regex, rc)
    (rc, ppn) = foo.generate_ppn(["0,1,2", "3,4,5", "6,7"])
    print("PPN, rc: ", ppn, foo.error_string(rc))
    darray = {'type':PMIX_INFO, 'array':[{'key':PMIX_ALLOC_FABRIC_ID,
                            'value':'SIMPSCHED.net', 'val_type':PMIX_STRING},
                           {'key':PMIX_ALLOC_FABRIC_SEC_KEY, 'value':'T',
                            'val_type':PMIX_BOOL},
                           {'key':PMIX_SETUP_APP_ENVARS, 'value':'T',
                            'val_type':PMIX_BOOL}]}
    kyvals = [{'key':PMIX_NODE_MAP, 'value':regex, 'val_type':PMIX_STRING},
              {'key':PMIX_PROC_MAP, 'value':ppn, 'val_type':PMIX_STRING},
              {'key':PMIX_ALLOC_FABRIC, 'value':darray, 'val_type':PMIX_DATA_ARRAY}]

    appinfo = []
    rc, appinfo = foo.setup_application("SIMPSCHED", kyvals)
    print("SETUPAPP: ", appinfo)

    rc = foo.setup_local_support("SIMPSCHED", appinfo)
    print("SETUPLOCAL: ", foo.error_string(rc))

    # get our environment as a base
    env = os.environ.copy()

    # register an nspace for the client app
    kvals = [{'key':PMIX_NODE_MAP, 'value':regex, 'val_type':PMIX_STRING},
             {'key':PMIX_PROC_MAP, 'value':ppn, 'val_type':PMIX_STRING},
             {'key':PMIX_UNIV_SIZE, 'value':1, 'val_type':PMIX_UINT32},
             {'key':PMIX_JOB_SIZE, 'value':1, 'val_type':PMIX_UINT32}]
    print("REGISTERING NSPACE")
    rc = foo.register_nspace("testnspace", 1, kvals)
    print("RegNspace ", foo.error_string(rc))

    # register a client
    uid = os.getuid()
    gid = os.getgid()
    print("REGISTERING CLIENT")
    rc = foo.register_client({'nspace':"testnspace", 'rank':0}, uid, gid)
    print("RegClient ", foo.error_string(rc))

    # setup the fork
    rc = foo.setup_fork({'nspace':"testnspace", 'rank':0}, env)
    print("SetupFrk", foo.error_string(rc))

    # setup the client argv
    args = ["./test/client.py"]
    # open a subprocess with stdout and stderr
    # as distinct pipes so we can capture their
    # output as the process runs
    p = subprocess.Popen(args, env=env,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # define storage to catch the output
    stdout = []
    stderr = []
    # loop until the pipes close
    while True:
        reads = [p.stdout.fileno(), p.stderr.fileno()]
        ret = select.select(reads, [], [])

        stdout_done = True
        stderr_done = True

        for fd in ret[0]:
            # if the data
            if fd == p.stdout.fileno():
                read = p.stdout.readline()
                if read:
                    read = read.decode('utf-8').rstrip()
                    print('stdout: ' + read)
                    stdout_done = False
            elif fd == p.stderr.fileno():
                read = p.stderr.readline()
                if read:
                    read = read.decode('utf-8').rstrip()
                    print('stderr: ' + read)
                    stderr_done = False

        if stdout_done and stderr_done:
            break

    print("FINALIZING")
    foo.finalize()

if __name__ == '__main__':
    global killer
    killer = GracefulKiller()
    main()
