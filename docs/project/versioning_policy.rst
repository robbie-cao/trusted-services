Versioning policy
==================

This document captures information about the version identifier used by the
project. It tells the meaning of each part, where the version information is
captured and how it is managed.

Summary
-------

The version identifier identifies the feature set supported by a specific
release, and captures compatibility information to other releases.

This project uses "Semantic Versioning", for details please refer to |SEMVER|.

In general the version number is constructed from three numbers. The `MAJOR`
number is changed when incompatible API changes are introduced, the `MINOR`
version when you functionality is added in a backward compatible manner, and
the `PATCH` version when backwards compatible bug fixes are added.

Each release will get a unique release id assigned. When a release is made, the
version number will get incremented in accordance with the compatibility rules
mentioned above.

This project is only using the core version and will not use pre-release or
build specific metadata extension.

Storage and format
------------------

The version number of each release will be stored at two locations:
  #. In a tag of the version control system in the form of "vX.Y.Z" where X Y
     and Z are the major, minor and patch version numbers.
  #. In a file called version.txt. This file uses ASCII encoding and will
     contain the version number as "X.Y.Z"  where X Y and Z are the major,
     minor and patch version numbers.

.. note:: The version id is independent from version identifiers of the
          versioning system used to store the |TS| (i.e. git).

--------------

.. _`Semantic Versioning`: https://semver.org/spec/v2.0.0.html

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
