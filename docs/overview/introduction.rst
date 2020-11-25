What are trusted services?
==========================

The term 'trusted service' is used as a general name for a class of application that runs in an isolated
processing environment.  Other applications rely on trusted services to perform security related operations in
a way that avoids exposing secret data beyond the isolation boundary of the environment.  The word 'trusted'
does not imply anything inherently trustworthy about a service application but rather that other applications
put trust in the service.  Meeting those trust obligations relies on a range of hardware and firmware
implemented security measures.

The Arm architecture, in combination with standard firmware, provides a range of isolated processing environments
that offer hardware-backed protection against various classes of attack.  Because of their strong security
properties, these environments are suitable for running applications that have access to valuable assets
such as keys or sensitive user data.  The goal of the Trusted Services project is to provide a framework in
which security related services may be developed, tested and easily deployed to run in any of the supported
environments.  A core set of trusted services are implemented to provide basic device security functions such
as cryptography and secure storage.

Example isolated processing environments are:

    - **Secure partitions** - secure world VMs managed by a secure partition manager
    - **Trusted applications** - application environments managed by a TEE
    - **Integrated microcontroller** - a secondary MCU used as a secure enclave

Typical problems solved by trusted services
-------------------------------------------

The following are examples of how trusted services can solve common device security problems.


Protecting IoT device identity
''''''''''''''''''''''''''''''

During the provisioning process, an IoT device is assigned a secure identity that consists of a public/private
key pair and a CA signed certificate that includes the public key.  The device is also provisioned with the
public key corresponding to the cloud service that it will operate with.  The provisioned material is used
whenever a device connects to the cloud during the authentication process.  To prevent the possibility
of device cloning or unauthorized transfer to a different cloud service, all provisioned material must be
held in secure storage and access to the private key must be prevented.  To achieve this, the certificate
verification and nonce signing performed during the TLS handshake is performed by the Crypto trusted service
that performs the operations without exposing the private key.


Protecting Software Updates
'''''''''''''''''''''''''''

To ensure that software updates applied to a device originate from a legitimate source, update packages are
signed.  A signed package will include a signature block that includes a hash of the package contents within
the signed data.  During the update process, a device will verify the signature using a provisioned public key
that corresponds to the signing key used by the update originator.  By holding the public key in secure storage
and performing the signature verification using the Crypto service, unauthorized modification of the update
source is prevented.


Secure Logging
''''''''''''''

A managed IoT device will often be configured by an installation engineer who has physical access to the
device.  To allow a cloud operator to audit configuration changes, it is necessary to keep a log of
configuration steps performed by the installation engineer.  To avoid the possibility of fraudulent
modification of the audit log, a device signs log data using a device unique key-pair.  The public key
corresponding to the signing private key may be retrieved by the cloud operator to allow the log to
be verified.  To protect the signing key, the Crypto service is used for signing log records.

--------------------

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
