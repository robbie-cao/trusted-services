User guide
==========

This page describes how to get started with compiling and running unit tests on the host machine.

Host machine requirements
-------------------------

The system has been successfully tested on the following platforms:

- Ubuntu 19.04
- Ubuntu 18.04
- Arch Linux
- MSYS2 MinGW64

Tools
-----

The following applications are expected to be installed in the build machine:

- CMake >=3.11

- Python >=3.4

- c-picker

  - pyyaml

  - clang (pip package if not included in libclang)

  - libclang

- git

- Toolchain

- Native build system

Ubuntu 19.04
^^^^^^^^^^^^

On Ubuntu 19.04 use the following commands to install the required tools:

::

  sudo apt-get install cmake git python3 python3-pip libclang-dev build-essential
  sudo pip3 install git+[TBD]

Ubuntu 18.04
^^^^^^^^^^^^

The official Ubuntu 18.04 package repository only contains CMake 3.10 which not satisfies the requirements for building unit
tests. Fortunately there's an official CMake APT repository provided by Kitware: https://apt.kitware.com/ By adding this server
to the repository list an up-to-date version of CMake can be installed on Ubuntu 18.04.

::

  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc \
    2>/dev/null | sudo apt-key add -
  sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

  sudo apt-get install cmake git python3 python3-pip libclang-dev build-essential
  sudo pip3 install git+[TBD]

Arch Linux
^^^^^^^^^^

On Arch Linux use the following commands to install the required tools:

::

  sudo pacman -Sy cmake git python python-pip python-yaml make gcc clang
  sudo pip install git+[TBD]

MSYS2 MinGW64
^^^^^^^^^^^^^

::

  pacman -Sy mingw-w64-x86_64-cmake git mingw-w64-x86_64-python3 \
    mingw-w64-x86_64-python3-pip make mingw-w64-x86_64-gcc mingw-w64-x86_64-clang
  pip install git+[TBD]]


Integrating firmware test builder into a project
------------------------------------------------

The firmware test builder can be simply integrated into a CMake based project by adding the lines below to its CMakeLists.txt.
This example shows how to fetch the firmware test builder files on demand using CMake's FetchContent module.

::

  FetchContent_Declare(
    firmware_test_builder
    GIT_REPOSITORY ${FIRMWARE_TEST_BUILDER_URL}
    GIT_TAG ${FIRMWARE_TEST_BUILDER_REFSPEC}
    GIT_SHALLOW TRUE
  )

  FetchContent_GetProperties(firmware_test_builder)
  if(NOT firmware_test_builder_POPULATED)
    message(STATUS "Fetching Firmware Test Builder")
    FetchContent_Populate(firmware_test_builder)
  endif()

  # Appending Firmware Test Builder's cmake directory to CMake module path
  list(APPEND CMAKE_MODULE_PATH ${firmware_test_builder_SOURCE_DIR}/cmake)

For more details on adding tests see :ref:`Implementing tests` chapter.


Building unit tests
-------------------

Building unit tests start with running CMake which will check all the prerequisites and generate the native build system's input
files. This example uses Unix Makefiles. Unit tests exercise the code directly by compiling it into the same binary as the test
code.

::

  cd project
  mkdir build
  cd build
  cmake -G"Unix Makefiles" ..

After running the previous steps the makefiles are generated into the build directory so make can build unit tests. During unit
test development if only the source files have changed it's not necessary to re-run cmake it's only needed to run make as shown
below.

::

  make -j

For building single unit test suites the test's name can be used as a makefile target. Let's assume there's a test suite called
best_unit_tests.

::

  make best_unit_tests


Running unit tests
------------------

CMake provides a built-in tool called ctest for running all the tests using a single command. It is also able to filter tests or
run them in parallel for speeding up the tests process. Run all the tests using the following command:

::

  ctest

Each unit test suite has its own executable. The easiest way of running single test suite is running it as a simple executable.

::

  ./best_unit_tests


Debugging unit tests
--------------------

As it was mentioned in the previous section test suites are basically separate executables so they can be debugged as any other
native applications on the host machine. In a Linux environment gdb or IDE's built-in debugger can be utilized for debugging.

::

  gdb ./best_unit_tests


Measuring code coverage
-----------------------

Inspecting code coverage is a useful method for detecting parts of the code which is not exercised by tests. The build system
includes an option for generating code coverage report of the unit tests. The coverage is processed by ``lcov`` which needs to
be installed for this feature. Also the coverage measurement in only available when GCC is used as a compiler.

The methods for enabling coverage measurement is project dependent.

Before collecting coverage info and generating reports the tests must be run as the coverage is a runtime measurement. See
section `Running unit tests`_ for more information about running unit tests.

In case of enabled coverage report the system adds two new build targets called ``coverage`` and ``coverage_report``. They can
be used simply by running the following commands if ``make`` is used as a build system.

::

  make coverage
  make coverage_report

The ``coverage`` target generates lcov info files for further processing. If there are coverage files available from different
sources (i.e. coverages of other tests) they can be merged with the unit test coverage file and evaluated together.

The ``coverage_report`` target generates a HTML report from the coverage info files. The coverage reports can be found in the
build directory. The report shows the directory structure of the code and each file can be inspected individually. Line,
function and branch coverage is included.


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*
SPDX-License-Identifier: BSD-3-Clause
