LegoStore: Modular Multi-Version Transactional Key-Value Storage System v0.1

# 1.Overview
LegoStore is a modular distributed key-value storage system. In particular, LegoStore permits experimentation with different implementations for storage, transactional protocols, communication protocols, etc. Modularity is benefitial for two main reasons.

1) Key-value storage systems that have a lot of similarities can share common infrastructure. For example, if two key-value storage systems implemented in top of LegoStore require the same consistency level, both can share the same concurrency control protocol implementation.
2) LegoStore provides a common platform for efficiently comparing different implementations. In particular, developers might modify a specific module (e.g. storage module) and observe the difference in performance without touching any of the other modules (minimum changes required).

Developers can read a more detailed explanation of supported modules in the src directory. This README is oriented to explain installation and usage of LegoStore.

# 2.Prerequisites

# 3.Installation

# 4.Usage

