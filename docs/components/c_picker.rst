c-picker
========

c-picker uses ``libclang``'s Python interface for parsing source files.

Command line options
--------------------

- ``-h, --help`` - Showing help message
- ``--root ROOT`` - Root source directory
- ``--config CONFIG`` - Configuration file (``.json|.yml``)
- ``--output OUTPUT`` - Output file
- ``--print-dependencies`` - Prints the dependencies
- ``--version`` - Shows the program's version number and exit
- ``--args ...``  - clang arguments


Configuration file format
-------------------------

c-picker configuration can be described in ``JSON`` or ``YAML`` format using the same object structure.

- ``elements`` - List of elements to pick from the original code

  - ``name`` - Name of the element
  - ``type`` - Type of the element: ``include``, ``function``, ``variable``
  - ``args`` - Arguments for clang used during picking the element
  - ``options`` - Currenly the only options is ``remove_static`` which removes
    the ``static`` qualifier from the element's definition.

- ``options`` - Global options for all elements
- ``args`` - Global clang arguments for all elements

YAML example
------------

YAML format is preferred because it can contain metadata like comments and licence information.

.. code-block:: YAML

  elements:
  - file: bl1/bl1_fwu.c
    type: variable
    name: bl1_fwu_loaded_ids
    options: [remove_static]
  - file: bl1/bl1_fwu.c
    type: function
    name: bl1_fwu_add_loaded_id
    options: [remove_static]
  - file: bl1/bl1_fwu.c
    type: function
    name: bl1_fwu_remove_loaded_id
    options: [remove_static]
  args:
  - -DFWU_MAX_SIMULTANEOUS_IMAGES=10
  - -DINVALID_IMAGE_ID=0xffffffff


--------------

*Copyright (c) 2019-2021, Arm Limited. All rights reserved.*
