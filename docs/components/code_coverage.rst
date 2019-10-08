Code coverage
=============

Coverage processing flow
------------------------

1. Prerequisites

   1. Building all or selected test binaries with coverage enabled

   2. Running all or selected test binaries

2. Collecting coverage data from ``.gcda`` and ``.gcno`` file into ``lcov`` coverage info file

3. Mapping c-picker generated files' coverage to the original source lines

4. Filtering coverage data for separating the coverage of the code under tests and the coverage of the test code

5. Generating HTML coverage report from the filtered lcov info files


.. cmake-module:: ../../cmake/Coverage.cmake


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*
