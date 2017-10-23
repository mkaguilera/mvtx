LegoStore: Modular Multi-Version Transactional Key-Value Storage System v0.1
============================================================================

Overview
--------
LegoStore is a modular distributed key-value storage system. In particular, LegoStore permits experimentation with different implementations for storage, transactional protocols, communication protocols, etc. Modularity is benefitial for two main reasons.

1) Key-value storage systems that have a lot of similarities can share common infrastructure. For example, if two key-value storage systems implemented in top of LegoStore require the same consistency level, both can share the same concurrency control protocol implementation.
2) LegoStore provides a common platform for efficiently comparing different implementations. In particular, developers might modify a specific module (e.g. storage module) and observe the difference in performance without touching any of the other modules (minimum changes required).

Developers can read a more detailed explanation of supported modules in the src directory. This README is oriented to explain installation and usage of LegoStore.

Prerequisites
-------------
Instructions are suitable for Ubuntu 16.04 (possibly for other Ubuntu distributions as well)

* Protocol Buffers - This packet is needed for serialization/deserialization of data transfered from coordinators to servers and vice versa.  
  You can download the source and follow the instructions [here](https://github.com/google/protobuf/releases). Version >= 3.0.0 is needed.  

* Google RPC - This packet is needed for RPC from coordinators to servers.  
  You can download the source and follow the instructions [here](https://github.com/grpc/grpc/blob/master/INSTALL.md)  

* Packages `pkg-config`, `flex`, `bison` are also needed. Run the following command to install them:  
  `sudo apt install pkg-config flex bison`

Installation
------------
Simply follow the steps below.  
1. Download LegoStore from source (link missing).
2. `cd LegoStore`
3. `make`
4. `make shell` for installing a shell that helps experimentation with LegoStore (optional).

Usage
-----
