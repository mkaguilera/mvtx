LegoStore: Modular Multi-Version Transactional Key-Value Storage System v0.1

# 1. Overview
LegoStore is a modular distributed key-value storage system. In particular, LegoStore permits experimentation with different implementations for storage, transactional protocols, communication protocols, etc. Modularity is benefitial for two main reasons.

1) Key-value storage systems that have a lot of similarities can share common infrastructure. For example, if two key-value storage systems implemented in top of LegoStore require the same consistency level, both can share the same concurrency control protocol implementation.
2) LegoStore provides a common platform for efficiently comparing different implementations. In particular, developers might modify a specific module (e.g. storage module) and observe the difference in performance without touching any of the other modules (minimum changes required).

Developers can read a more detailed explanation of supported modules in the src directory. This README is oriented to explain installation and usage of LegoStore.

# 2. Build Instructions

To build FFFS, please download [hadoop 2.4.1 source code](https://archive.apache.org/dist/hadoop/core/hadoop-2.4.1/hadoop-2.4.1-src.tar.gz); apply it with the patch in `sources/fffs-for-hadoop-2.4.1-src.patch.tgz`. Then, build the patched source code.

## 2.1 System Requirement

Although we only tried CentOS 6.5 and Ubuntu 16.04/14.04/12.10, FFFS can be built by any recent linux distribution. Please make sure you have at least 10G diskspaces and have the following software installed:
* gcc, g++, and make
* cmake >= version 2.8.0
* openssl development package (CentOS: sudo yum install openssl openssl-devel; Ubuntu: sudo apt-get install openssl libssl-dev)
* zlib development package (CentOS: sudo yum install zlib zlib-devel; Ubuntu: sudo apt-get install zlib1g zlib1g-dev)
* protobuf-2.5.0
* Oracle Java SE 7
* Apache Ant 1.9.6
* Apache Maven 3.1.1
* [OFED](http://downloads.openfabrics.org/OFED/) >= 1.5

## 2.2 Download Source and Apply the Patch
Download [hadoop-2.4.1 tarball](https://archive.apache.org/dist/hadoop/core/hadoop-2.4.1/hadoop-2.4.1-src.tar.gz). Unpack it. Download [FFFS patch](https://github.com/songweijia/fffs/blob/master/sources/fffs-for-hadoop-2.4.1-src.patch.tgz), unpack it and put it in the extracted folder hadoop-2.4.1-src. Patch the source code as follows:

` -p1 < fffs-for-hadoop-2.4.1-src.patch`

## 2.3 Build FFFS

Make sure current path is hadoop-2.4.1-src. Use the following command to build FFFS:

`> mvn package -Pnative,dist -Dtar -DskipTests`

This will take a while. After it finishes successfully, find the binary package at hadoop-2.4.1-src/hadoop-dist/target/hadoop-2.4.1.tar.gz. Use this package for deployment.

# 3 Deployment

Deploying FFFS is basically the same as deploying the original HDFS. Please follow the online hadoop guide for how to deploy HDFS. Note that we need a working HDFS setup from this point to continue. We assume the users are familiar with HDFS deployment. To enable FFFS, set the following configurations in /etc/hadoop/hdfs-site.xml:

1) Enable the FFFS block log, and set the memory buffer size for it.
```xml
<property>
  <name>dfs.datanode.fsdataset.factory</name>
  <value>org.apache.hadoop.hdfs.server.datanode.fsdataset.impl.MemDatasetFactory</value>
</property>
<property>
  <name>dfs.memory.capacity</name>
  <value>34359738368</value><!--32GB -->
</property>
```
2) PageSize: the default size is 4KB. Small page size relieve internal fragmentation but cause higher overhead. If the workload mainly consists of very small writes, use page smaller than 4KB.
```xml
<property>
  <name>dfs.memblock.pagesize</name>
  <value>4096</value>
</property>
```
3) Packet size represents the maximum write log resolution. The default packet size is 64KB. Use larger one for better performance if the application is tolerant to coarse log resolution.
```xml
<property>
  <name>dfs.client-write-packet-size</name>
  <value>65536</value>
</property>
```
4) Turn off checksum.
FFFS relies on TCP checksum instead of using another layer of checksum. We plan to support stronger data integrity check in future work.
``` xml
<property>
  <name>dfs.checksum.type</name>
  <value>NULL</value>
</property>
```
5) Turn off replication.
FFFS does not support block replication but we plan to support on demand caching to enable high performance with many readers, which is faster and more space-efficient.
``` xml
<property>
  <name>dfs.replication</name>
  <value>1</value>
</property>
```
6) Optional: RDMA settings

Set the name of the rdma device.
```xml
<property>
  <name>dfs.rdma.device</name>
  <value>mlx5_0</value>
</property>
```


LegoStore: Modular Multi-Version Transactional Key-Value Storage System v0.1

LegoStore is a modular distributed key-value storage system. In particular, LegoStore permits experimentation with
different implementations for storage, transactional protocols, communication protocols, etc. Modularity is benefitial
for two main reasons.
- Key-value storage systems that have a lot of similarities can share common infrastructure. For example, if two
key-value storage systems implemented in top of LegoStore require the same consistency level, both can share the same
concurrency control protocol implementation.
- LegoStore provides a common platform for efficiently comparing different implementations. In particular, developers
might modify a specific module (e.g. storage module) and observe the difference in performance without touching any of
the other modules (minimum changes required).

Developers can read a more detailed explanation of supported modules in the src directory. This README is oriented to
explain installation and usage of LegoStore.

Prerequisites
-------------
TODO: Write about the packets.

Installation
-------------
TODO: Write installation details

Usage
-------------
TODO: Write 

