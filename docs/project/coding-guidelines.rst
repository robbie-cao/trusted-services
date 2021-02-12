Coding Style & Guidelines
=========================

The following sections contain |C_PICKER| coding guidelines. They are continually evolving and should not be considered "set
in stone". Feel free to question them and provide feedback.

The |C_PICKER| project uses multiple "domains" (textual content types, like programming languages) and each defines its own
rules.

To help configuring text editors the project comes with "`EditorConfig`_" file(s). (:download:`../../.editorconfig`).

Shared rules
------------

The following rules are common for all domains, except where noted otherwise:

#. Files shall be **UTF-8** encoded.
#. Use **Unix** style line endings (``LF`` character)
#. The primary language of the project is English. All comments and documentation must be in this language.
#. Trailing whitespace is not welcome, please trim these.

Python Domain
-------------

Python source code rules follow `PEP 8 -- Style Guide for Python Code`_.


Restructured Text Domain
------------------------

Please refer to :ref:`Writing documentation`.

--------------

.. _`EditorConfig`: https://editorconfig.org/
.. _`PEP 8 -- Style Guide for Python Code`: https://www.python.org/dev/peps/pep-0008/

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
