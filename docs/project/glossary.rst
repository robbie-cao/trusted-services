Glossary
========

This glossary provides definitions for terms and abbreviations used in the unit testing framework's documentation.

You can find additional definitions in the `Arm Glossary`_.

.. glossary::
   :sorted:

   CppUMock
      Built-in mocking system of CppUTest.

   CppUTest
      Open source C/C++ unit testing framework.

   FTB
      Firmware Test Builder

   LCS
     `Linux Coding Style`_

   Test case
      Single use case tested. Defined by ``TEST`` macro of CppUTest.

   Test group
      Multiple test cases with common setup/teardown steps and helper functions and variables. Defined by ``TEST_GROUP`` macro
      of CppUTest.

   Test suite
      Test binary which contains one or more test groups. Defined by :cmake:command:`unit_test_add_suite` CMake function.

--------------

*Copyright (c) 2020-2021, Arm Limited. All rights reserved.*

.. _`Arm Glossary`: https://developer.arm.com/support/arm-glossary
.. _`Linux Coding Style`: https://www.kernel.org/doc/html/v4.10/process/coding-style.html
