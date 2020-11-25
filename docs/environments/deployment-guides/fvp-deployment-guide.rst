Deploying Programs on FVP
=========================
This page explains how to load and run user space programs on a Linux image running in FVP simulation.
The loaded programs may use any trusted services that are available as part of the image firmware.

To prepare and run an image that includes trusted services running in S-EL0 secure partitions under
OP-TEE. see:
:ref:`Deploying trusted services in S-EL0 Secure Partitions under OP-TEE`

Shared directory
----------------
The OP-TEE image built for the FVP virtual platform supports directory sharing between the running
OS and the host computer.  This provides a convenient way to transfer files between the host and
the device simulation.  When the FVP is run using the *run-only* target in *fvp.mk*, the shared
directory is set-up automatically.

On the host, the shared directory is located here::

    optee/shared

On the simulated device, running under FVP, a mount is created here::

    /mnt

Deploying service level tests
-----------------------------
As an example of how to load and run programs, the *ts-service-test* binary executable is used.
The build file for the *arm-linux* deployment of *ts-service-test* lives under::

    trusted-services/deployments/ts-service-test/arm-linux

The executable includes service level test cases that exercise trusted services via their
standard interfaces.  Test cases use *libts* for locating services and establishing RPC
sessions.  *ts-service-test* provides a useful reference for understanding how *libts* may
be used for accessing trusted services.

The build file for the *arm-linux* deployment of *libts* lives under::

    trusted-services/deployments/libts/arm-linux

Trusted service build instructions are here:
:ref:`Build Instructions`

Having built *ts-service-test* and *libts* for the *arm-linux* environment, the steps
are required.

Copy files to share directory
'''''''''''''''''''''''''''''
Assuming that the *OP-TEE* and *trusted-services* projects are located under a common
parent directory, the following files need to be copied from the host using::

    cp trusted-services/deployments/libts/arm-linux/build/install/lib/libts.so optee/shared/.
    cp trusted-services/deployments/ts-service-test/arm-linux/build/install/bin/ts-service-test optee/shared/.

Installing libts.so
'''''''''''''''''''
Having copied *libts.so* to the shared directory, it needs to be copied to one of the standard
lib directories so that it is located when a dependent program is started.  From the terminal
command prompt for the booted FVP, use::

    cd /mnt
    cp libts.so /usr/lib/.

Running the program executable
''''''''''''''''''''''''''''''
The *ts-service-test* program may be run directly from the */mnt* directory  using::

    ./ts-service-test -v

If all is well, you should see something like::

    TEST(E2EcryptoOpTests, generateRandomNumbers) - 3 ms
    TEST(E2EcryptoOpTests, asymEncryptDecrypt) - 8 ms
    TEST(E2EcryptoOpTests, signAndVerifyHash) - 29 ms
    TEST(E2EcryptoOpTests, exportAndImportKeyPair) - 17 ms
    TEST(E2EcryptoOpTests, exportPublicKey) - 10 ms
    TEST(E2EcryptoOpTests, generatePersistentKeys) - 34 ms
    TEST(E2EcryptoOpTests, generateVolatileKeys) - 16 ms

    OK (7 tests, 7 ran, 56 checks, 0 ignored, 0 filtered out, 117 ms)

--------------

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
