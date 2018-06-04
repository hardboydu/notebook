# Demystifying Userspace Packet IO Frameworks

## ABSTRACT

Packet forwarding applications such as virtual switches or routers moved from the kernel to userspace processes in the last years facilitated by frameworks like DPDK or netmap. These frameworks are often regarded as black-boxes by developers due to  their perceived inherent complexity. We present ixy, a userspace packet IO framework designed for simplicity and educational purposes to show that fast packet IO in the userspace is not black magic. Ixy avoids external dependencies, unnecessary abstractions, and kernel components completely, allowing the user to understand the whole stack from application logic down to the driver.  A packet forwarder built on ixy is only about 1000 lines of code including the whole driver that runs in the same process. Our code is available as free and open source under the BSD license at [https://github.com/emmericp/ixy](https://github.com/emmericp/ixy).

近些年，由于DPDK或者netmap这些框架的出现，促成了包转发应用程序（例如虚拟交换机，路由器）从内核态转移到用户空间进程。这些框架由于其固有的复杂性，所以通常会被开发人员视为黑盒子。所以我们提出了ixy，一个设计目标为简单和教学为目的的用户空间数据包I/O框架，以显示用户空间快速数据包I/O处理并不是黑科技。Ixy避免了外部以来，不必要的抽象，和完全的内核组件。允许用户理解包括从应用逻辑到下边的驱动程序的整个技术栈。由ixy构建的包转发器只有1000行代码，包括了同一个进程中的驱动程序。我们的代码在BSD许可下可作为免费和开放源代码使用

## CCS Concepts

* Networks→Network performance evaluation; Network experimentation;
* 网络→网络性能评估；网络设备

## Keywords

Userspace, Packet IO, Linux, NIC

用户空间，包I/O，Linux，NIC

## 1. INTRODUCTION

Low-level packet processing on top of traditional socket APIs is too slow for modern requirements and was therefore often done in the kernel in the past. Two examples for packet forwarders utilizing kernel components are OpenvSwitch [21] and the Click modular router [18]. Writing kernel code is not only a relatively cumbersome process with slow turn-around times, it also proved to be too slow for specialized applications as the network stack is designed for a general-purpose operating system. OpenvSwitch was since extended to include DPDK [6] as an optional alternative backend to improve performance [20] over the kernel-based implementation. Click was ported to both netmap [22] and DPDK for the same reasons [1]. Similar moves from kernel-based code to specialized userspace code can also be found in other projects [14, 23].

传统 socket APIS 的底层数据包处理对于现代需求来说太慢了，因此通常在内核中进行。利用内核组件的包转发器有OpenvSwitch[21] 和 Click Modular Router [18]。编写内核代码不仅是一个相对繁琐的过程，而且周转时间很慢，对于专门的应用程序来说，它的速度也很慢，因为网络堆栈是为通用操作系统设计的。OpenvSwitch 后来扩展到包含DPDK [6]作为可选的替代后端，以提高基于内核的实现的性能[20]。同样，Click 则包含了netmap和DPDK。类似的，将基于内核的代码转移到用户空间的实现，可以在另外两个工程中找到[14, 23]。

Developers and researchers still often treat DPDK as a black-box that magically increases speed. One reason for this is that DPDK – unlike netmap and others – does not come from an academic background. It was first developed by Intel and then moved to the Linux Foundation in 2017 [17]. This means that there is no academic paper describing its architecture or implementation. The netmap paper [22] is often used as surrogate to explain how userspace packet IO frameworks work in general. However, DPDK is based on a completely different architecture than seemingly similar frameworks.

开发人员和研究人员仍然经常讲DPDK视为一个神奇的增加速度的黑盒子。其中一个原因就是其没有一个学术背景，这不像netmap和其他组件那样。它最初由英特尔开发，然后在2017年转移到Linux基金会[17]。这就意味着没有学术论文描述其架构和实现。netmap的论文[22] 通常被用来解释用户空间数据包I/O框架如何工作。然而，DPDK基于完全不同的架构，但又看似相似的框架。

We present ixy, a userspace packet framework that is architecturally similar to DPDK [6] and Snabb [16]. Both usefull userspace drivers, unlike netmap [22], pfring [19], pfq [3] or similar frameworks that rely on a kernel driver. There is currently no academic publication available describing the inner workings of such a full userspace framework.

我们提出了ixy，一个用户空间包处理框架，在结构上类似DPDK [6] 和Snabb [16]。与 netmap [22]，pfring [19]，pfq [3]或其他类似的基于内核驱动的框架不同，这两个框架使用用户空间驱动程序。目前还没有学术刊物可以描述这种完整的用户空间框架的内部工作。

Ixy is designed for educational use only, i.e., you are meant to use it to understand how userspace packet framework sand drivers work, not to use it in a production setting. Our whole architecture aims at simplicity and is trimmed down to the bare minimum. A packet forwarding application is less than 1000 lines of code including the whole driver from device reset and initialization to receiving and sending packets. It is certainly possible to read and understand drivers found in other frameworks, but ixy’s driver is at least an order of magnitude simpler than other implementations. For example, the file ```ixgbe_rxtx.c``` in DPDK implementing the normal receive and transmit paths (there also specialized implementations for vector instruction sets) is 5400 lines long. Ixy’s receive and transmit path is only 127 lines of code. This, of course, means that we support far fewer features. But the goal is to understand the basics of a high-performance implementation, not support every conceivable scenario and hardware feature. Ixy implements all common optimizations like batching at various stages and its speed rivals DPDK.

Ixy只被设计用来教学，例如，你可以用来理解用户空间包处理框架发送驱动程序如何工作，但你不能将其用于生产环境。我们的整个架构旨在简化并将其削减至最低程度。一个包转发应用只有不到一百行代码，但包含了完整的驱动程序，它可以重置设备，初始化设备，接受并发送数据包。当然你可以从其他的框架上阅读并理解驱动，但是 ixy 驱动程序比他们至少简单了一个数量级。举个例子，DPDK中的文件 ```ixgbe_rxtx.c``` 实现了接收和发送路径（包含了向量指令集的实现）超过了5400行。Ixy的接收和发送路径只用了127行代码。当然，这意味着我们只提供了非常少的功能。但是我们的目标是理解基本的高性能实现，并不支持每个可以想象的场景和硬件功能。Ixy实现了所有通用的优化，例如各阶段的批量处理，其速度可以媲美DPDK。

The remainder of this paper is structured as follows. We first discuss background and related work, i.e., the basics of other userspace packet IO frameworks and the differences between them. We then dive into ixy’s design and implementation in Sections 3 and 4. A rudimentary performance evaluation is discussed in Section 5. We conclude with an outlook on our future plans for ixy as this is work-in-progress and meant as a basis to evaluate different performance optimization techniques for packet processing and forwarding.

本论文的结构如下，首先，我们讨论背景和相关工作，例如，其他用户空间包I/O框架的基础知识，以及他们之间的区别。接下来我们将在第3和第4节讨论ixy的设计和实现。在第5节给出了一个初步的性能评估。我们对未来ixy计划的展望结束，因为这是工作进行中的，并且意味着评估不同的数据包处理和转发性能优化技术的基础。

## 2. BACKGROUND AND RELATED WORK

A multitude a packet IO frameworks have been built over the past years, each focusing on different aspects with different trade-offs. They can be broadly categorized into two categories: those relying on a driver running in the kernel and those that re-implement the whole driver in userspace. Examples for the former category are netmap [22], PF_RING ZC [19],pfq [3], OpenOnload [24], and XDP [13].  They all use the default driver (sometimes with small custom patches) and an additional kernel component that provides a fast interface based on memory mapping for the userspace application. Packet IO is still handled by the normal driver here, but the driver is attached to the application directly instead of to the normal kernel datapath. This has the advantage that integrating existing kernel components or forwarding packets to the default network stack is relatively simple with these frameworks. By default, these applications still provide an application with exclusive access to the NIC. But the NIC can often still be controlled with standard tools like ethtool to configure hardware features.

过去几年已经建立了众多的数据包IO框架，每个框架都侧重于不同的方面并进行不同的权衡。它们可以大致分为两类: 运行在内核中的驱动程序和在用户空间中重新实现了的驱动程序。前一种例如netmap [22]，PF_RING ZC [19]，pfq [3]，OpenOnload [24] 和 XDP [13]。他们都使用了默认的驱动程序（通常有一些小的补丁）和附加的内核组件，并提供了一个快速的接口，这个接口基于用户空间的内存映射。数据包的I/O操作依然使用传统的驱动程序，但是驱动程序终结挂接到应用程序，而不是传统的内核数据路径。这样做的优势就是可以有效的整合现有的内核组件，并且转发数据包到默认的协议栈也相对简单一些。默认情况下，这些应用程序会独占网卡。但是网卡任然可以用标准的工具控制，例如，ethtool可以配置硬件功能。

In particular, netmap [22] and XDP [13] are good examples of integrating kernel components with specialized applications. netmap (a standard component in FreeBSD and also available on Linux) offers interfaces to pass packets between the kernel network stack and a userspace app, it can even make use of the kernel’s TCP/IP stack with StackMap [25]. Further, netmap supports using a NIC with both netmap and the kernel simultaneously by using hardware filters to steer packets to receive queues either managed by netmap or the kernel [2] (flow bifurcation).  XDP goes one step further as it’s a default part of the Linux kernel and hence very well integrated. It’s technically not a userspace framework: the code is compiled to eBPF which is run by a JIT in the kernel, this restricts the choice of programming language to those that can target eBPF bytecode (typically a restricted subset of C is used). It is commonly used to implement firewalls against DDoS attacks that need to pass on traffic to the network stack [10]. Despite being part of the kernel, XDP does not yet work with all drivers as it requires a new driver API that is still lacking. At the time of writing, XDP in kernel 4.14 supports fewer drivers than DPDK [12, 4].

特别需要指出的是，netmap [22] 和 XDP [13] 是非常好的整合内核组件的应用实例。netmap（FreeBSD的标准组件，也可以用于Linux）提供了内核网络协议栈和用户空间应用之间传递数据包的接口，它甚至可以通过StackMap使用内核的TCP/IP协议栈。netmap支持同时使用netmap和内核，通过使用硬件的过滤器，引导数据包到netmap的接收队列或者内核接收队列 [2] （分流）。XDP更进一步，其作为内核的默认部分和内核之间整合的非常好。XDP从技术上说不是用户空间框架，他的代码使用 eBPF 编译，并使用 JIT 运行在内核空间中，这就限制了编程语言的选择，这个编程语言可以通过 eBPF 编译成 字节码（典型的限制是只能使用 C 语言的一个子集）。它通常被用于实现防火墙，在将数据包传递到协议栈之间抵御 DDoS的攻击。尽管作为内核的一部分，由于缺乏新驱动API，XDP还是不能喝所有的驱动一起工作。在我正在写本文时，XDP在内核 4.14版本中支持的驱动少于DPDK [12, 4]。

DPDK <sup>1</sup> [6] and Snabb [16] implement the driver completely in userspace.  DPDK still uses a small kernel module to help with memory mapping, but it does not contain driver logic and is only used during initialization. A main advantages of the full userspace approach is that the application has full control over the driver leading to a far better integration of the application with the driver and hardware. DPDK features the largest selection of offloading and filtering features of all investigated frameworks [5]. Configuring hardware features through the kernel driver while using a framework of the first category is often error-prone, difficult to use, or hardware features might not be supported. The downside is the poor integration with the kernel, DPDK’s KNI (kernel network interface) needs to copy packets to pass them to the kernel unlike XDP or netmap which can just pass a pointer. Moreover, flow bifurcation is only supportedon Mellanox NICs in DPDK. Other big advantages of DPDK are its support in the industry, mature code base, and large community. DPDK had a slow start in academia due to its background, but more and more publications use DPDK fortheir prototypes. DPDK supports virtually all NICs commonly found in servers [4], far more than any other framework we investigated here.

DPDK <sup>1</sup> [6] 和 Snabb [16] 完全在用户空间中实现驱动程序。DPDK仍然在内核使用了一个小模块来帮助内存映射，但它并不包含驱动的逻辑，只在初始化时被使用。完全的用户空间方法的一个主要优势就是，应用程序可以通过驱动程序完全控制硬件，这样就使应用程序和硬件很好的整合在了一起。DPDK在所有调查的框架中，拥有最强的卸载和过滤功能。

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