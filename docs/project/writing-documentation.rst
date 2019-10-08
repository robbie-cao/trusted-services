Writing documentation
=====================

|FTB| is documented using `Sphinx`_, which in turn uses Docutils and `Restructured Text`_ (|REST| hereafter).

The source files for the documents are in the *docs* directory of the |FTB_REPO|.

The preferred output format is *HTML*, and other formats may or may not work.


Section Headings
----------------

In order to avoid problems if documents include each other, it is important to follow a consistent section heading
style. Please use at most five heading levels. Please use the following style::

    First-Level Title
    =================

    Second-Level Title
    ------------------

    Third-Level Title
    '''''''''''''''''

    Forth-level Title
    """""""""""""""""

    Fifth-level Title
    ~~~~~~~~~~~~~~~~~


Inline documentation
--------------------

To get all information integrated into a single document the project uses Sphinx extensions to allow capturing inline
documentation into this manual.


CMake
'''''

Apologies, this section is not ready yet. Please check existing cmake and |REST| files to see how documentation can be added.

--------------

.. _`Restructured Text`: https://docutils.sourceforge.io/rst.html
.. _`Sphinx`: https://www.sphinx-doc.org

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
