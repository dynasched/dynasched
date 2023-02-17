#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

import pluggy

hookimpl = pluggy.HookimplMarker("dsched")
"""Marker to be imported and used in plugins (and for own implementations)"""

