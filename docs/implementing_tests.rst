Implementing tests
==================

Concept of unit testing
-----------------------

First of all unit tests exercise the C code on a function level. The tests should call functions directly from the code under
tests and verify if their return values are matching the expected ones and the functions are behaving according to the
specification.

Because of the function level testing the dependencies of the tested functions should be detached. This is done by mocking the
underlying layer. This provides an additional advantage of controlling and verifying all the call to the lower layer.


Adding new unit test suite
--------------------------

The first step is to define a new unit test suite. If a completely new module is being test the test suite definition should be
created in a separate ``.cmake`` file which is placed in the test files' directory. Otherwise the test definition can be added
to an existing ``.cmake`` file. These files should be included in the root ``CMakeLists.txt``.

The ``UnitTest`` CMake module defines the ``unit_test_add_suite`` function so before using this function the module must be
included in the ``.cmake`` file. The function first requires a unique test name which will be test binary's name. The test
sources, include directories and macro definition are passed to the function in the matching arguments. CMake variables can be
used to reference files relative to common directories:

- ``CMAKE_CURRENT_LIST_DIR`` - Relative to the ``.cmake`` file
- :cmake:variable:`UNIT_TEST_PROJECT_PATH` - Relative to the project's root directory

.. code-block:: cmake

  # tests/new_module/new_test_suite.cmake
  include(UnitTest)

  unit_test_add_suite(
  	NAME [unique test name]
  	SOURCES
  		[source files]
  	INCLUDE_DIRECTORIES
  		[include directories]
  	COMPILE_DEFINITIONS
  		[defines]
  )

.. code-block:: cmake

  # Root CMakeLists.txt
  include(tests/new_module/new_test_suite.cmake)

Example test definition
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  unit_test_add_suite(
  	NAME memcmp
  	SOURCES
  		${CMAKE_CURRENT_LIST_DIR}/test_memcmp.cpp
  		${CMAKE_CURRENT_LIST_DIR}/memcmp.yml
  	INCLUDE_DIRECTORIES
  		${UNIT_TEST_PROJECT_PATH}/include
  		${UNIT_TEST_PROJECT_PATH}/include/lib/libc/aarch64/
  )


Using c-picker
--------------

c-picker is a simple tool used for detaching dependencies of the code under test. It can copy elements (i.e. functions,
variables, etc.) from the original source code into generated files. This way the developer can pick functions from compilation
units and surround them with a mocked environment.

If a ``.yml`` file listed among source files the build system invokes c-picker and the generated ``.c`` file is implicitly added
to the source file list.

Example .yml file
^^^^^^^^^^^^^^^^^

In this simple example c-picker is instructed to copy the include directives and the ``memcmp`` function from the
``lib/libc/memcmp.c`` file. The root directory of the source files referenced by c-picker is the project's root directory.

.. code-block:: yaml

  elements:
  - file: lib/libc/memcmp.c
    type: include
  - file: lib/libc/memcmp.c
    type: function
    name: memcmp


Writing unit tests
------------------

Unit test code should be placed in ``.cpp`` files.

Four-phase test pattern
^^^^^^^^^^^^^^^^^^^^^^^

All tests cases should follow the four-phase test pattern. This consists of four simple steps that altogether ensure the
isolation between test cases. These steps follows below.

- Setup
- Exercise
- Verify
- Teardown

After the teardown step all global states should be the same as they were at the beginning of the setup step.

CppUTest
^^^^^^^^

CppUTest is an open source unit testing framework for C/C++. It is written in C++ so all the useful features of the language is
available while testing. It automatically collects and runs the defined ``TEST_GROUPS`` and provides an interface for
implementing the four-phase test pattern. Furthermore the framework has assertion macros for many variable types and test
scenarios.

Include
'''''''

The unit test source files should include the CppUTest header after all other headers to avoid conflicts.

.. code-block:: C++

  // Other headers
  // [...]

  #include "CppUTest/TestHarness.h"

Test group
''''''''''

The next step is to define a test group. When multiple tests cases are written around testing the same function or couple
related functions these tests cases should be part of the same test group. Basically test cases in a test group share have same
setup/teardown sequence. In CppUTest the ``TEST_GROUP`` macro defines a new class which can contain member variables and
functions. Special setup/teardown function are defined using ``TEST_SETUP`` and ``TEST_TEARDOWN`` macros. These functions are
called before/after running each test case of the group so all the common initilization and cleanup code should go into these
functions.

.. code-block:: C++

  TEST_GROUP(List) {
  	TEST_SETUP() {
  		list = list_alloc();
  	}

  	TEST_TEARDOWN() {
  		list_cleanup(list);
  	}

  	bool has_element(int value) {
  		for (int i = 0; i < list_count (list); i++) {
  			if (list_get(i) == value) { return true; }
  		}
  		return false;
  	}

  	List* list;
  };


Test case
'''''''''

Test cases are defined by the ``TEST`` macro. This macro defines a child class of the test group's class so it can access the
member functions and variables of the test group. The test case's block itself is the body of the function of the child class.

.. code-block:: C++

  TEST(List, add_one) {
  	// Exercise
  	const int test_value = 5;
  	list_add(list, test_value);

  	// Verify using CHECK_TRUE assertion and TEST_GROUP member function
  	CHECK_TRUE(has_element(test_value));
  }

  TEST(List, add_two) {
  	// Exercise
  	const int test_value1 = 5;
  	const int test_value2 = 6;
  	list_add(list, test_value1);
  	list_add(list, test_value2);

  	// Verify
  	CHECK_TRUE(has_element(test_value1));
  	CHECK_TRUE(has_element(test_value2));
  }

CppUMock
^^^^^^^^

During unit testing the dependencies of the tested functions should be replaced by stubs or mocks. When using mocks the
developer would usually like to check if the function was called with corrent parameters and would like to return controlled
values from the function. When a mocked function is called multiple times from the tested function maybe it should check or
return different values on each call. This is where CppUMock comes handy.

All CppUMock interactions start with calling the ``mock()`` function. This returs a reference to the mocking system. At this
point the developer either wants to define expected or actual calls. This is achiveable by calling
``expectOneCall(functionName)`` or ``expectNCalls(amount, functionName)`` or ``actualCall(functionName)`` member functions of
``mock()`` call's return value. Registering expected calls are done in the test case before exercising the code and actual calls
happen from the mocked functions.

After this point the following functions can be chained:

- ``onObject(object)`` - In C++ it is usually the ``this`` pointer but it can be
  useful in C too.
- ``with[type]Parameter(name, value)`` - Specifying and checking call parameters
- ``andReturnValue(result)`` - Specifying return value when defining expected
  call
- ``return[type]Value()`` - Returning value from function

The mocking system has two important functions. ``mock().checkExpectation()`` checks if all the expected calls have been
fulfilled and and the ``mock().clear()`` removes all the expected calls from CppUMock's registry. These two functions are
usually called from the ``TEST_TEARDOWN`` function because there should not be any crosstalk between test cases through the
mocking system.

CppUMock's typical use-case is shown below by a simple example of the ``print_to_eeprom`` function.

.. code-block:: C++

  int eeprom_write(const char* str); /* From eeprom.h */

  int printf_to_eeprom(const char* format, ...) {
  	char buffer[256];
  	int length, written_bytes = 0, eeprom_write_result;
  	va_list args;

  	va_start(args, format);
  	length = vsnprintf(buffer, sizeof(buffer), format, args);
  	va_end(args);

  	if (length < 0) {
  		return length;
  	}

  	while(written_bytes < length) {
  		eeprom_write_result = eeprom_write(&buffer[written_bytes]);
  		if (eeprom_write_result < 0) {
  			return eeprom_write_result;
  		}
  		written_bytes += eeprom_write_result;
  	}

  	return written_bytes;
  }

Having the code snipped above a real life usage of the function would look like something shown in the following sequence
diagram.

.. uml:: resources/sequence_print_to_eeprom.puml

It would be really hard to test unit this whole system so all the lower layers should be separated and mock on the first
possible level. In the following example the ``print_to_eeprom`` function is being tested and the ``eeprom_write`` function is
mocked. In test cases where ``eeprom_write`` function is expected to be called the test case should first call the
``expect_write`` function. This registers an expected call to CppUMocks internal database and when the call actually happens it
matches the call parameters with the entry in the database. It also returns the previously specified value.

.. code-block:: C++

  TEST_GROUP(printf_to_eeprom) {
  	TEST_TEARDOWN() {
  		mock().checkExpectations();
  		mock().clear();
  	}

  	void expect_write(const char* str, int result) {
  		mock().expectOneCall("eeprom_write").withStringParameter("str", str).
  			andReturnValue(result);
  	}
  };

  /* Mocked function */
  int eeprom_write(const char* str) {
  	return mock().actualCall("eeprom_write").withStringParameter("str", str).
  		returnIntValue();
  }

  TEST(printf_to_eeprom, empty) {
  	LONGS_EQUAL(0, printf_to_eeprom(""))
  }

  TEST(printf_to_eeprom, two_writes) {
  	expect_write("hello1hello2", 6);
  	expect_write("hello2", 6);
  	LONGS_EQUAL(12, printf_to_eeprom("hello%dhello%d", 1, 2))
  }

  TEST(printf_to_eeprom, error) {
  	expect_write("hello", -1);
  	LONGS_EQUAL(-1, printf_to_eeprom("hello"))
  }

This how the ``printf_to_eeprom/two_writes`` test case's sequence diagram looks like after mocking ``eeprom_write``. The test
case became able to check the parameters of multiple calls and it could return controlled values.

.. uml:: resources/sequence_print_to_eeprom_mock.puml


Analyzing code coverage
-----------------------

The code coverage reports can be easily used for finding untested parts of the code. The two main parts of the coverage report
are the line coverage and the branch coverage. Line coverage shows that how many times the tests ran the given line of the
source code. It is beneficial to increase the line coverage however 100% line coverage is still not enough to consider the code
fully tested.

Let's have a look on the following example.

.. code-block:: C++

  void set_pointer_value(unsigned int id, unsigned int value) {
  	unsigned int *pointer;

  	if (id < MAX_ID) {
  		pointer = get_pointer(id);
  	}

  	*pointer = value;
  }

The 100% line coverage is achievable by testing the function with an ``id`` value smaller than ``MAX_ID``. However if an ``id``
larger than or equal to ``MAX_ID`` is used as a parameter of this function it will try to write to a memory address pointed by
an uninitialized variable. To catch untested conditions like this the branch coverage comes handy. It will show that only one
branch of the  ``if`` statement has been tested as the condition was always true in the tests.


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*
