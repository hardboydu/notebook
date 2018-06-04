# Demystifying Userspace Packet IO Frameworks

## ABSTRACT

Packet forwarding applications such as virtual switches or routers moved from the kernel to userspace processes in the last years facilitated by frameworks like DPDK or netmap. These frameworks are often regarded as black-boxes by developers due to  their perceived inherent complexity. We present ixy, a userspace packet IO framework designed for simplicity and educational purposes to show that fast packet IO in the userspace is not black magic. Ixy avoids external dependencies, unnecessary abstractions, and kernel components completely, allowing the user to understand the whole stack from application logic down to the driver.  A packet forwarder built on ixy is only about 1000 lines of code including the whole driver that runs in the same process. Our code is available as free and open source under  the BSD license at [https://github.com/emmericp/ixy](https://github.com/emmericp/ixy).

## CCS Concepts

* Networks→Network performance evaluation; Net-work experimentation;

## Keywords

Userspace, Packet IO, Linux, NIC

## 1. INTRODUCTION

Low-level packet processing on top of traditional socket APIs is too slow for  modern requirements and was therefore often done in the kernel in the past. Two examples for packet forwarders utilizing kernel components are OpenvSwitch [21] and the Click modular router [18]. Writing kernel code is not only a relatively cumbersome process withslow turn-around times, it also proved to be too slow for specialized applications as the network stack is designed for a general-purpose operating system. Open vSwitch was since extended to include DPDK [6] as an optional alternative backend to improve performance [20] over the kernel-based implementation. Click was ported to both netmap [22] and DPDK for the same reasons [1]. Similar moves from kernel-based code to specialized userspace code can also be foundin other projects [14, 23].

Developers and researchers still often treat DPDK as a black-box that magically increases speed. One reason for this is that DPDK – unlike netmap and others – does not come from an academic background.  It was first developed by Intel and then moved to the Linux Foundation in 2017 [17]. This means that there is no academic paper describing its architecture or implementation. The netmap paper [22] is often used as surrogate to explain how userspace packet IO frameworks work in general. However, DPDK is based on a completely different architecture than seemingly similar frameworks.

We present ixy, a userspace packet framework that is architecturally similar to DPDK [6] and Snabb [16]. Both usefull userspace drivers, unlike netmap [22], pfring [19], pfq [3] or similar frameworks that rely on a kernel driver. There is currently no academic publication available describing theinner workings of such a full userspace framework.

Ixy is designed for educational use only, i.e., you are meant to use it to understand how userspace packet framework sand drivers work, not to use it in a production setting.  Our whole architecture aims at simplicity and is trimmed down to the bare minimum. A packet forwarding application is less than 1000 lines of code including the whole driver from device reset and initialization to receiving and sending packets. It is certainly possible to read and understand drivers found in other frameworks, but ixy’s driver is at least an order of magnitude simpler than other implementations. For example, the file ```ixgbe_rxtx.c``` in DPDK implementing the normal receive and  transmit paths (there also specialized implementations for vector instruction sets) is 5400 lines long. Ixy’s receive and  transmit  path  is  only  127  lines  ofcode.  This, of course, means that we support far fewer features. But the goal is to understand the basics of a high-performance implementation, not support every conceivable scenario and hardware feature. Ixy implements all common optimizations like batching at various stages and its speed rivals DPDK.

The remainder of this paper is structured as follows. We first discuss background and related work, i.e., the basics of other userspace packet IO frameworks and the  differences between them. We then dive into ixy’s design and implementation in Sections 3 and 4. A rudimentary performance evaluation is discussed in Section 5.  We conclude with an outlook on our future plans for ixy as this is work-in-progress and meant as a basis to evaluate different performance optimization techniques for packet processing and forwarding.

## 2. BACKGROUND AND RELATED WORK

A multitude a packet IO frameworks have been built over the past years, each focusing on different aspects with different trade-offs. They can be broadly categorized into two categories: those relying on a driver running in the kernel and those that re-implement the whole driver in userspace. Examples for the former category are netmap [22], PF_RING ZC [19], pfq [3], OpenOnload [24], and XDP [13].  They all use the default driver (sometimes with small custom patches) and an additional kernel component that provides a fast interface based on memory mapping for the userspace application. Packet IO is still handled by the normal driver here,but the driver is attached to the application directly instead of to the normal kernel datapath. This has the advantage that integrating existing kernel components or forwarding packets to the default network stack is relatively simple with these  frameworks. By default, these applications still provide an application with exclusive access to the NIC. But the NIC can often still be controlled with standard tools like ethtool to configure hardware features.

In particular, netmap [22] and XDP [13] are good examples of integrating kernel components with specialized applications. netmap (a standard component in FreeBSD and also available on Linux) offers interfaces to pass packets between the kernel network stack and a userspace app, it can even make use of the kernel’s TCP/IP stack with StackMap [25]. Further, netmap supports using a NIC with both netmap and the kernel simultaneously by using hardware filters to steer packets to receive queues either managed by netmap or the kernel [2] (flow bifurcation).  XDP goes one step further as it’s a default part of the Linux kernel and hence very well integrated. It’s technically not a userspace framework: the code is compiled to eBPF which is run by a JIT in the kernel, this restricts the choice of programming language to those that can target eBPF bytecode (typically a restricted subset of C is used). It is commonly used to implement firewalls against DDoS attacks that needto pass on traffic to the network stack [10]. Despite beingpart of the kernel, XDP does not yet work with all drivers as it requires a new driver API that is still lacking. At thetime of writing, XDP in kernel 4.14 supports fewer drivers than DPDK [12, 4].

DPDK <sup>1</sup> [6] and Snabb [16] implement the driver completely in userspace.  DPDK still uses a small kernel module to help with memory mapping, but it does not contain driver logic and is only used during initialization. A main advantages of the full userspace approach is that the applicationhas full control over the driver  leading to a far better integration of the application with the driver and hardware. DPDK features the largest selection of offloading and filtering features of all investigated frameworks [5]. Configuring hardware features through the kernel driver while using a framework of the first category is often error-prone, difficult to use, or hardware features might not be supported. The downside is the poor integration with the kernel, DPDK’s KNI (kernel network interface) needs to copy packets to pass them to the kernel unlike XDP or netmap which can just pass a pointer. Moreover, flow bifurcation is only supportedon Mellanox NICs in DPDK. Other big advantages of DPDK are its support in the industry, mature code base, and large community. DPDK had a slow start in academia due to its background, but more and more publications use DPDK fortheir prototypes. DPDK supports virtually all NICs commonly found in servers [4], far more than any other framework we investigated here.

Ixy’s architecture is based on ideas from both DPDK and Snabb. The initialization and operation without loading a driver is inspired by Snabb, the API based on explicit memory management and batching is based on DPDK.

---
> <sup>1</sup> Except when used with Mellanox NICs which still require kernel components

## 3. DESIGN

Ixy is explicitly designed for simplicity and to aid in understanding how a NIC driver works. Our design goals are:

* **Simplicity**. A forwarding application including the driver should be less than 1,000 lines of code.
* **No dependencies**. One self-contained project including the application and driver to aid reading the code.
* **Usability**.  Provide a simple-to-use interface for applications built on it.
* **Speed**. It should be reasonable fast without compromising simplicity, find the right trade-off.

The language of choice is C as the lowest common denominator of systems programming languages.

It should be noted that the Snabb project [16] has similar design goals, ixy tries to be one order of magnitude simpler than  Snabb. For example, Snabb targets 10,000 lines of code [15], we target 1,000 lines of code and Snabb builds on Lua with LuaJIT instead of C limiting accessibility.

### 3.1 Architecture

Ixy’s architecture is very simple due to its lack of abstractions. At its core it’s a custom full userspace implementation of a driver for NICs of the ixgbe family trimmed down forsimplicity. This driver exposes functions to initialize the NIC, read statistics, and to receive/send packets. Packet APIs are based on explicit allocation of packet buffers from specialized memory pool data structures. Applications include and use the driver directly in the same project. This means  that the driver logic is only a single function call away from the application logic, allowing the user to read the code from a top-down level without jumping between complex abstraction interfaces or userspace and kernelspace boundaries.

The complete lack of abstraction is currently only possible because we only support a single driver. We plan to introduce at least one additional driver (VirtIO) and hence a small abstraction layer based on function pointers in the device object (same approach as DPDK).

### 3.2  NIC Selection

Ixy is based on a custom userspace re-implementation of the Intel ixgbe driver cut down to its bare essentials. This means that we only support Intel NICs of the ixgbe family, we have tested it on X540, X550, and 82599ES (aka X520) NICs. All other frameworks except DPDK are also restricted to very few NICs models (typically 3 or fewer) and ixgbe is (except for OpenOnload only supporting their own NICs) always supported. We chose ixgbe for ixy because Intel releases extensive data sheets for all their NICs and the ixgbe NICs are commonly found in commodity servers. These NICs are also interesting because they still exposea relatively low-level interface to the drivers. This is especially visible in some advanced features like filters where implementation details such as chained hash map implementations for filters are directly exposed and configured by the driver.  Other NICs like the newer Intel XL710 series or Mellanox ConnectX-4/5 follow a more firmware-driven design: a lot of functionality is hidden behind a black-box firmware running on the NIC and the driver merely communicates via a message interface with the firmware which does the hard work. This approach has obvious advantages such as abstracting hardware details of different NICs allowing for a simpler more generic driver.  However, our goal with ixy is understanding the full stack – a black-box firmware is hence counterproductive and we do not plan to add  support for such NICs.

## 4. IMPLEMENTATION

All line numbers referenced in this Section are for commit 436750e of ixy. All page numbers and section numbers for the Intel datasheet refer to Revision 3.3 (March 2016) of the 82599 datasheet [11].  Function names and line numbers are hyperlinked to the implementation on GitHub in the PDF.

### 4.1 Userspace Drivers in Linux

We want to keep the whole stack inside a single program, i.e., no kernel code can be used. One needs to understand how a driver communicates with a device first to understand how a driver can be written in userspace. This simplified overview skips details but is sufficient to understand how ixy or similar frameworks work.  A driver can communicate via two ways with a PCIe device: directly with memory-mapped IO (MMIO) or indirectly with direct memory access (DMA).

MMIO maps a memory area to device IO, i.e., reading from or writing to this memory  area receives/sends datafrom/to the device. Devices commonly expose their configuration registers via this interface, the datasheet provides an offset into this memory area where normal reads and writes can be used to access the register. PCIe devices can specify multiple memory areas called Base Address Registers (BARs). These are exposed in Linux via the sysfs pseudo filesystem and can be mapped into a  process via mmap. The implementation of this mapping can be found in ```pci_map_resourceinpci.c```. ixgbe devices expose all configuration, statistics, and debugging registers via the BAR0 configuration space. The datasheet [11] lists all registers as offsets in this configuration space. We use ```ixgbe_type.h``` from Intel’s driver as machine-readable version of the datasheet<sup>2</sup>, it contains defines for all register names and offsets for bit fields.

---
> <sup>2</sup> This is technically a violation of both our goal about dependencies  and lines of code, but we only effectively use less than 100 lines that are just defines and simple structs. There is nothing to be gained from copy & pasting offsets and names from the datasheet or this file .

DMA is initiated by the PCIe device and allows it to read/write arbitrary <sup>3</sup> physical addresses. This is used to read/write packet data as well as to transfer the DMA descriptors (pointers to packet data and offloading information) between driver and NIC. The userspace driver hence needs to be able to translate its virtual addresses to physical addresses, this is possible via the procfs pseudo filesystem file ```/proc/self/pagemap```, the translation logic is implemented in ```virt_to_physinmemory.c```. Further, DMA needsto be enabled for the PCI devices on the system, this can be done with the function ```pci_set_master``` in the kernel which is exposed via a bitfield in sysfs that is manipulated in ```enable_dma``` in ```pci.c```.

---
> <sup>3</sup> Utilizing the IOMMU to restrict device access is planned in ixy

These file-based APIs give us full access to the device without needing to write any kernel code. Ixy also unloads any currently loaded kernel driver for the given PCIe device to prevent conflicts, i.e., there is no kernel driver configured for the device while ixy is running. The only capability that is missing is handling interrupts which could be done by loading the ```uio_pci_generic``` driver for the  module. Ixy only supports poll-mode at the moment to keep the code simple.

### 4.2 Application API and Memory Management

Ixy builds on an API with explicit memory allocation similar to DPDK which is a very different approach from netmap [22] that exposes a replica of the NIC’s ring buffer to the application. Memory allocation for packets was cited as one of the main reasons why netmap is faster than traditional in-kernel processing  [22]. Hence,  netmap exposes replicas of the ring buffers <sup>4</sup> to the application, and it’s then up to the application to handle memory. Many simple forwarding cases can  then be implemented by simply swapping pointers in the rings. However, more complex scenarios where packets are not forwarded immediately to a NIC (e.g., because they are passed to a different core in a pipeline setting) do not map well to this API  and require adding manual memory management on top of this API. Further, a ring-based API is very cumbersome to use compared to one with memory allocation.

---
> <sup>4</sup> Not the actual ring buffers to prevent the user-space application from crashing the kernel with invalid pointers.

It is true that memory allocation for packets is a significant overhead in the  Linux kernel, we have measured a per-packet overhead of 100 cycles <sup>5</sup> when forwarding packets with Open vSwitch on Linux 3.7 for allocating and freeing packet  memory (measured with ```perf```). However, this overhead is almost completely due  to (re-)initialization of the kernel ```sk_buff``` struct – a large data structure  with a lot of metadata fields targeted at a general-purpose network stack. Memory  allocation in ixy with minimum metadata required only adds an overhead of 24 cycles/packet, a price that we are willing to pay for the gained simplicity in the user-facing API. Careful design of the data structure containing the metadata can still keep this approach fast in the face of new features: fields needing a reset should be kept together. For example, DPDK stores 128 bytes of metadata per packet but only needs a single 8 byte write to reset a packet buffer

---
> <sup>5</sup> Forwarding 10 Gbit/s with minimum-sized packets on a single 3.0 GHz CPU core leaves a budget of 200 cycles/packet.

Ixy’s API is the same as DPDK’s API when it comes to sending and receiving packets  and managing memory. It can best be explained by reading the example applications ```ixy-fwd.c``` and ```ixy-pktgen.c```. The transmit-only example ```ixy-pktgen.c``` creates a memory pool, a fixed-size collection of fixed-size buffers that are used as packet buffers and prefills them with packet data. It then allocates a batch of packets from this memory pool and passes them to the transmit function. The driver takes care of freeing packets asynchronously in the transmit function. The forward example ```ixy-fwd.c``` can avoid explicit handling of memory pools in the application: the driver allocates a memory pool for each receive ring and automatically allocates packets, freeing is either handled in the transmit function as before or by dropping the packet explicitly if the output link is full.

### 4.3 NIC Ring API

NICs expose multiple circular buffers called queues or rings to transfer packets. The simplest setup uses only one receive and one transmit queue. Multiple transmit queues are merged on the NIC, incoming traffic is split according to filters or a hashing algorithm (RSS) if multiple receive queues are configured. Both receive and transmit rings work in a similar way: the driver programs a (physical) base address and the size of the ring. It then fills the memory area with DMA descriptors, i.e., pointers to physical addresses where the actual packet data is stored and some metadata. Sending and receiving packets is then done by passing ownership of the DMA descriptors between driver and hardware via a head and tail pointer. The driver controls the tail pointer, hardware the head pointer. Both pointers are stored
in device registers accessible via MMIO.

The initialization code is in ```ixgbe.c``` starting from line 124 for receive queues and from line 179 for transmit queues. Further details are in the datasheet in Section 7.1.9 and in the datasheet sections mentioned in the code.

#### 4.3.1 Receiving Packets

The driver fills up the ring buffer with physical pointers to packet buffers in ```start_rx_queue``` on startup. Each time a packet is received, the corresponding buffer is returned to the application and we allocate a new packet buffer and store its physical address in the DMA descriptor and reset the ready flag. It is up to the application to free the packet buffer of received packets. We also need a way to translate the physical addresses in the DMA descriptor found in the ring back to its virtual counterpart on packet reception. This is done by keeping a second copy of the ring that is populated with virtual instead of physical addresses, this is then used as a lookup table for the translation.

A naive way to check if packets have been received is reading the head register from the NIC. However, reading a NIC register requires a PCIe round trip – a slow operation. The hardware also sets a flag in the descriptor via DMA which is far cheaper to read as the DMA write is handled by the last-level cache on modern CPUs. This is effectively the difference between a cache miss and hit for every received packet. Figure 1 illustrates the memory layout: the DMA descriptors in the ring to the left contain physical pointers to packet buffers stored in a separate location in a memory pool. The packet buffers in the memory pool contain their physical address in a metadata field. Figure 2 shows the RDH (head) and RDT (tail) registers controlling the ring buffer on the right side, and the local copy containing the virtual addresses to translate the physical addresses in the descriptors in the ring back for the application. ixgbe_rx_packet in ixgbe.c implements the receive logic as described by Sections 1.8.2 and 7.1 of the datasheet.

#### 4.3.2 Transmitting Packets

Transmitting packets follows the same concept and API as receiving them, but the function is more complicated because the API between NIC and driver is asynchronous. Placing a packet into the ring does not immediately transfer it and blocking to wait for the transfer is unfeasible. Hence, the ```ixgbe_tx_packet``` function in ```ixgbe.c``` consists of two parts: freeing packets from previous calls that were sent out by the NIC followed by placing the current packets into the ring. The first part is often called cleaning and works similar to receiving packets: the driver checks a flag that is set by the hardware after the packet associated with the descriptor is sent out. Sent packet buffers can then be free’d, making space in the ring. Afterwards, the pointers of the packets to be sent are stored in the DMA descriptors and the tail pointer is updated accordingly.

#### 4.3.3 Batching

Each successful transmit or receive operation involves an update to the NIC’s tail pointer register (RDT and TDT for receive/transmit), a slow operation. This is one of the reasons why batching is so important for performance. Both the receive and transmit function operate on a batch of packets in ixy, updating the register only once per batch.

#### 4.3.4 Offloading Features

Ixy currently only enables CRC checksum offloading. Unfortunately, packet IO frameworks (e.g., netmap) are often restricted to this bare minimum of offloading features. DPDK is the exception here as it supports almost all offloading features offered by the hardware. However, as explained earlier its receive and transmit functions pay the price for these features in the form of complexity.

We will try to find a balance and showcase selected simple offloading features in Ixy in the future. These offloading features can be implemented in the receive and transmit functions, see comments in the code. This is simple for some features like VLAN tag offloading and more involved for more complex features requiring an additional descriptor containing metadata information.

#### 4.3.5 Huge Pages

Ixy requires the use of 2MiB huge pages because we need to allocate memory with contiguous physical addresses for the ring memory storing the DMA descriptors. Each DMA descriptor is 16 bytes, i.e., rings with a size of above 256 (typical ring sizes are 512 and above) would not fit on a single page 4KiB page and allocating multiple pages with contiguous physical addresses from user space is difficult. Memory pools are also allocated on huge pages. Huge pages are used by all such frameworks as they also increase the performance due to reduced dTLB usage.

## 5 PERFORMANCE EVALUATION

We run the ixy-fwd example under a full bidirectional load of 29.76Mpps (line rate with minimum-sized packets at 2x 10Gbit/s) pinned to a single CPU core.

### 5.1 Throughput

To quantify the baseline performance and identify bottlenecks, we run the forwarding example while increasing the CPU’s clock frequency from 1.6GHz to 3.3GHz. Figure 3 shows the resulting throughput when forwarding across the two ports of a dual-port NIC and when using two separate single-port NICs and compares it to DPDK’s l2fwd example. Ixy is faster than DPDK at low clock speeds, but it plateaus earlier than DPDK when both ports are on the same NIC. This indicates that the bottleneck is the NIC or PCIe bus in this scenario, DPDK’s driver is more optimized and accesses the NIC less often. Both Ixy and DPDK were configured with a batch size of 32.

### 5.2 Batching

Batching is one of the main drivers for performance. Receiving or sending a packet involves an access to the queue index registers, invoking a costly PCIe round-trip. Figure 4 shows how the performance increases as the batch size is increased in the bidirectional forwarding scenario with two single-port NICs. Increasing batch sizes have diminishing returns: this is clearly visible when the CPU is only clocked at 1.6GHz. Performance increases logarithmically until a batch size of 32, the gain afterwards drops off. This is caused by an increased cache pressure as more packets are kept in the cache. ixy-fwd does not touch the packet data, so the effect is still small but measurable on slow CPUs.

### 5.3 Profiling

We run perf on ixy-fwd running under full bidirectional load at 1.6GHz with two single-port NICs using the default batch size of 32. This configuration ensures that the CPU is the bottleneck. perf allows profiling with the minimum possible effect on the performance (throughput drops by only 3.5% while perf is running). Table 1 shows where CPU time is spent on average per forwarded packet and compared to DPDK. Ixy’s receive function leaves room for improvements, it is far less optimized than the transmit function. There are several places in the receive function where DPDK avoids memory accesses by batching compared to ixy. However, these optimizations were not applied for simplicity in ixy: DPDK’s receive function in ixgbe is quite complex.

Overhead for memory management is significant (but still low compared to 100 cycles/packet in the Linux kernel). 55% of the time is spent in non-batched memory operations and none of the calls are inlined. Inlining these functions increases throughput by 9.2% but takes away our ability to account time spent in them with perf. Overall, the overhead of memory management is larger than we initially expected, but we still think explicit memory management for the sake of a usable API is a worthwhile trade-off. This is especially true for ixy aiming at simplicity,but also for DPDK or other frameworks targeting complex applications. Simple forwarding can easily be done on an exposed ring interface, but anything more complex that does not sent out packets immediately (e.g., because they are processed further on a different core). Moreover, 24 cycles per packet that could be saved is still a tiny improvement compared to other architectural decisions like batch processing that reduces per-packet processing costs by 300 cycles when going from no batching to a batch size of 32.

### 5.4 Reproducible Research

The full code of ixy and the scripts used to run these benchmarks to reproduce these results is available at GitHub [7, 8]. We used commit 436750e for the evaluation. These results were obtained on an Intel Xeon E3-1230 V2 running Ubuntu 16.04.2 (kernel 4.4) with a dual port Intel X520-T2 (82599ES) NIC and two single port X520-T1 NICs. Turboboost, Hyper-Threading, and power-saving features were
disabled. An identical second server was used to run the MoonGen packet generator [9] with the l2-load-latency example script that can generate full bidirectional line rate.

Preliminary tests on other systems show that the performance can vary significantly with the CPU model. In particular, using a fast CPU or less load (e.g., only unidirectional load) will often lead to worse performance. In fact, ixy-fwd fails to achieve line rate in with unidirectional load at 3.3GHz on the hardware used here. Underclocking the CPU or adding artificial workload increases the speed in this case. The likely cause is inefficient utilization of the NIC or PCIe bus, we are still investigating this particular issue. Older versions of DPDK also suffered from this.

## 6 CONCLUSIONS AND FUTURE WORK

We discussed how to build a userspace driver for NICs of the ixgbe family which are commonly found in commodity servers. Ixy is work in progress, this paper only aims to explain the basics of ixy and similar frameworks. We plan to add support for VirtIO NICs to allow simple testing in virtual machines without hardware dependencies. We plan to use ixy to investigate the effects of several optimizations that are commonly found in similar frameworks: batching, huge page size, prefetching, DMA descriptor ring size, mem- ory pool data structures, etc. These individual effects can easily be tested in isolation (or combined with others)in ixy due to its simple and easily modifiable driver.

Further, we plan to investigate safety and security features on modern hardware for full userspace packet frameworks. Applications built on DPDK or ixy require full root privileges. These privileges can be dropped after initialization in ixy, but the process still has full access to the hardware which can write to arbitrary memory locations using DMA, posing a safety and security risk. This can be mitigated by using the IOMMU to restrict the device to a pre-defined address space. The IOMMU is a rarely used component with mostly unknown performance characteristics.

## 7 REFERENCES

* [1] T. Barbette, C. Soldani, and L. Mathy. Fast userspace packet processing. In ACM/IEEE ANCS, 2015.
* [2] G. Bertin. Single RX queue kernel bypass in Netmap for high packet rate networking, Oct. 2015. 
https://blog.cloudflare.com/ 
single-rx-queue-kernel-bypass-with-netmap/.
* [3] N. Bonelli, A. Pietro, S. Giordano, and G. Procissi. On multi-gigabit packet capturing with multi-core commodity hardware. In Passive and Active Measurement 2012, pages 64–73, Mar. 2012.
* [4] DPDK Project. DPDK: Supported NICs. http://dpdk.org/doc/nics. Last visited 2017-11-30.
* [5] DPDK Project. DPDK User Guide: Overview of Networking Drivers. http://dpdk.org/doc/guides/nics/overview.html. Last visited 2017-11-30.
* [6] DPDK Project. DPDK Website. http://dpdk.org/. Last visited 2017-11-30.
* [7] P. Emmerich. ixy code. https://github.com/emmericp/ixy.
* [8] P. Emmerich. Scripts used for the performance evaluation. https://github.com/emmericp/ixy-perf-measurements.
* [9] P. Emmerich, S. Gallenmüller, D. Raumer, F. Wohlfart, and G. Carle. MoonGen: A Scriptable High-Speed Packet Generator. In Internet Measurement Conference 2015 (IMC’15), Tokyo, Japan, Oct. 2015.
* [10] Gilberto Bertin. XDP in practice: integrating XDP into our DDoS mitigation pipeline. In Netdev 2.1, The Technical Conference on Linux Networking, May 2017.
* [11] Intel 82599 10 GbE Controller Datasheet Rev. 3.3. Intel, 2016.
* [12] IO Visor Project. BPF and XDP Features by Kernel Version. https://github.com/iovisor/bcc/blob/master/ 
docs/kernel-versions.md#xdp. Last visited 2017-11-30.
* [13] IO Visor Project. Introduction to XDP. https://www.iovisor.org/technology/xdp Last visited 2017-11-30.
* [14] Jim Thompson. DPDK, VPP & pfSense 3.0. In DPDK Summit Userspace, Sept. 2017.
* [15] Jonathan Corbet. User-space networking with Snabb. In LWN.net, Feb. 2017.
* [16] L. Gorrie et al. Snabb: Simple and fast packet networking. https://github.com/snabbco/snabb.
* [17] Linux Foundation. Networking Industry Leaders Join Forces to Expand New Open Source Community to Drive Development of the DPDK Project, Apr. 2017. Press release.
* [18] R. Morris, E. Kohler, J. Jannotti, and M. Frans Kaashoek. The click modular router. In Operating Systems Review - SIGOPS, volume 33, pages 217–231, Dec. 1999.
* [19] ntop. PF RING ZC (Zero Copy). http://www.ntop.org/products/packet-capture/pf
ring/pf ring-zc-zero-copy/. Last visited 2017-11-30.
* [20] Open vSwitch Project. Open vSwitch with DPDK. http://docs.openvswitch.org/en/latest/intro/install/dpdk/ Last visited 2017-11-30.
* [21] B. Pfaff, J. Pettit, T. Koponen, E. Jackson, A. Zhou, J. Rajahalme, J. Gross, A. Wang, J. Stringer, P. Shelar, K. Amidon, and M. Casado. The design and implementation of open vswitch. In 12th USENIX Symposium on Networked Systems Design and Implementation (NSDI 15), pages 117–130, Oakland, CA, 2015. USENIX Association.
* [22] L. Rizzo. netmap: A Novel Framework for Fast Packet I/O. In USENIX Annual Technical Conference, pages 101–112, 2012.
* [23] Snort Project. Snort 3 User Manual. https://www.snort.org/downloads/snortplus/snort manual.pdf Last visited 2017-11-30.
* [24] Solarflare. OpenOnload Website. http://www.openonload.org/. Last visited 2017-11-30.
* [25] K. Yasukata, M. Honda, D. Santry, and L. Eggert. StackMap: Low-Latency Networking with the OS Stack and Dedicated NICs. In 2016 USENIX Annual Technical Conference (USENIX ATC 16), pages 43–56, Denver, CO, 2016. USENIX Association.