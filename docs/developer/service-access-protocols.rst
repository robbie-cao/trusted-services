Service Access Protocols
========================

A trusted service is accessed by calling service-specific methods via an RPC mechanism.  The set of callable methods forms the
public interface exposed by a service.  This section is concerned with interface conventions and protocols used for serializing
method parameters and return values. It is anticipated that there will be a need to support different parameter serialization
schemes to suite different needs.  The project accommodates this with the following:

    - The Protocols directory structure allows for different protocol definitions for the same service.
    - Message serialization code is decoupled from service provider code using an abstract 'serializer' interface.  Alternative
      concrete serializers may provide implementations of the interface.

RPC Session
-----------

Before a client can call trusted service methods, an RPC session must be established where an association is made between an RPC
Caller and a call endpoint that corresponds to the required service provider instance.  To establish the session, the client
must provide:

    - An identifier for the service provider instance.
    - Any client credentials that allow RPC layer access control to be applied if needed.

.. uml:: uml/RpcSessionClassDiagram.puml

Once the RPC session is established, the client may call service methods via an abstract RPC Caller interface that takes the
following parameters:

    - The opcode that identifies the method to call.
    - A buffer for the serialized method parameters.
    - A buffer for the serialized return values.

A deployment independent interface for locating services and establishing RPC sessions is described here: :ref:`Service Locator`

Status Codes
------------

On returning from a request to invoke a service method, two status codes are returned as follows:

  - *RPC status* - A generic status code that corresponds to the RPC call transaction.  RPC status codes are standardized across
    all services.
  - *Operation status* - a service specific status code.

Separation of status codes by layer allows service specific status codes to be accommodated while keeping RPC status codes
common.

A client should only check the returned operation status if the returned RPC status value is RPC_CALL_ACCEPTED.  All other RPC
status values indicate that an error occurred in delivering the RPC request.  An RPC status of RPC_CALL_ACCEPTED does not
indicate that the service operation was successful.  It merely indicates that the request was delivered, a suitable handler was
identified and the request parameters were understood.

Service Access Protocol Definition Conventions
----------------------------------------------

A service access protocol defines the following:

  - Opcodes used for identifying service methods.
  - Request parameters for each method.
  - Response parameters for method return values.
  - Operation status code.

Details of how public interface definition files for trusted services are organized, see: :ref:`Project Structure`

It is possible that for certain deployments, it will be necessary to customize which parameter encoding scheme is used.  Many
schemes are possible such as Protocol Buffers, CBOR, JSON, TLV, TPM commands or packed C structures.  To make scheme
customization straight forward, serilize/deserialize operations should be encapsulated behind a common interface to decouple
service provider code from any particular serialization scheme.  A section below describes a pattern for achieving this.

Service Namespace
'''''''''''''''''

Definitions that form a service access protocol should live within a namespace that is unique for the particular service.  Using
a namespace for service definitions avoids possible clashes between similarly named definitions that belong to different
services.  How the namespace is implemented depends on how the access protocol is defined.  For example, the Protocol Buffers
definitions for the crypto service all live within the ts_crypto package.  The recommended convention for forming a trusted
service namespace is as follows::

  ts_<service_name>

  e.g.
  ts_crypto
  ts_secure_storage

Language Independent Protocol Definitions
'''''''''''''''''''''''''''''''''''''''''

By defining service access protocols using an interface description language (IDL) with good support for different programming
languages, it should be straight forward to access trusted services from clients written in a range of languages.  On Arm
Cortex-A deployments, it is common for user applications to be implemented using a range of languages such as Go, Python or
Java.  Rather than relying on a binding to a C client library, native client code may be generated from the formal protocol
definition files. Initial protocol definitions use Google Protocol Buffers as the IDL.  The project structure allows for use of
alternative definition schemes and serializations.

Opcode Definition
`````````````````

Opcodes are integer values that identify methods implemented by a service endpoint.  Opcodes only need to be unique within the
scope of a particular service.  The mapping of opcode to method is an important part of a service interface definition and
should be readily available to clients written in a variety of programming languages.  For a Protocol Buffers based definition,
opcodes are defined in a file called::

  opcodes.proto

For example, for the Crypto trusted service, the Protocol Buffers opcode definitions are in::

  protocols/service/crypto/protobuf/opcodes.proto

Alternative definitions for light-weight C clients using the packed-c scheme are in::

  protocols/service/crypto/packed-c/opcodes.h

Parameter Definition
````````````````````

The convention used for serializing method parameters and return values may be specific to a particular service.  The definition
file will include message definitions for both request and response parameters. Common objects that are used for multiple
methods should be defined in separate files.  When using Protobufs, the following naming convention for method parameter files
should be used::

  <method_name>.proto

For example, the Crypto export_public_key method is defined in a file called::

  protocols/service/crypto/protobuf/export_public_key.proto

RPC Status Codes
````````````````

Generic RPC status code definitions using different definition schemes are defined here::

  protocols/rpc/common/protobuf/status.proto
  protocols/rpc/common/packed-c/status.h

Service Status Codes
````````````````````

Service specific status code definitions using different definition schemes are defined here (using crypto service as an
example)::

  protocols/service/crypto/protobuf/status.proto
  protocols/service/crypto/packed-c/status.h

Status code definitions may also be shared between services.  For example, services that conform to PSA API conventions will use
standardized PSA status codes, defined here::

  protocols/service/psa/protobuf/status.proto
  protocols/service/psa/packed-c/status.h

Use of Protocol Buffers
-----------------------

When Protocol Buffers is used for protocol definition and parameter serialization, the following conventions have been adopted.

.proto File Style Guide
'''''''''''''''''''''''

The style of the .proto files should follow Google's Protocol Buffers Style Guide.

Protocol Buffer Library for Trusted Services
''''''''''''''''''''''''''''''''''''''''''''

Protocol Buffers standardizes how service interfaces are defined and the on-wire encoding for messages. Because of this, service
clients and service providers are free to use any conformant implementation. However for trusted services that may be deployed
across a range of environments, some of which may be resource constrained, a lightweight library should be used for C/C++ code
that implement or use trusted services.  For this purpose, Nanobp (https://github.com/nanopb/nanopb) should be used.

Serialization Protocol Flexibility
----------------------------------

Many different serialization protocols exist for encoding and decoding message parameters.  Hard-wiring a particular protocol
into a trusted service provider implementation isn't desirable for the following reasons:

    - Depending on the complexity of serialization operations, mixing serialization logic with protocol-independent code makes
      trusted service provider code bigger and more difficult to maintain.
    - Different protocols may be needed for different deployments.  It should be possible to make a build-time or even a
      run-time selection of which protocol to use.
    - The number of supported serializations protocols is likely to grow.  Adding a new protocol shouldn't require you to make
      extensive code changes and definitely shouldn't break support for existing protocols.

These problems can be avoided by implementing protocol specific operations behind a common interface. Serialize/deserialize
operations will have the following pattern::

  int serialize_for_method(msg_buffer *buf, in args...);
  int deserialize_for_method(const msg_buffer *buf, out args...);

To extend a service provider to support a new serialization encoding, the following steps are required:

  1. Define a new encoding identifier string if a suitable one doesn't exist.  Currently used identifiers are protobuf and
     packed-c.  The identifier will be used as a directory name so it needs to be filename-friendly.  Some likely candidate
     identifiers could be cbor and json.
  2. Add a new RPC encoding ID to *protocols/rpc/common/packed-c/encoding.h*.  This is used by a caller to identify the encoding
     used for RPC parameters.  This is analogous to the content-type header parameter used in HTTP.
  3. Under the protocols parent directory, add a new access protocol definition for the service that needs extending.  This will
     be a representation of existing service access protocols but using a definition notation compatible with the new encoding.
  4. Add a new serializer implementation under the service provider's serializer directory e.g. for the crypto service -
     *components/service/crypto/provider/serializer*.
  5. Add registration of the new serializer to any deployment initialization code where the new encoding is needed.

--------------

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
