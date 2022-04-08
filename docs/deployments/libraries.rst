Libraries
=========
Some deployments build common functionality into libraries that may be used by
other deployments or external applications. The following library deployments
are currently supported:

libts
-----
Userspace applications that depend on trusted services may use *libts* for handling
service discovery and RPC messaging. A major benefit to application developers is
that *libts* entirely decouples client applications from details of where a service
provider is deployed and how to communicate with it. All TS test executables and
tools that interact with service providers use *libts*.

To facilitate test and development within a native PC environment, the *libts*
deployment for the *linux-pc* environment integrates a set of service providers
into the library itself. From a client application's perspective, this looks
exactly the same as when running on a target platform with service providers
deployed in secure processing environments. For more information, see:
:ref:`Service Locator`.

.. list-table::
  :widths: 1 2
  :header-rows: 0

  * - Supported Environments
    - * | *linux-pc* - service providers integrated into library
      * | *arm-linux* - communicates with service providers in secure processing environment
  * - Used by
    - * Userspace applications

libsp
-----
*libsp* provides a functional interface for using FF-A messaging and memory
management facilities. *libsp* is used in SP deployments. For more information, see:
:ref:`libsp`.

.. list-table::
  :widths: 1 2
  :header-rows: 0

  * - Supported Environments
    - * | *opteesp*
  * - Used by
    - * Secure partitions

--------------

*Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
