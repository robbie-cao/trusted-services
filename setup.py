#!/usr/bin/env python3
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Setup file for c-picker.
"""

import importlib
import setuptools
import pkg_resources
from c_picker import __version__ as CPICKER_VERSION
from c_picker import __license__ as CPICKER_LICENSE

def get_install_requires():
    """
    Collecting dependencies on install.
    clang module can be part of the distro's package and it should not be
    overwritten by the clang pip package.
    """

    deps = ["pyyaml>5"]

    try:
        pkg_resources.get_distribution("clang")
        has_clang_pip = True
    except pkg_resources.DistributionNotFound:
        has_clang_pip = False

    clang_spec = importlib.util.find_spec("clang")
    has_clang_module = clang_spec is not None

    # Install clang Python bindings from pip if it has not been install
    # any other way. This condition prevents overwriting clang Python
    # modules if they were installed without pip (i.e. using the system's
    # package manager instead of pip).
    if not has_clang_module or has_clang_pip:
        deps.append("clang")

    return deps

setuptools.setup(
    name="c-picker",
    version=CPICKER_VERSION,
    author="Arm Limited",
    author_email="imre.kis@arm.com",
    license=CPICKER_LICENSE,
    description="C source code picker",
    platforms=["any"],
    packages=[
        "c_picker"
    ],
    install_requires=get_install_requires(),
    entry_points={
        "console_scripts": [
            'c-picker=c_picker.runner:main',
            'c-picker-coverage-mapper=c_picker.coverage:main'
        ],
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: BSD License",
        "Operating System :: OS Independent",
    ],
)
