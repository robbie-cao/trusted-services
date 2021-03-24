Documentation Build Instructions
================================

To create a rendered copy of this documentation locally you can use the
`Sphinx`_ tool to build and package the plain-text documents into HTML-formatted
pages.

If you are building the documentation for the first time then you will need to
check that you have the required software packages, as described in the
*Prerequisites* section that follows.

Prerequisites
-------------

For building a local copy of the |TS| documentation you will need, at minimum:

- GNUMake
- Python 3 (3.5 or later)
- PlantUML (1.2017.15 or later)

You must also install the Python modules that are specified in the
``requirements.txt`` file in the root of the ``docs`` directory. These modules
can be installed using ``pip3`` (the Python Package Installer). Passing this
requirements file as an argument to ``pip3`` automatically installs the specific
module versions required.

Example environment
-------------------

An example set of installation commands for Linux with the following assumptions:
    #. OS and version: Ubuntu 18.04 LTS
    #. `virtualenv` is used to separate the python dependencies
    #. pip is used for python dependency management
    #. `bash` is used as the shell.

.. code:: shell

    sudo apt install make python3 python3-pip virtualenv python3-virtualenv plantuml
    virtualenv -p python3 ~/sphinx-venv
    . ~/sphinx-venv/bin/activate
    pip3 install -r requirements.txt
    deactivate

.. note::
   More advanced usage instructions for *pip* are beyond the scope of this
   document but you can refer to the `pip homepage`_ for detailed guides.

.. note::
   For more information on Virtualenv please refer to the `Virtualenv documentation`_

Building rendered documentation
-------------------------------

From the ``docs`` directory of the project, run the following commands.

.. code:: shell

   . ~/sphinx-venv/bin/activate
   make clean
   make
   deactivate

Output from the build process will be placed in:

::

   <tf-a CMF root>/docs/_build/html/

--------------

.. _Sphinx: http://www.sphinx-doc.org/en/master/
.. _pip homepage: https://pip.pypa.io/en/stable/
.. _`Virtualenv documentation`: https://virtualenv.pypa.io/en/latest/

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
