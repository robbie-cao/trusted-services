Change Log & Release Notes
==========================

This document contains a summary of the new features, changes, fixes and known issues in each release of Trusted
Services.

Version 1.0.0-Beta
------------------

The first tagged release of the project.

Feature Highlights
^^^^^^^^^^^^^^^^^^

The project supports the following services:

  - Secure Storage
  - Crypto
  - Initial Attestation
  - Smm Variable

Services may be accessed using client components that implement "`Psacertified v1.0 APIs`_". The project includes deployments
that integrate `PSA API certification tests`_ with API clients to facilitate end-to-end PSA certification testing.

Known limitations
'''''''''''''''''

  - Crypto key store partitioning by client is not yet supported.
  - Discovery support is only currently integrated into the Crypto service provider. In case of services not supporting
    this feature yet, communication parameters (e.g. maximum buffer size) and supported feature set needs to be hardcode
    to the service provider and service client.

Supported Trusted Environments
''''''''''''''''''''''''''''''

In the default configuration each service is deployed to a dedicated FF-A Secure Partition and executes isolated.
Service implementations are platform, trusted environment and service deployment agonistic. With appropriate enablement
work services can be enabled to work in any combination of these.

The reference integration uses the SPMC implemented in OP-TEE OS to manage TS SPs. This release supports `OP-TEE v3.19`_.

Supported Integration Systems
'''''''''''''''''''''''''''''

The reference solution uses the OP-TEE integration methodology. This relies on the google repo tool for high-level dependency
management and a set of makefiles to capture the build configuration information. For details please refer to
`OP-TEE git repo documentation`_.

The project is officially enabled in `Yocto meta-arm`_.

Supported Target Platforms
''''''''''''''''''''''''''

The only reference platform supported by this release is the `AEM FVP`_ build using the OP-TEE integration method.

Known limitations:

  - Non-volatile backend secure storage is not currently provided.


Test Report
^^^^^^^^^^^

Please find the Test Report covering this release in the `tf.org wiki`_.



--------------

.. _`FF-A Specification v1.0`: https://developer.arm.com/documentation/den0077/a
.. _`Psacertified v1.0 APIs`: https://www.psacertified.org/development-resources/building-in-security/specifications-implementations/
.. _`OP-TEE v3.19`: https://github.com/OP-TEE/optee_os/tree/3.19.0
.. _`Yocto meta-arm` : https://gitlab.oss.arm.com/engineering/yocto/meta-arm/-/tree/master/meta-arm/recipes-security/trusted-services
.. _`tf.org wiki`: https://developer.trustedfirmware.org/w/trusted-services/test-reports/v1.0.0-beta/
.. _`AEM FVP`: https://developer.arm.com/-/media/Files/downloads/ecosystem-models/FVP_Base_RevC-2xAEMvA_11.18_16_Linux64.tgz
.. _`PSA API certification tests`: https://github.com/ARM-software/psa-arch-tests
.. _`OP-TEE git repo documentation`: https://optee.readthedocs.io/en/latest/building/gits/build.html

*Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
