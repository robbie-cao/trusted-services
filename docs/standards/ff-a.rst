Firmware Framework for Armv8-A
==============================

|FF-A| is a standard which *"describes interfaces that standardize communication between the various software images. This
includes communication between images in the Secure world and Normal world."*

Trusted Services is the home of the FF-A S-EL0 Secure Partitions implementing PSA functionality. The component :ref:`libsp`
captures helpful abstractions to allow easy FF-A compliant S-EL0 SP development. S-EL0 SPs are SPMC agonistic and can be used
with an SPMC running in any higher secure exception level (S-EL1 - S-EL3). Currently the solution is tested with an SPMC
running at S-SEL1 integrated into OP-TEE OS.

--------------

.. _`PSA homepage`: https://developer.arm.com/architectures/security-architectures/platform-security-architecture
.. _`www.psacertified.org`: https://www.psacertified.org/certified-products/
.. _`Hafnium project`: https://www.trustedfirmware.org/projects/hafnium/

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
