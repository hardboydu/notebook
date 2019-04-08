# DPDK NUM

## Retrieve NUMA node information on linux

* List of NUMA nodes

```sh
/sys/devices/system/node/node<numa_node_id>
```

* List of CPU cores associated with each NUMA node

```sh
/sys/devices/system/node/node<numa_node_id>/cpu<thread_id>/topology/core_id
```

* List of thread_siblings for each core

```sh
/sys/devices/system/node/node<numa_node_id>/cpu<thread_id>
```

* NUMA Node ID for network interfaces

```sh
/sys/class/net/<interface name>/device/numa_node
```

or

```sh
$ lspci -nn | grep Eth

$ cat /sys/bus/pci/devices/0000\:02\:00.0/numa_node
```

* RAM available for each NUMA node

```sh
/sys/devices/system/node/node<numa_node_id>/meminfo
```

> *参考 [OpenStack : Retrieve NUMA node information](https://specs.openstack.org/openstack/ironic-inspector-specs/specs/NUMA_node_info.html)*
