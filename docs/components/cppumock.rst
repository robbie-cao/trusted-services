CppUMock
========

CppUMock is the built-in mocking system of CppUTest. This chapter describes the basic features of the system. For details please
refer the `official CppUMock manual`_.

System functions
----------------

Checking expectations
^^^^^^^^^^^^^^^^^^^^^

After defining expected calls an invoking actual call the test should check if all the expected calls have happened. This can be
done by calling ``mock().checkExpectations()``. The common place to put this function call is the ``TEST_TEARDOWN`` function of
the test group.


Resetting mocking system
^^^^^^^^^^^^^^^^^^^^^^^^

After the last interaction with the mocking system the developer should reset the state of the system by calling
``mock().clear()``. The common place to put this function call is the ``TEST_TEARDOWN`` function of the test group after calling
``mock().checkExpectations()``.


Namespaces
^^^^^^^^^^

All interactions with the mocking system start by calling ``mock()``. This function has an option ``name`` string parameter
which can be used for limiting the scope of the mocking operation.

.. code-block:: C++

  mock("bl1").expectOneCall("bl1_main");


Enable/disable
^^^^^^^^^^^^^^

The mocking system can be enabled/disabled runtime using the functions below. This causes call expected and actual call to be
ignored. Default settings are restored by ``mock().clear()``.

- ``enable()``
- ``disable()``

.. code-block:: C++

  mock().disable();
  // All CppUMock calls are ignored after this point
  mock().enable();
  // CppUMock calls are working again

  mock().disable();
  // All CppUMock calls are ignored after this point
  [...]

  mock().clear();
  // Default settings are restored


String order
^^^^^^^^^^^^

After defining multiple expected function call the mocking system always The mocking system always uses the next matching
function when an actual call happens but by default it doesn't check if different function calls happen in the order of
definition. This behaviour can be turned on using the ``mock().strictOrder()`` function. This option is also set to default by
the ``mock().clear()`` function.

.. code-block:: C++

  mock().expectOneCall("A");
  mock().expectOneCall("B");

  mock().actualCall("B");
  mock().actualCall("A");
  // No error

  mock().strictOrder();
  mock().expectOneCall("A");
  mock().expectOneCall("B");

  mock().actualCall("B"); // Error generated here
  mock().actualCall("A");


Ignoring unspecified calls
^^^^^^^^^^^^^^^^^^^^^^^^^^

If there are addition actual calls happening in the test case which are unrelated to the test case (i.e. log or other messages)
the unspecified calls can be ignored by adding ``mock().ignoreOtherCalls()`` to the test case. This function should be used
carefully because it can hide unexpected call which are indicating errors in the code. The affect of this call ends by calling
``mock().clear()``.


Specifying object
-----------------

In object oriented environment member function should validate if the function call happened on the suitable object. This is
done by adding the following function to the mock specification.

- ``onObject(objectPtr)``


Validating parameters
---------------------

Each supported parameter type has a corresponding function. These are the same
in the expected and actual calls.

- ``withBoolParameter(name, bool value)``
- ``withIntParameter(name, int value)``
- ``withUnsignedIntParameter(name, unsigned int value)``
- ``withLongIntParameter(name, long int value)``
- ``withUnsignedLongIntParameter(name, unsigned long int value)``
- ``withDoubleParameter(name, double value)``
- ``withStringParameter(name, const char* value)``
- ``withPointerParameter(name, void* value)``
- ``withFunctionPointerParameter(name, void (*value)())``
- ``withConstPointerParameter(name, const void* value)``
- ``withMemoryBufferParameter(name, const unsigned char* value, size_t size)``

If custum types are defined and copier/comparator objects were installed the following function can handle these parameters.

- ``withParameterOfType(typeName, name, value)``

There's an option to copying data from the test environment into the mock function. When setting expectations the following
function can be used to set the pointer and the address of the data. **The mocking system will not create a copy of this data**
so the original data should be kept intact until the actual call happens.

- ``withOutputParameterReturning(name, value, size)``
- ``withOutputParameterOfTypeReturning(typeName, name, value)``

In the actual call the pair of these function are shown below.

- ``withOutputParameter(name, output)``
- ``withOutputParameterOfType(typeName, name, output)``


Ignoring parameters
^^^^^^^^^^^^^^^^^^^

There are cases when the developer doesn't want to specify all parameters. The following function can set this behaviour in the
expected call.

- ``ignoreOtherParameters()``


Specifying return values
------------------------

Using function name overloading the return values are specified by calling ``andReturnValue`` and the parameter type will
determine the exact function.

- ``andReturnValue(bool value)``
- ``andReturnValue(int value)``
- ``andReturnValue(unsigned int value)``
- ``andReturnValue(long int value)``
- ``andReturnValue(unsigned long int value)``
- ``andReturnValue(double value)``
- ``andReturnValue(const char* value)``
- ``andReturnValue(void* value)``
- ``andReturnValue(const void* value)``
- ``andReturnValue(void (*value)())``


Returning value in actual calls
-------------------------------

All of these function have version with ``OrDefault(type default_value)`` suffix. These version return a default value if the
return value was not specified in the expected call.

- ``bool returnBoolValue()``
- ``int returnIntValue()``
- ``unsigned int returnUnsignedIntValue()``
- ``long int returnLongIntValue()``
- ``unsigned long int returnUnsignedLongIntValue()``
- ``double returnDoubleValue()``
- ``const char * returnStringValue()``
- ``void * returnPointerValue()``
- ``const void * returnConstPointerValue()``
- ``void (*returnFunctionPointerValue())()``


Debugging CppUMock errors
-------------------------

Debugging CppUMock errors can be hard unlike assertion errors because a mocking failure can happen multiple layers of function
calls under the test case. The mocking system has a very similar feature to CppUTest's ``UT_CRASH()`` which is
``mock().crashOnFailure()``. By enabling this feature the code will crash on mocking errors and the developer could easily catch
it with the debugger.


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*

.. _`official CppUMock manual`: https://cpputest.github.io/mocking_manual.html
