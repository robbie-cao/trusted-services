CppUTest
========

This document is based on CppUTest v3.8. CppUtest is a unit testing framework for testing C and C++ code. This document
introduces the basic features of the framework. For further information check the `official manual of CppUTest`_.


Why CppUTest?
-------------

First of all it was not our goal to develop a new unit testing framework while plenty of open source solutions are already
available. There were no special requirements agains the unit testing framework that would rule out all existing frameworks so
we only had to choose a suitable one for our current and possible future needs.

We ended up selecting CppUTest because of its small footprint and easy portability. It also goes along with the standard xUnit
frameworks' principles and provides a standard interface to the outside world. Some details are listed below.

- C/C++ support
- Small footprint (compared to Google Test)
- Easy to use on embedded systems
- Built-in mocking system (CppUMock)
- Implements four-phase testing pattern (setup, exercise, verify, teardown)
- Selective run of test cases
- Standard output format (JUnit, TeamCity)


Assertions
----------

Generally is a good practice to use more specific assertions because it can output more informative error messages in case of a
failure. The generic form or assertions is ``assert(expected, actual)``. Each assert type has a _TEXT variant for user defined
error messages as last parameter.

- Boolean

  - ``CHECK(condition)`` - Same as ``CHECK_TRUE``
  - ``CHECK_TRUE(condition)``
  - ``CHECK_FALSE(condition)``

- C string

  - ``STRCMP_EQUAL(expected, actual)``
  - ``STRNCMP_EQUAL(expected, actual, length)``
  - ``STRCMP_NOCASE_EQUAL(expected, actual)``
  - ``STRCMP_CONTAINS(expected, actual)``
  - ``STRCMP_NOCASE_CONTAINS(expected, actual)``

- Integer

  - ``LONGS_EQUAL(expected, actual)``
  - ``UNSIGNED_LONGS_EQUAL(expected, actual)``
  - ``LONGLONGS_EQUAL(expected, actual)``
  - ``UNSIGNED_LONGLONGS_EQUAL(expected, actual)``
  - ``BYTES_EQUAL(expected, actual)``
  - ``SIGNED_BYTES_EQUAL(expected, actual)``
  - ``POINTERS_EQUAL(expected, actual)``
  - ``FUNCTIONPOINTERS_EQUAL(expected, actual)``

- Enums

  - ``ENUMS_EQUAL_INT(expected, actual)``
  - ``ENUMS_EQUAL_TYPE(underlying_type, expected, actual)``

- Other assertions

  - ``CHECK_EQUAL(expected, actual)`` - Requires ``operator=`` and ``StringFrom(type)`` to be implemented
  - ``CHECK_COMPARE(first, relop, second)`` - Same as ``CHECK_EQUAL`` but with any type of compare
  - ``DOUBLES_EQUAL(expected, actual, threshold)``
  - ``MEMCMP_EQUAL(expected, actual, size)``
  - ``BITS_EQUAL(expected, actual, mask)``
  - ``FAIL()`` or ``FAIL_TEST()`` - Test case fails if called
  - ``CHECK_THROWS(expected, expression)`` - Catching C++ exceptions

- Miscellaneous macros

  - ``IGNORE_TEST`` - Same as ``TEST`` but it’s not called
  - ``TEST_EXIT`` - Exists test
  - ``UT_CRASH()`` - Crashes the test which is easy to catch with debugger
  - ``UT_PRINT(text)`` - Generic print function


Test runner
-----------

Test cases are collected automatically. Under the hood the ``TEST`` macros are creating global instances of classes and their
constructor registers the test cases into the test registry. This happens before entering the ``main`` function. In the ``main``
function only the ``RUN_ALL_TESTS`` macro needs to be placed with the command line arguments passed to it. On executing the
binary the command line arguments will control the behaviour of the test process.

.. code-block:: C++

  #include "CppUTest/CommandLineTestRunner.h"

  int main(int argc, char* argv[]) {
  	return RUN_ALL_TESTS(argc, argv);
  }

The default ``main`` implementation is added to all unit test suites by the
build system.


Command line options
--------------------

Command line options are available mainly for configuring the output format of
the test binary and for filtering test groups or cases.

- Output

  - ``-v`` - Prints each test name before running them
  - ``-c`` - Colorized output
  - ``-o{normal, junit, teamcity}`` - Output format, junit can be processed by
    most CIs
  - ``-k packageName`` - Package name for junit output
  - ``-lg`` - List test groups
  - ``-ln`` - List test cases

- Other

  - ``-p`` - Runs each test case in separate processes
  - ``-ri`` - Runs ignored test cases
  - ``-r#`` - Repeats testing ``#`` times
  - ``-s seed`` - Shuffles tests

- Filtering test cases

  - ``"TEST(groupName, testName)"`` - Running single test
  - ``"IGNORE_TEST(groupName, testName)"`` -- Running single ignored test
  - ``-g text`` - Runing groups containing text
  - ``-n text`` - Runing tests containing text
  - ``-sg text`` - Runing groups matching text
  - ``-sn text`` - Runing tests matching text
  - ``-xg text`` - Excluding groups containing text
  - ``-xn text`` - Excluding tests containing text
  - ``-xsg text`` - Excluding groups matching text
  - ``-xsn text`` - Excluding tests matching text


Troubleshooting
---------------

Output messages
^^^^^^^^^^^^^^^

When one of tests fails the first step is to run it separately and check its
output message. Usually it shows the exact line of the file where the error
happened.

::

  test_memcmp.cpp:17: error: Failure in TEST(memcmp, empty)
    expected <1 0x1>
    but was  <0 0x0>

The executed tests can be followed by adding ``-v`` command line option.

::

  ./memcmp -v
  TEST(memcmp, different) - 0 ms
  TEST(memcmp, same) - 0 ms
  TEST(memcmp, empty) - 0 ms

  OK (3 tests, 3 ran, 1 checks, 0 ignored, 0 filtered out, 0 ms)


Catching failure with debugger
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If a failure happens in a helper function or in a loop where the assertion
is called multiple times it is harder to get the exact environment of a failure.
In this case it's a good practice to put a ``UT_CRASH()`` call into a
conditional block which hits when the failure happens. This way the debugger can
stop on failure because the code emits a signal.

.. code-block:: C++

  TEST(magic, iterate) {
  	int result;

  	for(int i = 0; i < 1000; i++) {
  		result = magic_function(i);

  		// Debug code
  		if (result) {
  			UT_CRASH();
  		}

  		LONGS_EQUAL(0, result);
  	}
  }


Using ``FAIL`` macro
^^^^^^^^^^^^^^^^^^^^

It's recommended to use ``FAIL`` macro in conditions that should never occur in
tests. For example if a test case loads test data from an external file but the
file could not be opened the ``FAIL`` macro should be used with an informative
message.

.. code-block:: C++

  fd = open("test.bin", O_RDONLY);
  if (fd < 0) {
  	FAIL("test.bin open failed");
  }


Interference between test cases
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Test cases can interfere if there's a global resource which was not restored to
its original state after leaving a test case. This can be hard to find but at
least the it's easy to make sure that this is root case of an error. Let's
assume there's a global variable which is set during the test case but it
original value is not restore at the end. CppUTest has an command line option
for running each test case in separate process. This makes the global variable
to have its original value at the beginning of the test cases. Basically if the
test works by passing argument ``-p`` when running but fails without it, there's
a good chance for having an interference between test cases.

.. code-block:: C++

  int x = 0;

  TEST_GROUP(crosstalk) {
  };

  TEST(crosstalk, a) {
  	LONGS_EQUAL(0, x);
  	x = 1;
  }

  TEST(crosstalk, b) {
  	LONGS_EQUAL(0, x);
  	x = 1;
  }

  TEST(crosstalk, c) {
  	LONGS_EQUAL(0, x);
  	x = 1;
  }

By running the test executable with different command line arguments it produces
a different result.

.. code-block::

  ./crosstalk -v

  TEST(crosstalk, c) - 0 ms
  TEST(crosstalk, b)
  test_crosstalk.cpp:37: error:
  Failure in TEST(crosstalk, b)
  	expected <0 0x0>
  	but was  <1 0x1>

   - 0 ms
  TEST(crosstalk, a)
  test_crosstalk.cpp:32: error: Failure in TEST(crosstalk, a)
  	expected <0 0x0>
  	but was  <1 0x1>

   - 0 ms

  Errors (2 failures, 3 tests, 3 ran, 3 checks, 0 ignored, 0 filtered out, 0 ms)

  ./crosstalk -v -p
  TEST(crosstalk, c) - 1 ms
  TEST(crosstalk, b) - 0 ms
  TEST(crosstalk, a) - 0 ms

  OK (3 tests, 0 ran, 0 checks, 0 ignored, 0 filtered out, 2 ms)


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*

.. _`official manual of CppUTest`: https://cpputest.github.io/
