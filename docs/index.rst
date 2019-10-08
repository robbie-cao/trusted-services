Welcome to the Firmware Test Builds's documentation!
====================================================

.. toctree::
   :maxdepth: 1
   :hidden:
   :numbered:

   Home<self>
   user_guide
   implementing_tests
   components/index
   project/index

This repository contains the unit testing framework.

These tests are meant to run on host machine and used to cover platform independent code on the unit test level. In this case a
unit is considered to be a C function or couple related functions. Each unit test suite compiles into a binary which can be run
and debugged as any ordinary executable.

The system uses CppUTest as unit testing framework. The tests are written in C++ in order to be able to use CppUTests' all
useful features like the automatic collection of test cases and the CppUMock mocking system.

Separating dependencies apart from the code under test is a crutial step in unit testing systems. In many cases this can be
easily done by linking mocked functions to the tested code but sometimes it's difficult like when the code under test and its
dependencies are in the same compilation unit. For separating the code under test from its dependencies a tool called c-picker
can be used. It can pick pieces of code (functions, variables, etc.) based on descriptor files.

The build system is based on CMake. The repository contains CMake modules for defining unit test suites. It also invokes
c-picker if a descriptor file is listed among the test sources. CMake has a built in test driver system called ctest. It runs
all the test binaries and produces an well structured output. Test filtering and parallel test run is also available.

For measuring unit test coverage lcov is utilized. The coverage of c-picker generated sources is mapped to the original sources
files. Coverage currently only works with GCC.

As a next step start with reading the :ref:`User guide` and the :ref:`Implementing tests` section of this manual. For detailed
description of the components check the :ref:`Component user manuals` section.


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
