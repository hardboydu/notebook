# User Space Network Drivers

## Abstract

The rise of user space packet processing frameworks like DPDK and netmap makes low-level code more accessible to developers and researchers. Previously, driver code was hidden in the kernel and rarely modified–or even looked at–by developers working at higher layers. These barriers are gone nowadays, yet developers still treat user space drivers as black-boxes magically accelerating applications. We want to change this: every researcher building network applications should understand the intricacies of the underlying drivers, especially if they impact performance. We present ixy, a user space network driver designed for simplicity and educational purposes. Ixy focuses on the bare essentials of user space packet processing: a packet forwarder including the whole NIC driver uses less than 1000 lines of C code. We discuss how ixy implements drivers for both the Intel 82599 family and for virtual VirtIO NICs. The former allows us to reason about driver and framework performance on a stripped-down implementation to assess individual optimizations in isolation. VirtIO support ensures that everyone can run it in a virtual machine. Our code is available as free and open source under the BSD license at https://github.com/emmericp/ixy.

DPDK 和 netmap 等用户空间数据包处理框架的兴起使得开发人员和研究人员更容易访问低级代码。以前，驱动程序代码隐藏在内核中，很少被修改 - 甚至是在更高层工作的开发人员。这些障碍现在已经消失，但开发人员仍将用户空间驱动程序视为黑盒子神奇地加速应用程序。我们想要改变这一点：每个构建网络应用程序的研究人员都应该了解底层驱动程序的复杂性，特别是如果它们影响性能。我们提供了ixy，一种用于简单和教育目的的用户空间网络驱动程序。Ixy专注于用户空间数据包处理的基本要素：包含整个 NIC 驱动程序的包转发器使用少于1000行的C代码。我们将讨论 ixy 如何实现 Intel 82599 系列和虚拟 VirtIO NIC 的驱动程序。前者允许我们在精简实现上推断驱动程序和框架性能，以单独评估单个优化。VirtIO 支持确保每个人都可以在虚拟机中运行它。我们的代码在 BSD 许可下通过 https://github.com/emmericp/ixy 以免费和开源的形式提供。

## 1 Introduction

Low-level packet processing on top of traditional socket APIs is too slow for modern requirements and was therefore often done in the kernel in the past. Two examples for packet forwarders utilizing kernel components are Open vSwitch[36]and the Click modular router[33]. Writing kernel code is not only a relatively cumbersome process with slow turn-around times, it also proved to be too slow for specialized applications. Open vSwitch was since extended to include DPDK [7] as an optional alternative backend to improve performance [35]. Click was ported to both netmap [40] and DPDK for the same reasons [2]. Other projects also moved kernel-based code to specialized user space code [27, 42].

传统套接字API之上的低级数据包处理对于现代需求而言太慢，因此通常在内核中完成。使用内核组件的包转发器的两个例子是 Open vSwitch [36] 和 Click modular router [33]。编写内核代码不仅是一个相对繁琐的过程，周期长，而且对于专业应用程序来说，它也被证明是太慢了。Open vSwitch 后来扩展到包括 DPDK [7] 作为可选的替代后端以提高性能[35]。由于同样的原因，Click 被移植到 netmap [40] 和 DPDK [2]。其他项目还将基于内核的代码移动到专用用户空间代码[27,42]。

Developers and researchers still often treat DPDK as a black-box that magically increases speed. One reason for this is that DPDK – unlike netmap and others – does not come from an academic background. It was first developed by Intel and then moved to the Linux Foundation in 2017 [30]. This means that there is no academic paper describing its architecture or implementation. The netmap paper [40] is often used as surrogate to explain how user space packet IO frameworks work in general. However, DPDK is based on a completely different architecture than seemingly similar frameworks.

开发人员和研究人员仍然经常将 DPDK 视为一个神奇地提高速度的黑匣子。其中一个原因是DPDK - 不像 netmap 和其他框架 - 它不是来自学术背景。它最初由英特尔开发，然后在2017年转移到Linux 基金会[30]。这意味着没有学术论文描述其架构或实现。netmap 论文[40] 通常用作代理来解释用户空间数据包IO框架的工作原理。但是，DPDK基于与看似相似的框架完全不同的架构。

We present ixy, a user space packet framework that is architecturally similar to DPDK [7] and Snabb [17]. Both use full user space drivers, unlike netmap [40], PF - RING [34], pfq [4] or similar frameworks that rely on a kernel driver. Ixy is designed for educational use only, i.e., you are meant to use it to understand how user space packet frameworks and drivers work, not to use it in a production setting. Our whole architecture aims at simplicity and is trimmed down to the bare minimum. We currently support the Intel ixgbe family of NICs and virtual VirtIO NICs. A packet forwarding application is less than 1000 lines of code including the whole driver. It is possible to read and understand drivers found in other frameworks, but ixy’s driver is at least an order of magnitude simpler than other implementations. For example, DPDK’s implementation of the 82599 driver needs 5400 lines of code just to handle receiving and sending packets in a highly optimized way. Ixy’s receive and transmit path for the same driver is only 127 lines of code.

我们提出了一个用户空间数据包框架 ixy，它在架构上类似于 DPDK [7] 和 Snabb [17]。两者都使用完整的用户空间驱动程序，不像 netmap [40]，PF-RING [34]，pfq [4] 或依赖于内核驱动程序的类似框架。 Ixy 仅用于教育用途，即，您可以使用它来了解用户空间数据包框架和驱动程序的工作方式，而不是在生产环境中使用它。我们的整个架构旨在简化并减少到最低限度。我们目前支持 Intel ixgbe 系列 NIC 和虚拟 VirtIO NIC。数据包转发应用程序少于1000行代码，包括整个驱动程序。可以阅读和理解在其他框架中找到的驱动程序，但 ixy 的驱动程序至少比其他实现简单一个数量级。例如，DPDK 的 82599 驱动程序的实现需要 5400 行代码，以便以高度优化的方式处理接收和发送数据包。Ixy 的同一驱动程序的接收和发送路径只有 127 行代码。

It is not our goal to support every conceivable scenario, hardware feature, or optimization. We aim to provide an educational platform for experimentation with
driver-level features or optimizations. Ixy is available under the BSD license for this purpose [8]. Further, we publish all scripts used for our evaluation [10].

我们的目标不是支持所有可能的场景，硬件功能或优化。我们的目标是提供一个教育平台，用于实验驱动程序级功能或优化。为此目的，Ixy 可根据 BSD 许可证获得[8]。此外，我们发布了用于评估的所有脚本[10]。

The remainder of this paper is structured as follows. We first discuss background and related work, i.e., the basics of other user space packet IO frameworks and the differences between them. We then look at ixy’s design at a high level in Section 3 before diving into implementation (4) and performance (5) of our ixgbe driver. Section 6 discusses our VirtIO driver before concluding with an explanation on reproducing our results and running ixy in a virtual machine in Section 7.

本文的其余部分的结构如下。我们首先讨论背景和相关工作，即其他用户空间数据包 I/O 框架的基础知识以及它们之间的差异。然后我们在第3节中高级别地看一下 ixy 的设计，然后再深入探讨ixgbe驱动程序的实现（4）和性能（5）。第6节讨论了我们的VirtIO驱动程序，然后在第7节中总结了有关复制结果和在虚拟机中运行ixy的说明。

## 2 Background and Related Work

A multitude of packet IO frameworks have been built over the past years, each focusing on different aspects. They can be broadly categorized into two categories: those relying on a driver running in the kernel and those that re-implement the whole driver in user space.

在过去几年中，已经构建了大量的数据包 I/O 框架，每个框架都侧重于不同的方面。它们可以大致分为两类：依赖于在内核中运行的驱动程序和在用户空间中重新实现整个驱动程序的框架。

Examples for the former category are netmap [40], PF_RING_ZC [34], pfq [4], OpenOnload [43], and XDP [26]. They all use the default driver (sometimes with small custom patches) and an additional kernel component that provides a fast interface based on memory mapping for the user space application. Packet IO is still handled by the normal driver here, but the driver is attached to the application directly instead of to the normal kernel datapath. This has the advantage that integrating existing kernel components or forwarding packets to the default network stack is relatively simple with these frameworks. By default, these applications still provide an application with exclusive access to the NIC. Parts of the NIC can often still be controlled with standard tools like ethtool to configure packet filtering or queue sizes. However, offloading features are often poorly supported, e.g., netmap supports no hardware checksums or tunnel en-/decapsulation features at all [13].

前一类的例子是 netmap [40]，PF_RING_ZC [34]，pfq [4]，OpenOnload [43]和XDP [26]。它们都使用默认驱动程序（有时使用小型自定义补丁）和一个额外的内核组件，它为用户空间应用程序提供基于内存映射的快速接口。数据包 I/O 仍然由普通驱动程序处理，但驱动程序直接连接到应用程序而不是普通的内核数据路径。这具有以下优点：利用这些框架将现有内核组件或转发分组转发到默认网络堆栈相对简单。默认情况下，这些应用程序仍然为应用程序提供对 NIC 的独占访问权限。 NIC 的某些部分通常仍然可以使用 ethtool 等标准工具进行控制，以配置数据包过滤或队列大小。但是，卸载功能通常支持不足，例如，netmap 根本不支持硬件校验和或隧道连接/解封装功能[13]。

In particular, netmap [40] and XDP [26] are good examples of integrating kernel components with specialized applications. netmap (a standard component in FreeBSD and also available on Linux) offers interfaces to pass packets between the kernel network stack and a user space app, it can even make use of the kernel’s TCP/IP stack with StackMap [46]. Further, netmap supports using a NIC with both netmap and the kernel simultaneously by using hardware filters to steer packets to receive queues either managed by netmap or the kernel [3]. XDP is technically not a user space framework: the code is compiled to eBPF which is run by a JIT in the kernel, this restricts the choice of programming language to those that can target eBPF bytecode (typically a restricted subset of C is used). It is a default part of the Linux kernel nowadays and hence very well integrated. It is well-suited to implement firewalls that need to pass on traffic to the network stack [16]. However, it is currently not feasible to use as a foundation for more complex applications due to limited functionality and restrictions imposed by running as eBPF code in the kernel. Despite being part of the kernel, XDP does not yet work with all drivers as it requires a new memory model for all supported drivers. At the time of writing, XDP in kernel 4.15 supports fewer drivers than DPDK [25, 5].

特别是，netmap [40]和XDP [26]是将内核组件与专用应用程序集成的好例子。netmap（FreeBSD中的一个标准组件，也可以在Linux上使用）提供了在内核网络堆栈和用户空间应用程序之间传递数据包的接口，它甚至可以利用 StackMap [46] 的内核 TCP/IP 堆栈。此外，netmap 支持同时使用带有 netmap 和内核的 NIC，通过使用硬件过滤器来引导数据包接收由 netmap 或内核管理的队列[3]。XDP在技术上不是用户空间框架：代码被编译为由内核中的JIT运行的eBPF，这将编程语言的选择限制为可以针对eBPF字节码的那些（通常使用 C 的受限子集）。它是当今Linux内核的默认部分，因此非常好地集成。它非常适合实现需要将流量传递到网络堆栈的防火墙[16]。然而，由于在内核中作为eBPF代码运行所施加的有限功能和限制，目前不可能用作更复杂应用程序的基础。尽管作为内核的一部分，XDP还没有适用于所有驱动程序，因为它需要为所有支持的驱动程序提供新的内存模型。在撰写本文时，内核4.15中的XDP支持的驱动程序少于DPDK [25,5]。

DPDK [7] and Snabb [17] implement the driver completely in user space. DPDK still uses a small kernel module with some drivers, but it does not contain driver logic and is only used during initialization. A main advantage of the full user space approach is that the application has full control over the driver leading to a far better integration of the application with the driver and hardware. DPDK features the largest selection of offloading and filtering features of all investigated frameworks [6]. The downside is the poor integration with the kernel, DPDK’s KNI (kernel network interface) needs to copy packets to pass them to the kernel unlike XDP or netmap which can just pass a pointer. Other advantages of DPDK are its support in the industry, mature code base, and large community. DPDK supports virtually all NICs commonly found in servers [5], far more than any other framework we investigated here.

DPDK [7]和 Snabb [17] 完全在用户空间中实现驱动程序。DPDK仍然使用带有一些驱动程序的小型内核模块，但它不包含驱动程序逻辑，仅在初始化期间使用。完整用户空间方法的一个主要优点是应用程序可以完全控制驱动程序，从而更好地集成应用程序与驱动程序和硬件。DPDK具有所有调查框架中最大的卸载和过滤功能选择[6]。缺点是与内核的集成很差，DPDK 的 KNI（内核网络接口）需要复制数据包以将它们传递给内核，这与 XDP 或 netmap 不同，后者只是传递一个指针。DPDK的其他优点是它在业界的支持，成熟的代码库和大型社区。DPDK几乎支持服务器中常见的所有 NIC [5]，远远超过我们在此调查的任何其他框架。

Ixy’s architecture is based on ideas from both DPDK andSnabb. The initialization and operation without loading a driver is inspired by Snabb, the API based on explicit memory management, batching, and abstraction from the driver is similar to DPDK.

Ixy 的架构基于 DPDK 和 Snabb 的想法。无需加载驱动程序的初始化和操作受 Snabb 的启发，基于显式内存管理的API，批处理和驱动程序的抽象类似于 DPDK。

## 3 Design

The language of choice is C as the lowest common denominator of systems programming languages.

选择的语言是C作为系统编程语言的最小公分母。

Our design goals for ixy are:

我们的ixy设计目标是：

* Simplicity. A forwarding application including a driver should be less than 1,000 lines of C code.
* No dependencies. One self-contained project including the application and driver.
* Usability. Provide a simple-to-use interface for applications built on it.
* Speed. It should be reasonable fast without

---

* 简单，包含驱动程序的转发应用程序应少于1,000行C代码。
* 没有依赖，一个独立的项目，包括应用程序和驱动程序。
* 可用性，为构建在其上的应用程序提供简单易用的界面。
* 速度，它应该是合理的快速

It should be noted that the Snabb project [17] has similar design goals; ixy tries to be one order of magnitude simpler. For example, Snabb targets 10,000 lines of code [28], we target 1,000 lines of code and Snabb builds on Lua with LuaJIT instead of C limiting accessibility.

应该指出的是，Snabb项目[17]具有类似的设计目标; ixy试图简化一个数量级。例如，Snabb针对10,000行代码[28]，我们针对1000行代码和Snabb构建在Lua上，使用LuaJIT而不是C限制可访问性。

### 3.1 Architecture

Ixy only features one abstraction level: it decouples the used driver from the user’s application. Applications call into ixy to initialize a network device by its PCI address, ixy choses the appropriate driver automatically and returns a struct containing function pointers for driverspecific implementations. We currently expose packet reception, transmission, and device statistics to the application. Packet APIs are based on explicit allocation of buffers from specialized memory pool data structures.

Ixy 仅具有一个抽象级别：它将使用的驱动程序与用户的应用程序分离。应用程序调用 ixy 以通过其 PCI 地址初始化网络设备，ixy自动选择适当的驱动程序并返回包含驱动程序特定实现的函数指针的结构。我们目前向应用程序公开数据包接收，传输和设备统计信息。分组API基于来自专用内存池数据结构的缓冲区的显式分配。

Applications include the driver directly, ensuring a quick turn-around time when modifying the driver. This means that the driver logic is only a single function call away from the application logic, allowing the user to read the code from a top-down level without jumping between complex abstraction interfaces or even system calls.

应用程序直接包含驱动程序，确保在修改驱动程序时快速周转时间。这意味着驱动程序逻辑只是远离应用程序逻辑的单个函数调用，允许用户从自上而下的级别读取代码，而无需在复杂的抽象接口甚至系统调用之间跳转。

### 3.2 NIC Selection

Ixy is based on custom user space re-implementation of the Intel ixgbe driver and the VirtIO virtio-net driver cut down to their bare essentials. We’ve tested our ixgbe driver on Intel X550, X540, and 82599ES (aka X520) NICs, virtio-net on qemu with and without vhost and on VirtualBox. All other frameworks except DPDK are also restricted to very few NIC models (typically 3 or fewer families) and ixgbe is (except for OpenOnload only supporting their own NICs) always supported.

Ixy 基于自定义用户空间重新实现的英特尔 ixgbe 驱动程序和 VirtIO virtio-net 驱动程序，可以简化其基本要素。我们在 Intel X550，X540 和 82599ES（又名X520）网卡上测试了我们的ixgbe 驱动程序，在有和没有 vhost 的 qemu 上以及在 VirtualBox 上测试了virtio-net。除DPDK之外的所有其他框架也仅限于极少数 NIC 模型（通常为3个或更少的系列），并且始终支持ixgbe（仅支持自己的NIC的 OpenOnload 除外）。

We chose ixgbe for ixy because Intel releases extensive datasheets and the ixgbe NICs are commonly found in commodity servers. These NICs are also interesting because they expose a relatively low-level interface to the drivers. Other NICs like the newer Intel XL710 series or Mellanox ConnectX-4/5 follow a more  firmware-driven design: a lot of functionality is hidden behind a black-box firmware running on the NIC and the driver merely communicates via a message interface with the firmware which does the hard work. This approach has obvious advantages such as abstracting hardware details of different NICs allowing for a simpler more generic driver. However, our goal with ixy is understanding the full stack – a black-box firmware is counterproductive here and we have no plans to add support for such NICs.

我们选择ixgbe作为ixy，因为英特尔发布了大量的数据表，ixgbe网卡通常在商用服务器中找到。这些NIC也很有趣，因为它们向驱动程序公开了一个相对低级的接口。其他NIC（如较新的Intel XL710系列或Mellanox ConnectX-4/5）遵循更加固件驱动的设计：许多功能隐藏在NIC上运行的黑盒固件后面，驱动程序仅通过消息接口与做了艰苦工作的固件。这种方法具有明显的优势，例如抽象不同NIC的硬件细节，从而允许更简单的更通用的驱动程序。但是，我们对ixy的目标是了解完整的堆栈 - 黑盒固件在这里适得其反，我们没有计划添加对这些NIC的支持。

VirtIO was selected as second driver to ensure that everyone can run the code without hardware dependencies. A second interesting characteristic of VirtIO is that it’s based on PCI instead of PCIe, requiring a different approach to implement the driver in user space.

选择VirtIO作为第二个驱动程序，以确保每个人都可以在没有硬件依赖性的情况下运行代码。 VirtIO的另一个有趣特性是它基于 PCI 而不是 PCIe，需要采用不同的方法在用户空间中实现驱动程序。

### 3.3 User Space Drivers in Linux

All function names in the following sections are clickable hyperlinks to our source code on GitHub.

以下部分中的所有函数名称都是可点击的GitHub源代码的超链接。

Linux exposes all necessary interfaces to write full user space drivers via the sysfs pseudo filesystem. These file-based APIs give us full access to the device without needing to write any kernel code. Ixy unloads any kernel driver for the given PCI device to prevent conflicts, i.e., there is no driver configured for the NIC while ixy is running. The only capability that is missing is handling interrupts which could be done by using the uio pci generic driver for the NIC. Ixy only supports poll-mode at the moment to keep the code simple.

Linux公开了通过sysfs伪文件系统编写完整用户空间驱动程序所需的所有接口。 这些基于文件的API使我们无需编写任何内核代码即可完全访问设备。Ixy卸载给定PCI设备的任何内核驱动程序以防止冲突，即在 ixy 运行时没有为 NIC 配置驱动程序。缺少的唯一功能是处理中断，这可以通过使用NIC 的 uio pci 通用驱动程序来完成。Ixy目前仅支持轮询模式以保持代码简单。

One needs to understand how a driver communicates with a device to understand how a driver can be written in user space. This overview skips details but is sufficient to understand how ixy or similar frameworks work. A driver can communicate via two ways with a PCIe device: The driver can initiate an access to the device’s Base Address Registers (BARs) or the device can initiate a direct memory access (DMA) to access arbitrary main memory locations. BARs are used by the device to expose configuration and control registers to the drivers. These registers are available either via memory mapped IO (MMIO) or via x86 IO ports depending on the device, the latter way of exposing them is deprecated in PCIe.

需要了解驱动程序如何与设备通信，以了解如何在用户空间中编写驱动程序。此概述会跳过详细信息，但足以了解 ixy 或类似框架的工作原理。驱动程序可以通过两种方式与 PCIe 设备进行通信：驱动程序可以启动对设备的基址寄存器（BAR）的访问，或者设备可以启动直接存储器访问（DMA）以访问任意主存储器​​位置。设备使用 BAR 将配置和控制寄存器暴露给驱动程序。这些寄存器可通过存储器映射IO（MMIO）或通过x86 IO端口获得，具体取决于器件，后者暴露它们的方式在 PCIe 中已弃用。

#### 3.3.1 Accessing Device Registers

MMIO maps a memory area to device IO, i.e., reading from or writing to this memory area receives/sends data from/to the device. Linux exposes all BARs in the sysfs pseudo filesystem, a privileged process can simply mmap them into its address space. Devices commonly expose their configuration registers via this interface where normal reads and writes can be used to access the register. For example, ixgbe NICs expose all configuration, statistics, and debugging registers via the BAR0 address space. The datasheet [22] lists all registers as offsets in this memory area. Our implementation of this mapping can be found in `pci_map_resource()` in `pci.c`.

MMIO 将存储区域映射到设备IO，即，从该内存区域读取数据就是从该设备接收数据，从该内存区域写入数据就是向该设备发送数据。Linux 暴露了 sysfs 伪文件系统中的所有 BAR，特权进程可以简单地将它们映射到其地址空间。设备通常通过此接口公开其配置寄存器，其中可以使用正常读取和写入来访问寄存器。例如，ixgbe NIC 通过BAR0地址空间公开所有配置，统计和调试寄存器。数据表[22] 将所有寄存器列为该存储区中的偏移量。我们在 `pci.c` 中的 `pci_map_resource()` 中可以找到这种映射的实现。

VirtIO (in the version we are implementing) is unfortunately based on PCI and not on PCIe and its BAR is an IO port resource that must be accessed with the archaic IN and OUT x86 instructions requiring IO privileges. Linux can grant processes the necessary privileges via ioperm(2) [18], DPDK uses this approach for their VirtIO driver. We found it too cumbersome to initialize and use as it requires either parsing the PCIe configuration space or text files in procfs and sysfs. Linux also exposes IO port BARs via sys as files that, unlike their MMIO counterparts, cannot be mmaped. These files can be opened and accessed via normal read and write calls that are then translated to the appropriate IO port commands by the kernel. We found this easier to use and understand but slower due to the required syscall. See `pci_open_resource()` in `pci.c` and read/write - `ioX()` in device.h for the implementation.

不幸的是，VirtIO（我们正在实现的版本）基于 PCI 而不是基于 PCIe，其 BAR 是一个IO端口资源，必须使用需要IO权限的古老的 IN 和 OUT x86 指令进行访问。Linux 可以通过 ioperm 授予进程必要的权限（2）[18]，DPDK 将这种方法用于他们的 VirtIO 驱动程序。我们发现初始化和使用它太麻烦，因为它需要解析 PCIe 配置空间或 procfs 和 sysfs 中的文本文件。Linux还通过sys 公开 IO 端口 BAR 作为文件，与 MMIO 对应的文件不同，不能进行 mmaped。可以通过正常的读写调用来打开和访问这些文件，然后由内核将其转换为相应的 IO 端口命令。我们发现这更容易使用和理解，但由于所需的系统调用而变慢。请参阅 `pci.c` 中的`pci_open_resource()` 和`device.h` 中的 read/write - `ioX()` 以获取实现。

#### 3.3.2 DMA in User Space

DMA is initiated by the PCI device and allows it to read/write arbitrary physical addresses. This is used to read/write packet data and to transfer the DMA descriptors (pointers to packet data and offloading information) between driver and NIC. DMA needs to be explicitly enabled for a device via the PCI configuration space, our implementation is in enable dma() in pci.c. The user space driver hence needs to be able to translate its virtual addresses to physical addresses, this is possible via the `procfs` file `/proc/self/pagemap`, the translation logic is implemented in virt_to_phys() in memory.c.

DMA 由 PCI 设备启动，允许它 读/写 任意物理地址。这用于 读取/写入 数据包数据并在驱动程序和 NIC 之间传输 DMA 描述符（指向数据包数据和卸载信息的指针）。需要通过 PCI 配置空间为设备显式启用DMA，我们的实现是在 `pci.c` 中启用 `dma()`。用户空间驱动程序因此需要能够将其虚拟地址转换为物理地址，这可以通过 `procfs` 文件 `/proc/self/pagemap` 实现，转换逻辑 在 `memory.c`中 `virt_to_phys()` 实现。

Memory used for DMA transfer must stay resident in physical memory. mlock(2) [29] can be used to disable swapping. However, this only guarantees that the page stays backed by memory, it does not guarantee that the physical address of the allocated memory stays the same. The linux page migration mechanism can change the physical address of any page allocated by the user space at any time, e.g., to implement transparent huge pages and NUMA optimizations [31]. Linux does not implement page migration of explicitly allocated huge pages (2MiB or 1GiB pages on x86). Ixy therefore uses huge pages which also simplify allocating physically contiguous chunks of memory. Huge pages allocated in user space are used by all investigated full user space drivers, but they are often passed off as a mere performance improvement [21, 41] despite being crucial for reliable allocation of DMA memory. If Linux ever starts moving explicitly allocated huge pages in physical memory, a new memory allocation method is required for all full user space driver frameworks. The uio framework with its uio pci generic driver is one candidate.

用于 DMA 传输的内存必须驻留在物理内存中。mlock(2) [29] 可用于禁用交换。但是，这只能保证页面保持内存支持，并不能保证分配的内存的物理地址保持不变。linux页面迁移机制可以随时更改用户空间分配的任何页面的物理地址，例如，实现透明的大页面和NUMA优化[31]。Linux没有实现显式分配的大页面的页面迁移（x86上的 2MiB 或 1GiB 页面）。因此，Ixy使用大页面，这也简化了物理上连续的内存块分配。所有被调查的完整用户空间驱动程序都使用在用户空间中分配的大页面，但它们通常只是作为性能改进而传递[21,41]，尽管对于可靠分配DMA内存至关重要。如果Linux开始在物理内存中显式移动显式分配的大页面，则所有完整用户空间驱动程序框架都需要新的内存分配方法。带有uio pci通用驱动程序的uio框架是一个候选者。

### 3.4 Memory Management

Ixy builds on an API with explicit memory allocation similar to DPDK which is a very different approach from netmap [40] that exposes a replica of the NIC’s ring buffer to the application. Memory allocation for packets was cited as one of the main reasons why netmap is faster than traditional in-kernel processing [40]. Hence, netmap exposes replicas of the ring buffers 1 to the application, and it is then up to the application to handle memory. Many forwarding cases can then be implemented by simply swapping pointers in the rings. However, more complex scenarios where packets are not forwarded immediately to a NIC (e.g., because they are passed to a different core in a pipeline setting) do not map well to this API and require adding manual memory management on top of this API. Further, a ring-based API is very cumbersome to use compared to one with memory allocation.

Ixy 构建在具有类似于 DPDK 的显式内存分配的 API 上，这与netmap [40] 的一种非常不同的方法是将 NIC 的环形缓冲区的副本暴露给应用程序。数据包的内存分配被认为是 netmap 比传统内核处理更快的主要原因之一[40]。因此，netmap 将环形缓冲区 1 的副本暴露给应用程序，然后由应用程序处理内存。然后可以通过简单地交换环中的指针来实现许多转发情况。但是，更复杂的情况下，数据包不会立即转发到NIC（例如，因为它们被传递到管道设置中的不同核心）不能很好地映射到此API，并且需要在此API之上添加手动内存管理。此外，与具有存储器分配的API相比，基于环的API使用起来非常麻烦。

It is true that memory allocation for packets is a significant overhead in the Linux kernel, we have measured a per-packet overhead of 100 cycles 2 when forwarding packets with Open vSwitch on Linux for allocating and freeing packet memory (measured with `perf`). This overhead is almost completely due to (re-)initialization of the kernel `sk_buff` struct – a large data structure with a lot of metadata fields targeted at a general-purpose network stack. Memory allocation in ixy with minimum metadata required only adds an overhead of 30 cycles/packet, a price that we are willing to pay for the gained simplicity in the user-facing API.

确实，数据包的内存分配在Linux内核中是一个重要的开销，我们在 Linux 上使用 Open vSwitch 转发数据包时测量了每个数据包的100个周期开销，用于分配和释放数据包内存（用 `perf` 测量）。这种开销几乎完全归因于内核 `sk_buff` 结构的（重新）初始化 - 一个大型数据结构，其中包含许多针对通用网络堆栈的元数据字段。 ixy中的内存分配只需要最少的元数据，只会增加 30个周期/每个数据包 的开销，这是我们愿意为面向用户的 API 中获得的简单性付出的代价。

Ixy’s API is the same as DPDK’s API when it comes to sending and receiving packets and managing memory. It can best be explained by reading the example applications `ixy-fwd.c` and `ixy-pktgen.c`. The transmitonly example ixy-pktgen.c creates a memory pool, a fixed-size collection of fixed-size packet buffers and prefills them with packet data. It then allocates a batch of packets from this pool, adds a sequence number to the packet, and passes them to the transmit function. The transmit function is asynchronous: it enqueues pointers to these packets, the NIC fetches and sends them later. Previously sent packets are freed asynchronously in the transmit function by checking the queue for sent packets and returning them to the pool. This means that a packet buffer cannot be re-used immediately, the ixy-pktgen example looks therefore quite different from a packet generator built on a classic socket API.

在发送和接收数据包以及管理内存时，Ixy 的 API 与 DPDK 的 API 相同。最好通过阅读示例应用程序 `ixy-fwd.c` 和 `ixy-pktgen.c` 来解释。sendonly 示例 `ixy-pktgen.c` 创建了一个内存池，一个固定大小的固定大小的数据包缓冲区集合，并预先填充了数据包数据。然后，它从该池分配一批数据包，向数据包添加序列号，并将它们传递给传输功能。传输函数是异步的：它将这些数据包的指针排入队列，然后 NIC 会提取并发送它们。以前发送的数据包在发送功能中异步释放，方法是检查队列中是否有发送的数据包并将它们返回到池中。这意味着数据包缓冲区不能立即重用，因此 `ixy-pktgen` 示例与基于经典套接字API的数据包生成器完全不同。

The forward example `ixy-fwd.c` can avoid explicit handling of memory pools in the application: the driver allocates a memory pool for each receive ring and automatically allocates packets. Allocation is done by the packet reception function, freeing is either handled in the transmit function as before or by dropping the packet explicitly if the output link is full.

转发示例 `ixy-fwd.c` 可以避免在应用程序中显式处理内存池：驱动程序为每个接收环分配内存池并自动分配数据包。分配由分组接收功能完成，释放要么像以前那样在发送功能中处理，要么在输出链路满了时明确地丢弃分组。

### 3.5 Memory Pools and Multi-Threading

Packets may be passed to different threads, for example, a service function chaining application might run different network functions on different CPU cores and pass packets between them. Allocating and freeing packets will happen in different threads in this case as memory management is handled in the receive and transmit functions. Packets must be returned to the memory pool they were allocated from (they keep a reference to the pool) to prevent starving or overflowing pools when forwarding unidirectionally. Therefore, memory pools must be thread-safe for such an application. Memory pools in ixy are currently not thread-safe; they are based on a free list kept in a stack, making bulk operations on the pools trivial and fast. Lock-free stacks or queues could be used, but these data structures are complicated [44]. We do not want the memory pool to be the most complex and hard to understand part of ixy – we therefore do not support passing packets between threads at the moment.

数据包可以传递给不同的线程，例如，服务功能链接应用程序可能在不同的 CPU 核心上运行不同的网络功能，并在它们之间传递数据包。在这种情况下，分配和释放数据包将在不同的线程中发生，因为在接收和发送功能中处理存储器管理。必须将数据包返回到从中分配的内存池（它们保留对池的引用），以防止在单向转发时使池不足或溢出。因此，对于此类应用程序，内存池必须是线程安全的。ixy 中的内存池目前不是线程安全的，它们基于保存在堆栈中的空闲列表，使得池上的批量操作变得微不足道且快速。可以使用无锁堆栈或队列，但这些数据结构很复杂[44]。我们不希望内存池是 ixy中最复杂和最难理解的部分 - 因此我们不支持在线程之间传递数据包。

Choosing the right data structure for memory management also affects performance beyond the efficiency of the data structure itself. A stack is better than a queue: it improves temporal cache locality because it recycles packets immediately. DPDK is an interesting case study as they offer thread-safe memory pools. Free buffers are kept in a lock-free queue and each thread keeps a thread-local stack as a cache: an unsynchronized local stack is faster than a lock-free data structure. This works well if multiple threads share a memory pool but do not pass packets between each other. But the cache does not help if the "producers" and "consumers" are separate threads because they exhaust their cache and effectively fall back to the queue. DPDK added a memory pool backed by a stack protected with a spin-lock specifically for such applications because effective cache usage is more important than a lock-free data structure in practice [20].

为内存管理选择正确的数据结构也会影响性能，超出数据结构本身的效率。堆栈优于队列：它改进了临时缓存局部性，因为它可以立即回收数据包。 DPDK是一个有趣的案例研究，因为它们提供线程安全的内存池。空闲缓冲区保存在无锁队列中，每个线程将线程本地堆栈保存为缓存：非同步本地堆栈比无锁数据结构更快。如果多个线程共享内存池但不在彼此之间传递数据包，则此方法很有效。但是如果 “生产者” 和 “消费者” 是单独的线程，缓存就无济于事，因为它们耗尽了缓存并有效地回退到队列中。 DPDK添加了一个由受自旋锁保护的堆栈支持的内存池，专门用于此类应用程序，因为有效的缓存使用在实践中比无锁数据结构更重要[20]。

### 3.6 Security Considerations

Applications built with ixy require root access to access the hardware, the same is true for virtually all other packet processing frameworks. The only noteworthy exception here is netmap which can grant unprivileged users access and performs checks on user-provided data in the kernel interfaces. Despite the need for root access, the other frameworks are still an improvement over the previous solution: custom kernel modules running C code. The user space solutions can use safer languages, for example, the Snabb drivers are written in Lua 3 [17].

使用 ixy 构建的应用程序需要 root 访问才能访问硬件，几乎所有其他数据包处理框架都是如此。这里唯一值得注意的例外是 netmap，它可以授予非特权用户访问权限，并对内核接口中用户提供的数据执行检查。尽管需要root访问权限，但其他框架仍然是对以前解决方案的改进：运行C代码的自定义内核模块。用户空间解决方案可以使用更安全的语言，例如，Snabb驱动程序是用Lua 3编写的[17]。

It is possible to drop all privileges using seccomp(2) once the PCIe memory regions have been opened or mmaped and all necessary DMA memory has been allocated. We have implemented this in ixy on a branch [39]. However, this is still insecure – the device is under full control of the application and it has full access via DMA to the whole memory. Modern CPUs offer a solution: the IO memory management unit (IOMMU) allows using virtual addresses, translation, and protection for PCIe devices. IOMMUs are available on CPUs offering hardware virtualization features as it was designed to pass PCIe devices (or parts of them via SR-IOV) directly into VMs in a secure manner. Linux abstracts different IOMMU implementations via the vfio framework which is specifically designed for "safe non-privileged userspace drivers" [32] beside virtual machines. This, combined with dropping privileges after initialization (or delegating initialization to a separate process), allows implementing a secure user space driver that requires no privileged access during operation.

一旦 PCIe 内存区域被打开或 mmaped 并且已经分配了所有必需的 DMA 内存，就可以使用seccomp(2) 删除所有权限。我们已经在 ixy 的分支上实现了这个[39]。然而，这仍然是不安全的 - 设备完全控制应用程序，它通过 DMA 完全访问整个内存。现代CPU提供了一种解决方案：I/O 内存管理单元（IOMMU）允许使用 PCIe 设备的虚拟地址，转换和保护。 IOMMU可用于提供硬件虚拟化功能的CPU，因为它旨在以安全的方式将 PCIe 设备（或部分通过SR-IOV）直接传递到 VM。Linux通过vfio 框架抽象出不同的 IOMMU 实现，该框架专为虚拟机旁边的 “安全非特权用户空间驱动程序”[32]而设计。这与初始化之后删除权限（或将初始化委派给单独的进程）相结合，允许实现安全的用户空间驱动程序，该驱动程序在操作期间不需要特权访问。

## 4 ixgbe Implementation

All line numbers referenced in this Section are for commit df1cddb of ixy. All page numbers and section numbers for the Intel datasheet refer to revision 3.3 (March 2016) of the 82599ES datasheet [22]. Function names and line numbers are hyperlinked to the implementation.

本节中引用的所有行号都是针对 ixy 的 `commit df1cddb`。英特尔数据表的所有页码和节号均参考 82599ES 数据表的修订版 3.3（2016年3月）[22]。函数名称和行号超链接到实现。

ixgbe devices expose all configuration, statistics, and debugging registers via the BAR0 MMIO region. The datasheet [22] lists all registers as offsets in this configuration space. We use `ixgbe_type.h` from Intel’s driver as machine-readable version of the datasheet 4 , it contains defines for all register names and offsets for bit fields.

ixgbe 设备通过 BAR0 MMIO 区域公开所有配置，统计和调试寄存器。数据表[22]将所有寄存器列为此配置空间中的偏移量。我们使用英特尔驱动程序中的 `ixgbe_type.h` 作为数据表4的机器可读版本，它包含所有寄存器名称和位字段偏移的定义。

### 4.1 NIC Ring API

NICs expose multiple circular buffers called queues or rings to transfer packets. The simplest setup uses only one receive and one transmit queue. Multiple transmit queues are merged on the NIC, incoming traffic is split according to filters or a hashing algorithm if multiple receive queues are configured. Both receive and transmit rings work in a similar way: the driver programs a physical base address and the size of the ring. It then fills the memory area with DMA descriptors, i.e., pointers to physical addresses where the packet data is stored with some metadata. Sending and receiving packets is done by passing ownership of the DMA descriptors between driver and hardware via a head and tail pointer. The driver controls the tail, hardware the head. Both pointers are stored in device registers accessible via MMIO.

NIC 暴露多个称为队列或环的循环缓冲区以传输数据包。最简单的设置仅使用一个接收和一个传输队列。在NIC上合并多个传输队列，如果配置了多个接收队列，则根据过滤器或散列算法分割传入流量。接收和发送环都以类似的方式工作：驱动程序编程物理基地址和环的大小。然后，它用DMA描述符填充存储区，即指向物理地址的指针，其中分组数据与一些元数据一起存储。通过头尾指针在驱动程序和硬件之间传递DMA描述符的所有权来完成发送和接收数据包。驱动程序控制尾部，硬件控制头部。两个指针都存储在可通过MMIO访问的设备寄存器中。

The initialization code is in `ixgbe.c` starting from line 124 for receive queues and from line 179 for transmit queues. Further details are in the datasheet in Section 7.1.9 and in the datasheet sections mentioned in the code.

初始化代码在 `ixgbe.c` 中，从第124行开始，用于接收队列，第179行从传输队列开始。更多详细信息，请参见第7.1.9节中的数据表以及代码中提到的数据表部分。

#### 4.1.1 Receiving Packets

The driver fills up the ring buffer with physical pointers to packet buffers in `start_rx_queue()` on startup. Each time a packet is received, the corresponding buffer is returned to the application and we allocate a new packet buffer and store its physical address in the DMA descriptor and reset the ready flag. We also need a way to translate the physical addresses in the DMA descriptor found in the ring back to its virtual counterpart on packet reception. This is done by keeping a second copy of the ring populated with virtual instead of physical addresses, this is then used as a lookup table for the translation.

驱动程序在启动时用 `start_rx_queue()` 中的数据包缓冲区的物理指针填充环形缓冲区。每次收到数据包时，相应的缓冲区将返回给应用程序，我们分配一个新的数据包缓冲区并将其物理地址存储在 DMA 描述符中并重置就绪标志。我们还需要一种方法将环中找到的 DMA 描述符中的物理地址转换回数据包接收时的虚拟对应物。这是通过保留用虚拟而不是物理地址填充的环的第二个副本来完成的，然后将其用作转换的查找表。

Figure 1 illustrates the memory layout: the DMA descriptors in the ring to the left contain physical pointers to packet buffers stored in a separate location in a memory pool. The packet buffers in the memory pool contain their physical address in a metadata field. Figure 2 shows the RDH (head) and RDT (tail) registers controlling the ring buffer on the right side, and the local copy containing the virtual addresses to translate the physical addresses in the descriptors in the ring back for the application. `ixgbe_rx_batch()` in `ixgbe.c` implements the receive logic as described by Sections 1.8.2 and 7.1 of the datasheet. It operates on batches of packets to increase performance. A na¨ ıve way to check if packets have been received is reading the head register from the NIC incurring a PCIe round trip. The hardware also sets a flag in the descriptor via DMA which is far cheaper to read as the DMA write is handled by the last-level cache on modern CPUs. This is effectively the difference between an LLC cache miss and hit for every received packet.

图1 说明了内存布局：左侧环中的DMA描述符包含存储在内存池中单独位置的数据包缓冲区的物理指针。内存池中的数据包缓冲区包含元数据字段中的物理地址。图2 显示了控制右侧环形缓冲区的RDH（头部）和RDT（尾部）寄存器，以及包含虚拟地址的本地副本，用于转换应用程序环中描述符中的物理地址。 `ixgbe.c`中的 `ixgbe_rx_batch()` 实现了接收逻辑，如数据表的第1.8.2节和第7.1节所述。它对批量数据包进行操作以提高性能。检查是否已收到数据包的一种简单方法是从 NIC 中读取头部寄存器，从而产生 PCIe 往返。硬件还通过 DMA 在描述符中设置一个标志，由于 DMA 写入由现代CPU上的最后一级高速缓存处理，因此读取便宜得多。这实际上是 LLC 高速缓存未命中和每个接收到的数据包的命中之间的差异。

#### 4.1.2 Transmitting Packets

Transmitting packets follows the same concept and API as receiving them, but the function is more complicated because the interface between NIC and driver is asynchronous. Placing a packet into the ring does not immediately transfer it and blocking to wait for the transfer is infeasible. Hence, the ixgbe tx batch() function in ixgbe.c consists of two parts: freeing packets from previous calls that were sent out by the NIC followed by placing the current packets into the ring. The first part is often called cleaning and works similar to receiving packets: the driver checks a flag that is set by the hardware after the packet associated with the descriptor is sent out. Sent packet buffers can then be free’d, making space in the ring. Afterwards, the pointers of the packets to be sent are stored in the DMA descriptors and the tail pointer is updated accordingly.

传输数据包遵循与接收数据包相同的概念和API，但功能更复杂，因为 NIC 和驱动程序之间的接口是异步的。将数据包放入环中不会立即转移它，阻塞等待转移是不可行的。因此，`ixgbe.c` 中的`ixgbe_tx_batch()` 函数由两部分组成：从NIC发出的先前调用中释放数据包，然后将当前数据包放入环中。第一部分通常称为清理，其工作方式类似于接收数据包：驱动程序在发送与描述符关联的数据包之后检查由硬件设置的标志。然后可以释放已发送的数据包缓冲区，从而在环中创建空间。然后，将要发送的分组的指针存储在DMA描述符中，并相应地更新尾指针。

Checking for transmitted packets can be a bottleneck due to cache thrashing as both the device and driver access the same memory locations [22]. The 82599 hardware implements two methods to combat this: marking transmitted packets in memory occurs either automatically in configurable batches on device side, this can also avoid unnecessary PCIe transfers. We tried different configurations (code in `init_tx()`) and found that the defaults from Intel’s driver work best. The NIC can also write its current position in the transmit ring back to memory periodically (called head pointer write back) as explained in Section 7.2.3.5.2 of the datasheet. However, no other driver implements this feature despite the datasheet referring to the normal marking mechanism as "legacy". We implemented support for head pointer write back on a branch [38] but found no measurable performance improvements or effects on cache contention. 

由于设备和驱动程序访问相同的内存位置，检查传输的数据包可能是由于缓存抖动造成的瓶颈[22]。82599硬件实现了两种方法来解决这个问题：在内存中标记传输的数据包在设备端以可配置的批量自动发生，这也可以避免不必要的 PCIe 传输。我们尝试了不同的配置（`init_tx()` 中的代码），发现英特尔驱动程序的默认配置效果最好。 NIC 也可以定期将其在发送环中的当前位置写回存储器（称为头指针写回），如数据表第7.2.3.5.2节中所述。但是，尽管数据表将正常标记机制称为“遗留”，但没有其他驱动程序实现此功能。我们在分支上实现了对头指针写回的支持[38]，但没有发现可测量的性能改进或对缓存争用的影响。

#### 4.1.3 Batching

Each successful transmit or receive operation involves an update to the NIC’s tail pointer register (RDT and TDT for receive/transmit), a slow operation. This is one of the reasons why batching is so important for performance. Both the receive and transmit function are batched in ixy, updating the register only once per batch.

每个成功的发送或接收操作都涉及更新 NIC 的尾指针寄存器（RDT和TDT用于接收/发送），这是一种慢速操作。这是批处理对性能如此重要的原因之一。接收和发送功能都在ixy中进行批处理，每批更新寄存器一次。

#### 4.1.4 Offloading Features

Ixy currently only enables CRC checksum offloading. Unfortunately, packet IO frameworks (e.g., netmap) are often restricted to this bare minimum of offloading features. DPDK is the exception here as it supports almost all offloading features offered by the hardware. However, as explained earlier its receive and transmit functions pay the price for these features in the form of complexity.

Ixy目前只启用CRC校验和卸载。遗憾的是，分组IO框架（例如，netmap）通常限于这个最小的卸载特征。 DPDK是例外，因为它支持硬件提供的几乎所有卸载功能。但是，如前所述，其接收和发送功能以复杂的形式为这些功能付出代价。

We will try to find a balance and showcase selected simple offloading features in ixy in the future. These offloading features can be implemented in the receive and transmit functions, see comments in the code. This is simple for some features like VLAN tag offloading and more involved for more complex features requiring an additional descriptor containing metadata information.

我们将尝试在ixy中找到平衡并展示所选的简单卸载功能。这些卸载功能可以在接收和发送功能中实现，请参阅代码中的注释。对于某些功能（如VLAN标记卸载）而言，这很简单，对于需要包含元数据信息的附加描述符的更复杂功能则更为复杂。

## 5 Performance Evaluation

We run the ixy-fwd example under a full bidirectional load of 29.76 million packets per second (Mpps), line rate with minimum-sized packets at 2x 10Gbit/s, and compare it to a custom DPDK forwarder implementing the same features. Both forwarders modify a byte in the packet to ensure that the packet data is fetched into the L1 cache to simulate a somewhat realistic workload.

我们以每秒2976万个数据包（Mpps）的完全双向负载运行 `ixy-fwd` 示例，以 `2x 10Gbit/s` 的最小大小数据包运行线路速率，并将其与实现相同功能的自定义DPDK转发器进行比较。两个转发器都修改数据包中的一个字节，以确保将数据包数据提取到L1缓存中，以模拟一些有点现实的工作负载。

### 5.1 Throughput

To quantify the baseline performance and identify bottlenecks, we run the forwarding example while increasing the CPU’s clock frequency from 1.2GHz to 2.4GHz. Figure 3 compares the throughput of our forwarder on ixy and on DPDK when forwarding across the two ports of a dual-port NIC and when using two separate NICs. The better performance of both ixy and DPDK when using two separate NICs over one dual-port NIC indicates a hardware limit (likely at the PCIe level). We run this test on Intel X520 (82599-based) and Intel X540 NICs with identical results. Ixy requires 96 CPU cycles to forward a packet, DPDK only 61. The high performance of DPDK can be attributed to its vector transmit path utilizing SIMD instructions to handle batches even better than ixy. This transmit path of DPDK is only used if no offloading features are enabled at device configuration time, i.e., it offers a similar feature set to ixy. Disabling the vector TX path in the DPDK configuration increases the CPU cycles per packet to 91 cycles packet, still slightly faster than ixy despite doing more (checking for more offloading flags). Overall, we consider ixy fast enough for our purposes. For comparison, we have previously studied the performance of older versions of DPDK, PF RING, and netmap and measured a performance of ≈ 100 cycles/packet for DPDK and PF RING and ≈ 120 cycles/packet for netmap [14].

为了量化基线性能并识别瓶颈，我们运行转发示例，同时将 CPU 的时钟频率从 1.2GHz 提高到2.4GHz。图3 比较了在双端口 NIC 的两个端口上转发时以及使用两个单独的 NIC 时 ixy 和 DPDK 上的转发器的吞吐量。在一个双端口 NIC 上使用两个独立的 NIC 时， ixy 和 DPDK 的性能更好表示硬件限制（可能在PCIe级别）。我们在 Intel X520（基于82599）和 Intel X540 NIC 上运行此测试，结果相同。 Ixy 需要 96 个CPU周期才能转发数据包，DPDK 只需61个。DPDK 的高性能可归功于其矢量传输路径，利用 SIMD 指令处理批次甚至比ixy更好。仅当在设备配置时未启用卸载功能时才使用DPDK的这个发送路径，即，它提供与ixy类似的特征集。在DPDK配置中禁用向量 TX路径会将每个数据包的CPU周期增加到91个周期数据包，尽管做得更多（检查更多的卸载标志），仍然比 ixy 略快。总的来说，我们认为 ixy 的速度足够快。为了进行比较，我们之前已经研究过旧版 DPDK，PF_RING 和 netmap 的性能，并测量了 DPDK 和 PF_RING的 ≈100个周期/包 的性能以及 netmap 的 ≈120个周期/包 的性能[14]。

### 5.2 Batching

Batching is one of the main drivers for performance. DPDK even requires a minimum batch size of 4 when using the SIMD transmit path. Receiving or sending a packet involves an access to the queue index registers, invoking a costly PCIe round-trip. Figure 4 shows how the performance increases as the batch size is increased in the bidirectional forwarding scenario with two NICs. Increasing batch sizes have diminishing returns: this is especially visible when the CPU is only clocked at 1.2GHz. Reading the performance counters for all caches shows that the number of L1 cache misses per packet increases as the performance gains drop off. Too large batches thrash the L1 cache, possibly evicting lookup data structures in a real application. Therefore, batch sizes should not be chosen too large. Latency is also impacted by the batch size, but the effect is negligible compared to other buffers (e.g., NIC ring sizes are an order of magnitude larger than the batch size).

批处理是性能的主要驱动因素之一。使用SIMD发送路径时，DPDK甚至要求最小批量为4。接收或发送数据包涉及访问队列索引寄存器，从而调用昂贵的PCIe往返。图4显示了在具有两个NIC的双向转发方案中，随着批量大小的增加，性能如何提高。增加批量大小的回报会减少：当CPU的时钟频率仅为1.2GHz时，这一点尤为明显。读取所有高速缓存的性能计数器表明，每个数据包的L1高速缓存未命中数随着性能增益的下降而增加。太大的批次会破坏L1缓存，可能会逐出实际应用程序中的查找数据结构。因此，批量大小不应选择太大。延迟也受批量大小的影响，但与其他缓冲区相比，效果可忽略不计（例如，NIC环大小比批量大小大一个数量级）。

### 5.3 Profiling

We run perf on ixy-fwd running under full bidirectional load at 1.2GHz with two different NICs using the default batch size of 32 to ensure that CPU is the only bottleneck. perf allows profiling with the minimum possible effect on the performance: throughput drops by only ≈ 5% while perf is running. Table 1 shows where CPU time is spent on average per forwarded packet and compares it to DPDK. Receiving is slower because the receive logic performs the initial fetch, the following functions operate on the L1 cache. Ixy’s receive function still leaves room for improvements, it is less optimized than the transmit function. There are several places in the receive function where DPDK avoids memory accesses by batching compared to ixy. However, these optimizations were not applied for  simplicity in ixy: DPDK’s receive function is quite complex.

我们在 `ixy-fwd` 上运行 `perf`，在1.2GHz的完全双向负载下使用两个不同的 NIC 使用默认批量大小 32 来确保 CPU 是唯一的瓶颈。`perf`允许对性能进行最小化分析：在 `perf` 运行时，吞吐量仅下降 `≈5%`。表1显示了每个转发数据包平均花费的 CPU 时间，并将其与 DPDK 进行比较。接收速度较慢，因为接收逻辑执行初始提取，以下功能在L1高速缓存上运行。Ixy的接收功能仍然留有改进的余地，它不如传输功能优化。接收函数中有几个地方，与ixy相比，DPDK通过批处理避免了内存访问。但是，这些优化并不适用于ixy的简单性：DPDK的接收功能非常复杂。

Overhead for memory management is significant (but still low compared to 100 cycles/packet in the Linux kernel). 59% of the time is spent in non-batched memory operations and none of the calls are inlined. Inlining these functions increases throughput by 6.5% but takes away our ability to account time spent in them. Overall, the overhead of memory management is larger than we initially expected, but we still think explicit memory management for the sake of a usable API is a worthwhile trade-off. This is especially true for ixy aiming at simplicity, but also for other frameworks targeting complex applications. Simple forwarding can easily be done on an exposed ring interface, but anything more complex that does not sent out packets immediately (e.g., because they are processed further on a different core) requires memory management in the user’s application. Moreover, 30 cycles per packet that could be saved is still a tiny improvement compared to other architectural decisions like batch processing that reduces per-packet processing costs by 300 cycles when going from no batching to a batch size of 32.

内存管理的开销很大（但与Linux内核中的100个周期/数据包相比仍然很低）。59% 的时间用于非批处理内存操作，并且没有内联调用。内联这些函数可将吞吐量提高 6.5%，但却会影响我们在其中花费的时间。总体而言，内存管理的开销比我们最初预期的要大，但我们仍然认为，为了可用的API而进行明确的内存管理是一个值得的权衡。对于针对简单性的 ixy，以及针对复杂应用程序的其他框架，尤其如此。可以在暴露的环形接口上轻松地进行简单转发，但是任何更复杂的不立即发送数据包（例如，因为它们在不同的核心上进一步处理）需要在用户的应用程序中进行存储器管理。此外，与其他架构决策（如批量处理）相比，每个数据包可节省 30 个周期仍然是一个微小的改进，从无批处理到批量大小为 32 时，每个数据包的处理成本降低了 300 个周期。

### 5.4 Queue Sizes

Our driver supports descriptor ring sizes in power-of-two increments between 64 and 4096, the hardware supports more sizes but the restriction to powers of two simplify wrap-around handling. Linux defaults to a ring size of 256 for this NIC, DPDK’s example applications configure different sizes; the l2fwd forwarder sets 128/512 RX/TX descriptors. Larger ring sizes such as 8192 are sometimes recommended to increase performance [1] (source refers to the size as kB when it is actually number of packets). Figure 5 shows the throughput of ixy with various ring size combinations. There is no measurable impact on the maximum throughput for ring sizes larger than 64. Scenarios where a larger ring size can still be beneficial might exist: for example, an application producing a large burst of packets significantly faster than the NIC can handle for a very short time.

我们的驱动程序支持描述符环大小，功率为2，增量在 64 到 4096 之间，硬件支持更多大小，但对2 的幂的限制简化了环绕处理。Linux 默认为此 NIC 的环大小为 256，DPDK的示例应用程序配置不同的大小; `l2fwd` 转发器设置 128/512 个 RX/TX 描述符。有时建议使用较大的环大小（例如8192）来提高性能[1]（源指的是实际数据包数时的大小为kB）。图5显示了具有各种环尺寸组合的ixy的吞吐量。对于大于64的环大小，最大吞吐量没有可测量的影响。可能存在更大环大小的情况可能存在：例如，产生大量数据包的应用程序明显快于 NIC 可以处理的非常快短时间。

The second performance factor that is impacted by ring sizes is the overall latency caused by unnecessary buffering. Table 2 shows the latency (measured with MoonGen hardware timestamping [11]) of the ixy forwarder with different ring sizes. The results show a linear dependency between ring size and latency when the system is overloaded, but the effect under lower loads are negligible. Full or near full buffers are no exception on systems forwarding Internet traffic due to protocols like TCP that try to fill up buffers completely [15]. We conclude that tuning tipps like setting a ring size to 8192 [1] are detrimental for latency and likely do not help with throughput. Ixy uses a default ring size of 512 at the moment as a trade-off between providing some buffer and avoiding high worst-case latencies.

受环大小影响的第二个性能因素是由不必要的缓冲引起的总延迟。表2 显示了具有不同环大小的ixy前向转发器的延迟（使用MoonGen硬件时间戳[11]测量）。结果显示系统过载时环大小和延迟之间存在线性相关性，但在较低负载下的影响可忽略不计。由于像 TCP 这样试图完全填满缓冲区的协议，完全或接近完整的缓冲区在系统转发 Internet 流量时也不例外[15]。我们得出结论，调整tipps如将环大小设置为 8192 [1] 对延迟是有害的，并且可能对吞吐量没有帮助。Ixy目前使用512的默认环大小作为提供缓冲区和避免高最坏情况延迟之间的权衡。

### 5.5 Page Sizes

It is not possible to allocate DMA memory on small pages from user space in Linux in a reliable manner as described in Section 3.3.2. Despite this, we have implemented an allocator that performs a brute-force search for physically contiguous normal-sized pages from user space. We run this code on a system without NUMA and with transparent huge pages and page-merging disabled to avoid unexpected page migrations. The code for these benchmarks is not available in the main repo but on a branch [37] due to its unsafe nature on some systems. Benchmarks varying the page size are interesting despite these problems: kernel drivers (and user space packet IO frameworks using them) often only support normal-sized pages. Existing performance claims about huge pages in drivers are vague and unsubstantiated [21, 41].

如第3.3.2节所述，不可能以可靠的方式在Linux中的用户空间的小页面上分配DMA内存。尽管如此，我们已经实现了一个分配器，它可以对来自用户空间的物理连续的正常大小的页面执行强力搜索。我们在没有NUMA的系统上运行此代码，并禁用透明的大页面和页面合并以避免意外的页面迁移。这些基准测试的代码在主要仓库中没有，但在分支[37]上可用，因为它在某些系统上不安全。尽管存在以下问题，但改变页面大小的基准测试仍然很有趣：内核驱动程序（以及使用它们的用户空间数据包IO框架）通常只支持正常大小的页面。关于驱动程序中大页面的现有性能声明含糊不清且未经证实[21,41]。

Figure 6 shows that the impact on performance of huge pages in the driver is small. The performance difference is 5.5% with the maximum ring size, more realistic ring sizes only differ by 1-3%. This is not entirely unexpected: the largest queue size of 4096 entries is only 16kiB large storing pointers to up to 16MiB packet buffers. Huge pages are designed for, and usually used with, large data structures, e.g., big lookup tables for forwarding. The effect measured here is likely larger when a real forwarding application puts additional pressure on the TLB due to its other internal data structures. One should still use huge pages for other data structures in a packet processing application, but a driver not supporting them (e.g., netmap) is not as bad as one might expect when reading claims about their importance from authors of drivers supporting them.

图6显示了驱动程序中大页面性能的影响很小。最大环尺寸的性能差异为5.5%，更实际的环尺寸仅相差1-3%。这并非完全出乎意料：4096个条目的最大队列大小仅为 16kiB 大型存储指针，最多可达16MiB 数据包缓冲区。巨大的页面被设计用于并且通常与大数据结构一起使用，例如用于转发的大查找表。当真正的转发应用程序由于其其他内部数据结构而对TLB施加额外压力时，此处测量的效果可能更大。人们仍然应该在数据包处理应用程序中使用大页面用于其他数据结构，但是不支持它们的驱动程序（例如，netmap）并不像在支持它们的驱动程序的作者阅读关于它们的重要性的声明时那样糟糕。

### 5.6 NUMA Considerations

Non-uniform memory access (NUMA) architectures found on multi-CPU servers present additional challenges. Modern systems integrate cache, memory controller, and PCIe root complex in the CPU itself instead of using a separate IO hub. This means that a PCIe device is attached to only one CPU in a multi-CPU system, access from or to other CPUs needs to pass over the CPU interconnect (QPI on our system). At the same time, the tight integration of these components allows the PCIe controller to transparently write DMA data into the cache instead of main memory. This works even when DCA (direct cache access) is not used (DCA is only supported by the kernel driver, none of the full user space drivers implement it). Intel DDIO (Data Direct I/O) is another further optimization to prevent memory accesses by DMA [23]. However, we found by reading performance counters that even CPUs not supporting DDIO do not perform memory accesses in a typical packet forwarding scenario. DDIO is poorly documented and exposes no performance counters, its exact effect on modern systems is unclear. All recent (since 2012) CPUs supporting multi-CPU systems also support DDIO. Our NUMA benchmarks where obtained on a different system than the previous results because we wanted to avoid potential problems with NUMA for the other setups.

在多CP​​U服务器上的非一致性内存访问（NUMA）架构带来了额外的挑战。现代系统将高速缓存，内存控制器和 `PCIe root` 联合体集成在CPU本身中，而不是使用单独的IO集线器。这意味着 PCIe 设备仅连接到多 CPU 系统中的一个CPU，来自或来自其他 CPU 的访问需要通过 CPU 互连（我们系统上的QPI）。同时，这些组件的紧密集成允许 PCIe 控制器透明地将 DMA 数据写入缓存而不是主存储器。即使不使用 DCA（直接缓存访问）也是如此（DCA仅由内核驱动程序支持，没有一个完整的用户空间驱动程序实现它）。英特尔DDIO（数据直接I/O）是进一步优化以防止DMA访问内存[23]。但是，我们通过读取性能计数器发现，即使是不支持 DDIO 的CPU也不会在典型的数据包转发方案中执行内存访问。DDIO的文档记录很少，并且没有暴露性能计数器，它对现代系统的确切影响尚不清楚。所有最近（自2012年以来）支持多CPU系统的CPU也支持DDIO。我们的NUMA基准测试是在与以前的结果不同的系统上获得的，因为我们希望避免NUMA在其他设置方面的潜在问题。

A multi-CPU system consists of multiple NUMA nodes, each has its own CPU, memory, and PCIe devices. Our test system has one dual-port NIC attached to NUMA node 0 and a second to NUMA node 1. Both the forwarding process and the memory used for the DMA descriptors and packet buffers can be explicitly pinned to a NUMA node. This gives us 8 possible scenarios for unidirectional packet forwarding by varying the packet path and pinning. Table 3 shows the throughput at 1.2GHz. Forwarding from and to a NIC at the same node shows one unexpected result: pinning memory, but not the process itself, to the wrong NUMA node does not reduce performance. The explanation for this is that the DMA transfer is still handled by the correct NUMA node to which the NIC is attached, the CPU then caches this data while informing the other node. However, the CPU at the other node never accesses this data and there is hence no performance penalty. Forwarding between two different nodes is fastest when the the memory is pinned to the egress nodes and CPU to the ingress node and slowest when both are pinned to the ingress node. Real forwarding applications often cannot know the destination of packets at the time they are received, the best guess is therefore to pin the thread to the node local to the ingress NIC and distribute packet buffer across the nodes. Latency was also impacted by poor NUMA mapping, we measured an additional 1.7µs when unnecessarily crossing the NUMA boundary when forwarding between two ports on one NUMA node. Latency comparisons between forwarding within one node vs. forwarding between two nodes where not possible in a fair manner in this system as the NICs use different physical layers with different latencies: the 10GBASE-T NIC has more than 2µs additional latency.

多CPU系统由多个NUMA节点组成，每个节点都有自己的CPU，内存和PCIe设备。我们的测试系统有一个双端口NIC连接到NUMA节点0，第二个连接到NUMA节点1。转发过程和用于DMA描述符和数据包缓冲区的存储器都可以明确地固定到NUMA节点。这为我们提供了8种可能的方法，通过改变数据包路径和固定来实现单向数据包转发。表3显示了 1.2GHz 的吞吐量。在同一节点上从NIC转发到NIC会显示一个意外结果：将内存固定到错误的NUMA节点而不是进程本身不会降低性能。对此的解释是DMA传输仍然由NIC所连接的正确NUMA节点处理，然后CPU在通知另一节点时缓存该数据。但是，另一个节点上的CPU从不访问此数据，因此没有性能损失。当存储器被固定到出口节点并且CPU被固定到入口节点时，两个不同节点之间的转发最快，而当两者被固定到入口节点时，转发最慢。实际转发应用程序通常无法在收到数据包时知道数据包的目的地，因此最好的猜测是将线程固定到入口NIC的本地节点，并在节点之间分配数据包缓冲区。延迟也受到差的 NUMA 映射的影响，当在一个NUMA节点上的两个端口之间转发时，当不必要地越过NUMA边界时，我们测量了额外的 1.7μs。由于NIC使用具有不同延迟的不同物理层，因此在一个节点内的转发与两个节点之间的转发之间的延迟比较（在此系统中不可能以公平的方式进行）：10GBASE-T NIC具有超过2μs的额外延迟。

## 6 VirtIO Implementation

All line numbers referenced in this Section are for commit df1cddb of ixy. All section numbers for the specification refer to version 1.0 of the VirtIO specification [22]. Function names are hyperlinked to the implementation on GitHub containing further references to the relevant specification sections.

本节中引用的所有行号都是针对 ixy 的 `commit df1cddb`。规范的所有部分编号均参考VirtIO规范的版本1.0 [22]。函数名称被超链接到 GitHub 上的实现，其中包含对相关规范部分的进一步引用。

VirtIO defines different types of operational modes for emulatednetworkcards: legacy, modern, andtransitional devices. qemu implements all three modes, the default being transitional devices supporting both the legacy and modern interface after feature negotiation. Supporting devices operating only in modern mode would be the simplest implementation in ixy because they work with MMIO. Both legacy and transitional devices require support for PCI IO port resources making the device access different from the ixgbe driver. Modern-only devices are rare because they are relatively new (2016).

VirtIO 为仿真网络卡定义了不同类型的操作模式：传统设备，现代设备和传统设备。qemu 实现了所有三种模式，默认是在功能协商后支持传统和现代接口的过渡设备。仅在现代模式下运行的支持设备将是 ixy 中最简单的实现，因为它们与 MMIO 一起使用。传统和过渡设备都需要支持 PCI IO 端口资源，使设备访问与 ixgbe 驱动程序不同。现代设备很少见，因为它们相对较新（2016）。

We chose to implement the legacy variant because VirtualBox only supports the legacy operation mode. VirtualBox is an important target for ixy as it is the only hypervisor supporting VirtIO that is available on all common operating systems. Moreover, it is very well integrated with Vagrant [19] allowing us to offer a full selfcontained setup to run ixy on any platform [9].

我们选择实现传统变体，因为 VirtualBox 仅支持传统操作模式。VirtualBox 是 ixy 的重要目标，因为它是唯一支持 VirtIO 的虚拟机管理程序，可在所有常见操作系统上使用。此外，它与Vagrant [19]很好地集成，允许我们提供完整的自包含设置，以便在任何平台上运行ixy [9]。

#### 6.1 Device Initialization and Virtqueues

`virtio_legacy_init()` resets and configures a VirtIO device. It negotiates the VirtIO version and features to use, we do not try to negotiate any advanced features but the support for checksum-free transfer of packets between VMs. See specification Section 5.1.3 and 5.1.5 for the available feature flags and initialization steps.

`virtio_legacy_init()` 重置并配置 VirtIO 设备。它协商 VirtIO 版本和要使用的功能，我们不会尝试协商任何高级功能，而是支持在 VM 之间无校验和传输数据包。 有关可用的功能标志和初始化步骤，请参阅规范第5.1.3和5.1.5节。

VirtIO supports three different types of queues called Virtqueues: receive, transmit, and command queues. The queue sizes are controlled by the device and are fixed to 256 entries for legacy devices. Setup works the same as in the ixgbe driver: DMA memory for shared structures is allocated and passed to the device via a control register. Contrary to queues in ixgbe, a Virtqueue internally consists of a descriptor table and two rings: the available and used rings. While the table holds the complete descriptors with pointers to the physical addresses and length information of buffers, the rings only contain indices for this table as shown in Figure 7. To supply a device with new buffers, the driver first adds new descriptors into free slots in the descriptor table and then enqueues the slot indices into the available ring by advancing its head. Conversely, a device picks up new descriptor indices from this ring, takes ownership of them and then signals completion by enqueuing the indices into the used ring, where the driver finalizes the operation by clearing the descriptor from the table. The queue indices are maintained in DMA memory instead of in registers like in the ixgbe implementation. Therefore, the device needs to be informed about all modifications to queues, this is done by writing the queue ID into a control register in IO port memory region. Our driver also implements batching here to avoid unnecessary updates. This process is the same for sending and receiving packets. Our implementations are in `virtio_legacy` - `setup_tx/rx_queue()`.

VirtIO支持三种不同类型的队列，称为Virtqueues：接收，传输和命令队列。队列大小由设备控制，并固定为传统设备的 256 个条目。设置与 ixgbe 驱动程序的工作方式相同：共享结构的 DMA 存储器被分配并通过控制寄存器传递给设备。与 ixgbe 中的队列相反，Virtqueue 内部由描述符表和两个环组成：可用和已使用的环。虽然该表包含指向缓冲区物理地址和长度信息的指针的完整描述符，但环仅包含此表的索引，如图7所示。为了向设备提供新缓冲区，驱动程序首先将新描述符添加到空闲槽中在描述符表中，然后通过推进其头部将插槽索引排入可用环中。相反，设备从该环中获取新的描述符索引，获取它们的所有权，然后通过将索引排入已使用的环来发信号通知，其中驱动程序通过清除表中的描述符来完成操作。队列索引保存在DMA内存中，而不是像 ixgbe 实现中的寄存器中那样。因此，需要通知设备有关队列的所有修改，这是通过将队列ID写入IO端口存储区域中的控制寄存器来完成的。我们的驱动程序也在这里实现批处理以避免不必要的更此过程与发送和接收数据包相同。我们的实现在`virtio_legacy` - `setup_tx/rx_queue()`中。

The command queue is a transmit queue that is used to control most features of the device instead of via registers. For example, enabling or disabling promiscuous mode in `virtio_legacy_set_promiscuous()` is done by sending a command packet with the appropriate flags through this queue. See specification Section 5.1.6.5 for details on the command queue. This way of controlling devices is not unique to virtual devices. For example, the Intel XL710 40Gbit/s configures most features by sending messages to the firmware running on the device [24].

命令队列是一个传输队列，用于控制设备的大多数功能，而不是通过寄存器。例如，在`virtio_legacy_set_promiscuous()` 中启用或禁用混杂模式是通过在此队列中发送带有适当标志的命令包来完成的。有关命令队列的详细信息，请参阅规范第5.1.6.5节。这种控制设备的方式并非虚拟设备所独有。例如，Intel XL710 40Gbit/s 通过向设备上运行的固件发送消息来配置大多数功能[24]。

### 6.2 Packet Handling

Packet transmission in `virtio_tx_batch()` and reception in `virtio_rx_batch()` works similar to the ixgbe driver. The big difference to ixgbe is passing of metadata and offloading information. Virtqueues are not only used for VirtIO network devices, but for other VirtIO devices as well. Therefore, the DMA descriptor does not contain information specific for network devices. Packets going through Virtqueues have this information prepended in an extra header in the DMA buffer.

`virtio_tx_batch()` 中的数据包传输和 `virtio_rx_batch()` 中的接收工作类似于 ixgbe 驱动程序。与 ixgbe 的最大区别在于传递元数据和卸载信息。Virtqueues 不仅用于VirtIO网络设备，也用于其他 VirtIO 设备。因此，DMA描述符不包含特定于网络设备的信息。通过Virtqueues的数据包将此信息预先添加到DMA缓冲区中的额外标头中。

This means that the transmit function needs to prepend an additional header to each packet, and our goal to support device-agnostic applications means that the application cannot know about this requirement when allocating memory. Ixy handles this by placing this extra header in front of the packet as VirtIO DMA requires no alignment on cache lines. Our packet buffers already contain metadata before the actual packet to track the physical address and the owning memory pool. Packet data starts at an offset of one cache line (64 byte) in the packet buffer, due to alignment requirements of other NICs. This metadata cache line has enough space to accommodate the additional VirtIO header, we have explicitly marked this available area as head room for drivers requiring this. Our receive function offsets the address in the DMA descriptor by the appropriate amount to receive the extra header in the head room. The user’s ixy application treats the metadata header as opaque data.

这意味着传输功能需要为每个数据包预先添加一个标头，我们支持设备无关应用程序的目标意味着应用程序在分配内存时无法了解此要求。Ixy通过将这个额外的头部放在数据包前面来处理这个问题，因为 VirtIO DMA 不需要在缓存行上对齐。我们的数据包缓冲区已经包含实际数据包之前的元数据，以跟踪物理地址和拥有内存池。由于其他NIC的对齐要求，分组数据在分组缓冲器中的一个高速缓存行（64字节）的偏移处开始。此元数据缓存行有足够的空间容纳额外的 VirtIO 标头，我们已明确将此可用区域标记为需要此驱动程序的驱动程序的空间。我们的接收函数将DMA描述符中的地址偏移适当的数量，以在头部空间中接收额外的头部。用户的ixy应用程序将元数据标头视为不透明数据。

### 6.3 VirtIO Performance

Performance with VirtIO is dominated by the implementation of the virtual device, i.e., the hypervisor, and not the driver in the virtual machine. It is also possible to implement the hypervisor part of VirtIO, i.e., the device, in a separate user space application via the Vhost-user interface of qemu [45]. Implementations of this exist in both Snabb und DPDK. We only present baseline performance measurements running on qemu with Open vSwitch and in VirtualBox, because we are not interested in getting the fastest possible result, but results in an environment that we expect our users to have. Optimizations on the device side are out of scope for this paper.

VirtIO 的性能主要由虚拟设备（即虚拟机管理程序）的实现决定，而不是虚拟机中的驱动程序。还可以通过 qemu [45]的 Vhost 用户界面在单独的用户空间应用程序中实现 VirtIO 的管理程序部分，即设备。Snabb 和 DPDK 都存在这种实现方式。我们仅使用Open vSwitch和VirtualBox在qemu 上运行基线性能测量，因为我们不想获得最快的结果，但会产生我们希望用户拥有的环境。设备方面的优化超出了本文的范围。

Running ixy in qemu 2.7.1 on a Xeon E3-1230 V2 CPU clocked at 3.30 GHz yields a performance of only 0.94Mpps for the ixy-pktgen application and 0.36Mpps for ixy-fwd. DPDK is only marginally faster on the same setup: it manages to forward 0.4Mpps, these slow speeds are not unexpected on unoptimized hypervisors [12]. Performance is limited by packet rate, not data rate. Profiling with 1514byte packets yield near identical results with a forwarding rate of 4.8Gbit/s. VMs often send even larger packets with an offloading feature known as generic segmentation offloading offered by VirtIO to achieve higher rates. Profiling on the hypervisor shows that the interconnect is the bottleneck. It fully utilizes one core to forward packets with Open vSwitch 2.6 through the kernel to the second VM. Performance is even worse on VirtualBox 5.2 in our Vagrant setup [9]. It merely achieves 0.05Mpps on Linux with a 3.3GHz Xeon E3 CPU and 0.06Mpps on macOS with a 2.3GHz Core i7 CPU (606Mbit/s with 1514byte packets). DPDK achieves 0.08Mpps on the macOS setup. Profiling within the VM shows that over 99% of the CPU time is spent on an x86 OUT IO instruction to communicate with the virtual device/hypervisor.

在时钟频率为 3.30 GHz 的 Xeon E3-1230 V2 CPU上以qemu 2.7.1运行ixy，`ixy-pktgen` 应用的性能仅为 0.94Mpps，`ixy-fwd` 的性能仅为0.36Mpps。DPDK在相同的设置上仅稍微快一些：它设法转发 `0.4Mpps`，这些低速在未经优化的虚拟机管理程序上并不出乎意料[12]。性能受数据包速率限制，而不受数据速率的限制。使用1514byte数据包进行性能分析产生几乎相同的结果，转发速率为 4.8Gbit/s。虚拟机通常使用卸载功能发送更大的数据包，称为VirtIO提供的通用分段卸载，以实现更高的速率。虚拟机管理程序上的分析表明互连是瓶颈。它充分利用一个内核将 Open vSwitch 2.6通过内核转发到第二个VM。在我们的Vagrant设置中，VirtualBox 5.2的性能更差[9]。它仅使用3.3GHz Xeon E3 CPU 在 Linux上 实现 0.05Mpps，在具有2.3GHz Core i7 CPU（606Mbit / s，1514byte数据包）的macOS上实现0.06Mpps。DPDK在macOS设置上达到0.08Mpps。虚拟机内的分析表明，超过99％的CPU时间花费在x86 OUT IO指令上，以与虚拟设备/虚拟机管理程序进行通信。

## 7 Conclusions: Reproducible Research

We discussed how to build a user space driver for NICs of the ixgbe family which are commonly found in servers and for virtual VirtIO NICs. Our goal is not build yet another packet IO framework – but a tool for education. Therefore, reproducible research is important to us.

我们讨论了如何为 ixgbe 系列的NIC构建用户空间驱动程序，这些驱动程序通常位于服务器和虚拟VirtIO NIC 中。我们的目标不是建立另一个数据包IO框架 - 而是一种教育工具。因此，可重复的研究对我们很重要。

The full code of ixy and the scripts used to to reproduce these results is available on GitHub [8, 10]. Our DPDK forwarding application used for comparison is available in [10]. We used commit df1cddbb of ixy for the evaluation of ixgbe and virtio, commit a0f618d on a branch [37] for the normal sized pages. Most results were obtained on an Intel Xeon E5-2620 v3 2.4 GHz CPU running Debian 9.3 (kernel 4.9) with a dual port Intel X520-T2 (82599ES) NIC and a dual port X540-T2 NIC. The NUMA results where obtained on a system with two Intel Xeon E5-2630 v4 2.2 GHz CPUs with the same NICs and operating system. Turboboost, Hyper-Threading, and power-saving features were disabled. VirtIO results were obtained on various systems and hypervisors as described in the evaluation section. All loads where generated with MoonGen [11] and its l2-load-latency.lua script.

gitHub [8,10]提供了完整的ixy代码和用于重现这些结果的脚本。我们用于比较的 DPDK 转发应用程序可在[10]中找到。我们使用 ixy 的 `commit df1cddbb` 来评估 ixgbe 和 virtio，在正常大小的页面的分支[37]上提交 `a0f618d`。大多数结果是在运行 Debian 9.3（内核4.9）的Intel Xeon E5-2620 v3 2.4 GHz CPU上获得的，其中包括双端口Intel X520-T2（82599ES）NIC和双端口X540-T2 NIC。NUMA结果是在具有两个具有相同NIC和操作系统的Intel Xeon E5-2630 v4 2.2 GHz CPU的系统上获得的。Turboboost，超线程和省电功能已被禁用。如评估部分所述，在各种系统和管理程序上获得VirtIO结果。使用MoonGen [11]及其l2-load-latency.lua脚本生成的所有负载。

Our performance evaluation offers some unprecedented looks into performance of user space drivers. Ixy allows us to assess effects of individual optimizations, like DMA buffers allocated on huge pages, in isolation. Our driver allowed for a simple port to normalsized pages, this would be significant change in other frameworks 5 . Not everyone has access to servers with 10Gbit/s NICs to reproduce these results. However, everyone can build a VM setup to test ixy with our VirtIO driver. Our Vagrant setup is the simplest way to run ixy in a VM on any operating system in VirtualBox, instructions are in our repository [9]. VirtualBox turned out to be slower by a factor of 20 than a setup on qemu-kvm which is also relatively easy to build. We have validated Ixy’s functionality on a Proxmox 4.4 hypervisor and with virsh/libvirt on Ubuntu 16.04.

我们的性能评估提供了前所未有的用户空间驱动程序性能。Ixy允许我们单独评估单个优化的效果，例如在大页面上分配的DMA缓冲区。我们的驱动程序允许一个简单的端口到 normalsized 页面，这将是其他框架中的重大变化5。并非所有人都可以访问具有 10Gbit/s NIC 的服务器来重现这些结果。但是，每个人都可以使用我们的 VirtIO 驱动程序构建 VM 设置来测试 ixy。我们的 Vagrant设置是在 VirtualBox 中任何操作系统上运行 ixy 的最简单方法，指令在我们的存储库中[9]。事实上，VirtualBox 比 qemu-kvm 上的设置要慢20倍，而 qemu-kvm 也相对容易构建。我们在Proxmox 4.4虚拟机管理程序上验证了 Ixy 的功能，在Ubuntu 16.04上验证了virsh/libvirt。

## References

* [1] B AINBRIDGE , J., AND M AXWELL , J. **Red Hat Enterprise Linux Network Performance Tuning Guide**. Red Hat Documentation (Mar. 2015). Available at * https://access.redhat.com/sites/default/files/attachments/20150325_network_performance_tuning.pdf.
* [2] B ARBETTE , T., S OLDANI , C., AND M ATHY , L. Fast userspace packet processing. In ACM/IEEE ANCS (2015).
* [3] B ERTIN , G. Single RX queue kernel bypass in Netmap for high packet rate networking, Oct. 2015. https://blog.cloudflare.com/single-rx-queue-kernel-bypass-with-netmap/.
* [4] B ONELLI , N., G IORDANO , S., AND P ROCISSI , G. Network traffic processing with pfq. IEEE Journal on Selected Areas in Communications 34, 6 (June 2016), 1819–1833.
* [5] DPDK P ROJECT . DPDK: Supported NICs. http://dpdk.org/doc/nics. Last visited 2018-02-01.
* [6] DPDK P ROJECT . DPDK User Guide: Overview of Networking Drivers. http://dpdk.org/doc/guides/nics/overview.html. Last visited 2018-02-01.
* [7] DPDK P ROJECT . DPDK Website. http://dpdk.org/. Last visited 2018-02-01.
* [8] E MMERICH , P. ixy code. https://github.com/emmericp/ixy.
* [9] E MMERICH , P. ixy Vagrant setup. https://github.com/emmericp/ixy/tree/master/vagrant.
* [10] E MMERICH , P. Scripts used for the performance evaluation. https://github.com/emmericp/ixy-perf-measurements/tree/full-paper.
* [11] E MMERICH , P., G ALLENM ¨ ULLER , S., R AUMER , D., W OHL - FART , F., AND C ARLE , G. MoonGen: A Scriptable High-Speed Packet Generator. In Internet Measurement Conference 2015 * (IMC’15) (Tokyo, Japan, Oct. 2015).
* [12] E MMERICH , P., R AUMER , D., G ALLENM ¨ ULLER , S., W OHL - FART , F., AND C ARLE , G. Throughput and Latency of Virtual Switching with Open vSwitch: A Quantitative Analysis. * Journal of Network and Systems Management (July 2017).
* [13] F REE BSD P ROJECT . NETMAP(4). In FreeBSD Kernel Interfaces Manual (2017), FreeBSD 11.1-RELEASE.
* [14] G ALLENMLLER , S., E MMERICH , P., W OHLFART , F., R AUMER , D., AND C ARLE , G. Comparison of Frameworks for High-Performance Packet IO. In Architectures for Networking and * Communications Systems (ANCS) (Oakland, CA, 2015), ACM, pp. 29–38.
* [15] G ETTYS , J., AND N ICHOLS , K. Bufferbloat: Dark buffers in the internet. Queue 9, 11 (2011), 40.
* [16] G ILBERTO B ERTIN . XDP in practice: integrating XDP into our DDoS mitigation pipeline. In Netdev 2.1, The Technical Conference on Linux Networking (May 2017).
* [17] G ORRIE , L ET AL . Snabb: Simple and fast packet networking. https://github.com/snabbco/snabb.
* [18] H AARDT , M. ioperm(2). In Linux Programmer’s Manual (1993).
* [19] H ASI C ORP . Vagrant website. https://www.vagrantup.com/. Last visited 2018-02-02.
* [20] H UNT , D. mempool: add stack (lifo) mempool handler, 2016. Mailing list post. http://dpdk.org/ml/archives/dev/2016-July/043106.html.
* [21] I NTEL . DPDK Getting Started Guide for Linux. http://dpdk.org/doc/guides/linux_gsg/sys_reqs.html. Last visited 2018-02-01.
* [22] Intel 82599 10 GbE Controller Datasheet Rev. 3.3. Intel.
* [23] Intel Data Direct I/O Technology (Intel DDIO): A Primer. Available at https://www.intel.com/content/www/us/en/io/data-direct-i-o-technology-brief.html.
* [24] Intel Ethernet Controller XL710 Datasheet Rev. 2.1. Intel.
* [25] IO V ISOR P ROJECT . BPF and XDP Features by Kernel Version. https://github.com/iovisor/bcc/blob/master/docs/kernel-versions.md#xdp. Last visited 2018-02-01.
* [26] IO V ISOR P ROJECT . Introduction to XDP. https://www.iovisor.org/technology/xdp Last visited 2018-02-01.
* [27] JIM T HOMPSON . DPDK, VPP & pfSense 3.0. In DPDK Summit Userspace (Sept. 2017).
* [28] JONATHAN C ORBET . User-space networking with Snabb. In LWN.net (Feb. 2017).
* [29] KERRISK , M. mlock(2). In Linux Programmer’s Manual (2004).
* [30] LINUX F OUNDATION . Networking Industry Leaders Join Forces to Expand New Open Source Community to Drive Development of the DPDK Project, Apr. 2017. Press release.
* [31] LINUX K ERNEL D OCUMENTATION . Page migration. https://www.kernel.org/doc/Documentation/vm/page_migration.
* [32] LINUX K ERNEL D OCUMENTATION . VFIO - Virtual Function I/O. https://www.kernel.org/doc/Documentation/
* vfio.txt.
* [33] MORRIS , R., K OHLER , E., J ANNOTTI , J., AND F RANS K AASHOEK , M. The click modular router. In Operating Systems Review - SIGOPS (Dec. 1999), vol. 33, pp. 217–231.
* [34] NTOP . PF RING ZC (Zero Copy). http://www.ntop.org/products/packet-capture/pf_ring/pf_ring-zc-zero-copy/. Last visited 2017-11-30.
* [35] O PEN V S WITCH P ROJECT . Open vSwitch with DPDK. http://docs.openvswitch.org/en/latest/intro/install/dpdk/ Last visited 2018-02-01.
* [36] P FAFF , B., P ETTIT , J., K OPONEN , T., J ACKSON , E., Z HOU , A., R AJAHALME , J., G ROSS , J., W ANG , A., S TRINGER , J., S HELAR , P., A MIDON , K., AND C ASADO , M. The * design and implementation of open vswitch. In 12th USENIX Symposium on Networked Systems Design and Implementation (NSDI 15) (Oakland, CA, 2015), USENIX Association, pp. 117–130.
* [37] P UDELKO , M. ixy - DMA allocator on normal-sized pages. https://github.com/pudelkoM/ixy/tree/contiguous-pages.
* [38] P UDELKO , M. ixy - head pointer writeback implementation. https://github.com/pudelkoM/ixy/tree/head-pointer-writeback.
* [39] P UDELKO , M. ixy - seccomp implementation. https://github.com/pudelkoM/ixy/tree/seccomp.
* [40] R IZZO , L. netmap: A Novel Framework for Fast Packet I/O. In USENIX Annual Technical Conference (2012), pp. 101–112.
* [41] S NABB P ROJECT . Tuning the performance of the lwaftr. https://github.com/snabbco/snabb/blob/master/src/program/lwaftr/doc/performance.md. Last visited 2018-02-01.
* [42] S NORT P ROJECT . Snort 3 User Manual. https://www.snort.org/downloads/snortplus/snort_manual.pdf Last visited 2018-02-01.
* [43] S OLARFLARE . OpenOnload Website. http://www.openonload.org/. Last visited 2017-11-30.
* [44] S UTTER , H. Lock-Free Code: A False Sense of Security. Dr. Dobb’s Journal (Sept. 2008).
* [45] V IRTUAL O PEN S YSTEMS S ARL . Vhost-user Protocol, 2014. https://github.com/qemu/qemu/blob/stable-2.10/docs/interop/vhost-user.txt.
* [46] Y ASUKATA , K., H ONDA , M., S ANTRY , D., AND E GGERT , L.StackMap: Low-Latency Networking with the OS Stack and Dedicated NICs. In 2016 USENIX Annual Technical Conference (USENIX ATC 16) (Denver, CO, 2016), USENIX Association, pp. 43–56.