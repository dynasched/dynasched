#!/usr/bin/env python3
#
# Copyright (c) 2022      Nanook Consulting. All rights reserved
#

from setuptools import find_packages
from setuptools import setup

setup(
    name="dsched",
    install_requires="pluggy>=1.0",
    entry_points={"console_scripts": ["dsched=dsched.dsched:main"]},
    packages=find_packages(),
)
