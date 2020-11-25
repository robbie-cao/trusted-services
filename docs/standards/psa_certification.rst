PSA Certification Requirements
==============================

Trusted services contribute to meeting PSA security requirements.  In the context of PSA certification, the scope and required
capabilities of supported trusted services are guided by:

    - The security primitives needed by system software for PSA level 1 checklist compliance.
    - Test coverage of the certification test suite used for PSA Functional API certification.

To determine what security primitives are needed, the 'system software' PSA scope has been used as a reference.  The current PSA
level 1 questionnaire is highly Cortex-M specific so some interpretation is needed to map requirements to Cortex-A based designs
that use a rich-OS such as Linux.  The following assessment is based on the assumption that the PSA system software scope
encompasses an entire Linux distribution.  The reference system software for the assessment will include:

    - TF-A.
    - Secure partition manager (OP-TEE with FF-A extensions performs this role).
    - Set of trusted services, deployed in S-EL0 secure partitions that are built-in to the OP-TEE image to allow early loading.
    - Normal-world boot loader.  EDK2 with Grub used as reference.
    - Linux kernel.
    - Root filesystem that includes generic services, libraries and program binaries.

Product developers will extend the base distribution to add application specific components.

PSA System Software Questionnaire Analysis
------------------------------------------

The following table lists PSA level 1 assessment questions with an attempt to identify the back-end security primitives that are
needed to meet the requirement.  The table is based on PSA Certified Level 1 Questionnaire Version 2.1.

.. list-table:: PSA Questionnaire Analysis
    :widths: 13 40 15 15 28
    :header-rows: 1

    * - Requirement
      - Summary
      - Depends on
      - TS used
      - Comment
    * - R1.1
      - | System software and application software update
        | verification using immutable root-of-trust.
      - Verify signed hash
      - Crypto
      - Depends on secure for persistent key storage.
    * - R1.2
      - | System software and application software update
        | anti-rollback protection.
      - Increment only NV counter
      - | Secure Storage (for protecting normal
        | world components)
      - | TF-A uses OTP fuses for early boot stages. This requirement is much more difficult to meet
        | on Cortex-A compared to M as
        | there are potentially many separately updatable components.
    * - R2.1
      - Protected device ID
      - Sign hash
      - Crypto
      - Depends on secure storage for persistent key storage.
    * - R2.2
      - Secure storage
      - Replay protected persistent storage, tied to device
      - Secure storage
      - | A filesystem mount that provided file storage with the required security properties would be
        | the conventional interface for user-space processes.
    * - R2.3
      - Crypto best practices
      - TRNG for random number and key generation
      - Crypto
      - Crypto service will need an entropy source.
    * - R3.1
      - Remote server authentication
      - Verify signed hash
      - Crypto
      - Depends on secure storage for persistent key storage.
    * - R3.2
      - Communication encryption
      - Kernel encryption support
      - None
      - Not a use-case for trusted services.
    * - R3.3
      - TLS authentication support
      - Sign & verify hash
      - Crypto
      - TLS library such as Mbed TLS needs to use Crypto service.
    * - R4.1
      - Provide attestation token for device life-cycle state
      - Secure life-cycle state variable, Sign message.
      - Attestation, Secure lifecycle, Crypto
      -
    * - R4.2
      - Unused functionality disabled
      - Secure life-cycle state variable
      - Secure Lifecycle
      - | Although the PSA questionnaire doesn't state this, in practice, the minimal capabilities
        | required will depend on the device lifecycle state. This will extend to all parts of the
        | system software, including trusted service capabilities.
    * - R4.3
      - Secure logging
      - Sign hash for non-repudiable log.
      - Crypto
      - | PSA questionnaire isn't specific about non-repudiation but for security auditing purposes,
        | this may be a requirement. For example, for industrial IoT applications where conformance
        | to IEC 62443 is important, verifiable log data that can be reliably traced, back to the
        | originating device is understood to be required.
    * - R4.4
      - Restricted log file access
      - File access control
      -	None
      - File access using standard Linux DAC or MAC if used.
    * - R4.5
      - Data on external interfaces checked defensively
      - Good coding practices in application components
      -
      - | The same could be applied for messages crossing any security boundary e.g. trusted service
        | interfaces. PSA only mentions external interfaces and critical APIs but for Cortex-A, this
        | likely to be insufficient.
    * - R4.6
      - Principle of least privileges
      - Use of Linux isolation, access control and other containment methods.
      -
      - | No direct dependency on trusted services but access to TS held assets must be controlled
        | according to principle of least privileges.
    * - R5.1
      - Passwords not resettable
      - Enforcing password policy
      -
      -
    * - R5.2
      - Password best practices
      - Enforcing password policy
      -
      -
    * - R5.3
      - User authentication using critical security parameters
      - Depends on auth method
      - Crypto (possibly)
      - | Requirement is not specific enough to determine if there is a trusted service dependency.
        | Dependency may be indirect, say to authenticate an Oauth2 server.
    * - R6.1
      - Externally applied configuration signed
      - Verify signed hash
      - Crypto
      - Similar to firmware update verification requirement.
    * - R7.1
      - Persistent storage of user data must support destructive erase
      - Destructive erase	secure Storage (possibly)
      - Secure storage
      - | File based storage with a filesystem mount with the required security properties would be the
        | conventional way to provide persistent storage to user-space processes.

Observations
------------

- All dependencies on the PSA Crypto service involve persistent keys.  Volatile key support is not needed for compliance.
- Applying R4.2 to system software doesn't really make sense.  The system software features that are needed are largely
  determined by the application software that implements the device functionality.  For a supplier of the base system software,
  a super-set of required functionality is likely to be the right offering but with the facility to allow features to be
  configured by device developers.  The ability to apply a configuration that limits functionality would be a more useful goal.
- Although the PSA certification document doesn't mention this, available device capabilities need to be a function of device
  life-cycle state.  For example, some features used during manufacture should not be available when a device is delivered to
  end users.
- It's hard to see anyone using the PSA Protected Storage API directly.  However, enabling a filesystem mount with equivalent
  security properties would be beneficial.  For example, a protected storage filesystem driver that uses the Protected Storage
  service as its back-end would be a good fit.  The PSA Protected Storage API would never be used directly by user-space
  applications.
- Logging with the guarantee of non-repudiation is really required to support audit use-cases.  Without that guarantee, it's
  just logging.

PSA Functional API Tests
------------------------

Functional API tests suites are currently available for:

    - PSA Crypto
    - PSA Attestation
    - PSA Storage

Running and passing API tests is not a requirement for PSA level 1 certification.  However, incorporating the API tests into the
trusted service test process is obviously beneficial as a way of demonstrating a defined set of functionality.

Functional API tests focus on valid behavior testing and are quite thorough in exercising all required functionality that can be
observed at each API.  To pass all tests, the service implementation needs to be complete, at least from a functional
perspective.

Test code depends on an implementation of the PSA C APIs for each service under test.

--------------

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
