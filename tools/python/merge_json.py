#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (c) 2022, Arm Limited. All rights reserved.

"""
Merge two json files, the second is merged into the first. If the first file
doesn't exists yet, it will be created, along with its parent directories.
"""

import json
import os.path
import sys

if os.path.isfile(sys.argv[1]):
    with open(sys.argv[1], "rt", encoding="ascii") as f:
        combined = json.load(f)
else:
    os.makedirs(os.path.dirname(sys.argv[1]), exist_ok=True)
    combined = {}

with open(sys.argv[2], "rt", encoding="ascii") as f:
    current = json.load(f)
    combined = {**combined, **current}

with open(sys.argv[1], "wt", encoding="ascii") as f:
    json.dump(combined, f, indent=4)
