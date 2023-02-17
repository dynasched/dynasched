#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

import pluggy

hookspec = pluggy.HookspecMarker("dsched")


@hookspec
def queue_init():
    """Initialize the queues in this plugin
    """


@hookspec
def queue_status(queues: list):
    """Return the status of the specified queues

    :param queues: the list of queues whose status
                   is being queried. None implies
                   return the status of ALL queues
                   Do not modify the list!
    :return: (plugin_name, [(queue name, status)])
    """


@hookspec
def queue_add(req: dict):
    """Add an allocation request to the queues. It
       is up to the implementation to select the
       precise queue in its storage

    :param req: a dictionary containing the request.
    :return: nothing
    """


@hookspec
def queue_num_pending(queues: list):
    """Return the number of pending requests in
       the specified queues

    :param queues: the list of queues whose status
                   is being queried. None implies
                   return the status of ALL queues
                   Do not modify the list!
    :return: (plugin_name, [(queue name, status)])
    """


@hookspec
def queue_names():
    """Return the names of all queues managed by
       this plugin

    :param none
    :return: (plugin_name, [queue_names])
    """
