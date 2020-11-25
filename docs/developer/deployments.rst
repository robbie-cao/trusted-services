Deployments
===========
In the context of the Trusted Services project, a deployment represents a unit of functionality
that is built for a specific environment.  You can find out more about deployments here
:ref:`Project Structure`.

The following table lists currently supported deployments:

.. list-table:: Supported deployments
  :header-rows: 1

  * - Deployment Name
    - Environments
    - Provides
    - Used for
  * - component-test
    - linux-pc, arm-linux
    - Standalone tests for components and integrations
    - Test driven development and regression testing
  * - ts-service-test
    - linux-pc, arm-linux
    - Service API level tests
    - Tests services from perspective of client application
  * - ts-demo
    - linux-pc, arm-linux
    - Demonstration client application
    - Example user-space client application
  * - libts
    - linux-pc, arm-linux
    - Provides standard client interface for locating services and establishing RPC sessions
    - Client application development
  * - crypto
    - opteesp
    - Crypto trusted service
    - Production deployments
  * - secure-storage
    - opteesp
    - Secure storage trusted service
    - Production deployments
  * - libsp
    - opteesp
    - FF-A interface library
    - FF-A library used in secure partition environments

--------------

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
