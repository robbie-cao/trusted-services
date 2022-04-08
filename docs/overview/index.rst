Introduction
============

The term 'trusted service' is used as a general name for a class of application that runs in an isolated
processing environment. Other applications rely on trusted services to perform security related operations in
a way that avoids exposing secret data beyond the isolation boundary of the environment. The word 'trusted'
does not imply anything inherently trustworthy about a service application but rather that other applications
put trust in the service. Meeting those trust obligations relies on a range of hardware and firmware
implemented security measures.

The Arm A-profile architecture, in combination with standard firmware, provides a range of isolated
processing environments that offer hardware-backed protection against various classes of attack. Because
of their strong security properties, these environments are suitable for running applications that have
access to valuable assets such as keys or sensitive user data. The goal of the Trusted Services project is
to provide a framework in which security related services may be developed, tested and easily deployed to
run in any of the supported environments. A core set of trusted services are implemented to provide basic
device security functions such as cryptography and secure storage.

Example isolated processing environments are:

    - **Secure partitions** - secure world VMs managed by a secure partition manager
    - **Trusted applications** - application environments managed by a TEE
    - **Integrated microcontroller** - a secondary MCU used as a secure enclave

For more background on the type of problems solved by trusted services and how the project aims to
make solutions more accessible, see:

.. toctree::
    :maxdepth: 1

    example-usage
    goals



--------------

*Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
