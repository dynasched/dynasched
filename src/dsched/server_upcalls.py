#!/usr/bin/env python3
#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

from pmix import *
import signal, time
global killer

# where local data is published for testing
pmix_locdata = []

class GracefulKiller:
  kill_now = False
  def __init__(self):
    signal.signal(signal.SIGINT, self.exit_gracefully)
    signal.signal(signal.SIGTERM, self.exit_gracefully)

  def exit_gracefully(self,signum, frame):
    self.kill_now = True

def toolconnected(args:dict is not None):
    if 'nspace' in args:
        ns = args['nspace']
    else:
        ns = "dschedtool"

    results = {'nspace': ns, 'rank': 0}
    return PMIX_OPERATION_SUCCEEDED, results

def clientquery(args:dict is not None):
    print("SERVER: QUERY")
    # return a python info list of dictionaries
    info = {}
    results = []
    rc = PMIX_ERR_NOT_FOUND
    # find key we passed in to client, and
    # if it matches return fake PSET_NAME
    # since RM actually assigns this, we
    # just return arbitrary name if key is
    # found
    find_str = 'pmix.qry.psets'
    for q in args['queries']:
        for k in q['keys']:
            if k == find_str:
                info = {'key': find_str, 'value': 'PSET_NAME', 'val_type': PMIX_STRING}
                results.append(info)
                rc = PMIX_SUCCESS
    return rc, results

def client_register_events(args:dict is not None):
    print("CLIENT REGISTER EVENTS ", args['codes'])
    return PMIX_OPERATION_SUCCEEDED

def allocation(args:dict is not None):
    print("DSCHED ALLOCATE")
    results = []
    return PMIX_ERR_NOT_SUPPORTED, results

def session_ctrl(args:dict is not None):
    print("DSCHED SESSION CTRL")
    return PMIX_ERR_NOT_SUPPORTED

