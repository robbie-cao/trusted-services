Welcome to the c-picker's documentation!
========================================

.. toctree::
   :maxdepth: 1
   :hidden:
   :numbered:

   Home<self>
   user_guide
   project/index

This repository contains the c-picker-tool.

Separating dependencies apart from the code under test is a crutial step in unit testing systems. In many cases this can be
easily done by linking mocked functions to the tested code but sometimes it's difficult like when the code under test and its
dependencies are in the same compilation unit. For separating the code under test from its dependencies the tool called c-picker
can be used. It can pick pieces of code (functions, variables, etc.) based on descriptor files.

The coverage of c-picker generated sources is mapped to the original sources files.


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause