# Memory

System main memory stores application and kernel instructions, their working data, and file system caches. The secondary storage for this data is typically the storage devices—the disks—which operate orders of magnitude more slowly. Once main memory has filled, the system may begin switching data between main memory and the storage devices. This is a slow process that will often become a system bottleneck, dramatically decreasing performance. The system may also terminate the largest memory-consuming process, causing application outages.

系统主存存储应用程序和内核指令，以及它们的工作数据以及文件系统缓存。 此数据的辅助存储通常是存储设备（磁盘），它们的运行速度要慢几个数量级。 一旦主存已满，系统就可以开始在主存和存储设备（磁盘）之间切换数据。 这是一个缓慢的过程，通常会成为系统瓶颈，从而大大降低性能。 系统还可能终止最大的内存消耗过程，从而导致应用程序中断。

Other performance factors to consider include the CPU expense of allocating and freeing memory, copying memory, and managing memory address space mappings. On multisocket architectures, memory locality can become a factor, as memory attached to local sockets has lower access latency than remote sockets.

其他需要考虑的性能因素包括分配和释放内存，复制内存以及管理内存地址空间映射的CPU开销。 在多插槽体系结构上，内存局部性可能成为一个因素，因为连接到本地套接字的内存比远程套接字具有更低的访问延迟。

The learning objectives of this chapter are 本章的学习目标是：

* Understand memory concepts. <br> 了解内存概念。
* Become familiar with memory hardware internals. <br> 熟悉内存硬件内部。
* Become familiar with kernel and user allocator internals. <br> 熟悉内核和用户分配器内部。
* Have a working knowledge of the MMU and TLB. <br> 对MMU和TLB有一定的了解。
* Follow different methodologies for memory analysis. <br> 遵循不同的方法进行内存分析。
* Characterize system-wide and per-process memory usage. <br> 表征系统范围和每个进程的内存使用情况。
* Identify issues caused by low available memory. <br> 确定由可用内存不足引起的问题。
* Locate memory usage in a process address space and kernel slabs. <br> 在进程地址空间和内核平台中找到内存使用情况。
* Investigate memory usage using profilers, tracers, and flame graphs. <br> 使用探查器，跟踪器和火焰图调查内存使用情况。
* Become aware of tunable parameters for memory. <br> 意识到内存的可调参数。

This chapter has five parts, the first three providing the basis for memory analysis, and the last two showing its practical application to Linux-based systems. The parts are as follows:

本章分为五个部分，前三个部分提供了内存分析的基础，后两个部分显示了其在基于Linux的系统中的实际应用。 这些部分如下：

* **Background** introduces memory-related terminology and key memory performance concepts. <br> **背景技术**介绍了与内存有关的术语和关键的内存性能概念。
* **Architecture** provides generic descriptions of hardware and software memory architecture. <br> **体系结构**提供了硬件和软件存储器体系结构的一般描述。
* **Methodology** explains performance analysis methodology. <br> **方法论**解释了性能分析方法论。
* **Observability Tools** describes performance tools for memory analysis. <br> **观察工具**介绍了用于内存分析的性能工具。
* **Tuning** explains tuning and example tunable parameters. <br> **调优**说明了调优和示例可调参数。

The on-CPU memory caches (Level 1/2/3, TLB) are covered in Chapter 6, CPUs.

第6章，CPU中介绍了CPU上的内存缓存（级别1/2/3，TLB）。

## 7.1 TERMINOLOGY 术语

For reference, memory-related terminology used in this chapter includes the following:

* **Main memory**: Also referred to as physical memory, this describes the fast data storage area of a computer, commonly provided as DRAM.
* **Virtual memory**: An abstraction of main memory that is (almost) infinite and non-contended. Virtual memory is not real memory.
* **Resident memory**: Memory that currently resides in main memory.
* **Anonymous memory**: Memory with no file system location or path name. It includes the working data of a process address space, called the heap.
* **Address space**: A memory context. There are virtual address spaces for each process, and for the kernel.
* **Segment**: An area of virtual memory flagged for a particular purpose, such as for storing executable or writeable pages.
* **Instruction text**: Refers to CPU instructions in memory, usually in a segment.
* **OOM**: Out of memory, when the kernel detects low available memory.
* **Page**: A unit of memory, as used by the OS and CPUs. Historically it is either 4 or 8 Kbytes. Modern processors have multiple page size support for larger sizes.
* **Page fault**: An invalid memory access. These are normal occurrences when using on-demand virtual memory.
* **Paging**: The transfer of pages between main memory and the storage devices.
* **Swapping**: Linux uses the term swapping to refer to anonymous paging to the swap device (the transfer of swap pages). In Unix and other operating systems, swapping is the transfer of entire processes between main memory and the swap devices. This book uses the Linux version of the term.
* **Swap**: An on-disk area for paged anonymous data. It may be an area on a storage device, also called a physical swap device, or a file system file, called a swap file. Some tools use the term swap to refer to virtual memory (which is confusing and incorrect).

Other terms are introduced throughout this chapter. The Glossary includes basic terminology for reference if needed, including address, buffer, and DRAM. Also see the terminology sections in Chapters 2 and 3.

## 7.2 CONCEPTS 概念

The following are a selection of important concepts regarding memory and memory performance.

### 7.2.1 Virtual Memory

Virtual memory is an abstraction that provides each process and the kernel with its own large, linear, and private address space. It simplifies software development, leaving physical memory placement for the operating system to manage. It also supports multitasking (virtual address spaces are separated by design) and oversubscription (in-use memory can extend beyond main memory). Virtual memory was introduced in Chapter 3, Operating Systems, Section 3.2.8, Virtual Memory. For historical background, see [Denning 70].

Figure 7.1 shows the role of virtual memory for a process, on a system with a swap device (secondary storage). A page of memory is shown, as most virtual memory implementations are page-based.

![Figure 7.1 Process virtual memory](./chapter-07/07-01.png)
<center>Figure 7.1 Process virtual memory</center>

The process address space is mapped by the virtual memory subsystem to main memory and the physical swap device. Pages of memory can be moved between them by the kernel as needed, a process Linux calls swapping (and other OSes call anonymous paging). This allows the kernel to oversubscribe main memory.

The kernel may impose a limit to oversubscription. A commonly used limit is the size of main memory plus the physical swap devices. The kernel can fail allocations that try to exceed this limit. Such “out of virtual memory” errors can be confusing at first glance, since virtual memory itself is an abstract resource.

Linux also allows other behaviors, including placing no bounds on memory allocation. This is termed overcommit and is described after the following sections on paging and demand paging, which are necessary for overcommit to work.

### 7.2.2 Paging

Paging is the movement of pages in and out of main memory, which are referred to as page-ins and page-outs, respectively. It was first introduced by the Atlas Computer in 1962 [Corbató 68], allowing:

* Partially loaded programs to execute
* Programs larger than main memory to execute
* Efficient movement of programs between main memory and storage devices

These abilities are still true today. Unlike the earlier technique of swapping out entire programs, paging is a fine-grained approach to managing and freeing main memory, since the page size unit is relatively small (e.g., 4 Kbytes).

Paging with virtual memory (*paged virtual memory*) was introduced to Unix via BSD [Babaoglu 79] and became the standard.

With the later addition of the page cache for sharing file system pages (see Chapter 8, File Systems), two different types of paging became available: file system paging and anonymous paging.

#### File System Paging

File system paging is caused by the reading and writing of pages in memory-mapped files. This is normal behavior for applications that use file memory mappings (mmap(2)) and on file systems that use the page cache (most do; see Chapter 8, File Systems). It has been referred to as “good” paging [McDougall 06a].

When needed, the kernel can free memory by paging some out. This is where the terminology gets a bit tricky: if a file system page has been modified in main memory (called dirty), the page-out will require it to be written to disk. If, instead, the file system page has not been modified (called clean), the page-out merely frees the memory for immediate reuse, since a copy already exists on disk. Because of this, the term page-out means that a page was moved out of memory—which may or may not have included a write to a storage device (you may see the term page-out defined differently in other texts).

#### Anonymous Paging (Swapping)

Anonymous paging involves data that is private to processes: the process heap and stacks. It is termed anonymous because it has no named location in the operating system (i.e., no file system path name). Anonymous page-outs require moving the data to the physical swap devices or swap files. Linux uses the term swapping to refer to this type of paging.

Anonymous paging hurts performance and has therefore been referred to as “bad” paging [McDougall 06a]. When applications access memory pages that have been paged out, they block on the disk I/O required to read them back to main memory.1 This is an anonymous page-in, which introduces synchronous latency to the application. Anonymous page-outs may not affect application performance directly, as they can be performed asynchronously by the kernel.

> <sup>1</sup>If faster storage devices are used as swap devices, such as 3D XPoint with sub 10 μs latency, swapping may not be the same “bad” paging it once was, but rather become a simple way to intentionally extend main memory, one with mature kernel support.

Performance is best when there is no anonymous paging (swapping). This can be achieved by configuring applications to remain within the main memory available and by monitoring page scanning, memory utilization, and anonymous paging, to ensure that there are no indicators of a memory shortage.

### 7.2.3 Demand Paging

Operating systems that support demand paging (most do) map pages of virtual memory to physical memory on demand, as shown in Figure 7.2. This defers the CPU overhead of creating the mappings until they are actually needed and accessed, instead of at the time a range of memory is first allocated.

![Figure 7.2 Page fault example](./chapter-07/07-02.png)
<center>Figure 7.2 Page fault example</center>

The sequence shown in Figure 7.2 begins with a malloc() (step 1) that provides allocated memory, and then a store instruction (step 2) to that newly allocated memory. For the MMU to determine the main memory location of the store, it performs a virtual to physical lookup (step 3) for the page of memory, which fails as there is not yet a mapping. This failure is termed a page fault (step 4), which triggers the kernel to create an on-demand mapping (step 5). Sometime later, the page of memory could be paged out to the swap devices to free up memory (step 6).

Step 2 could also be a load instruction in the case of a mapped file, which should contain data but isn’t yet mapped to this process address space.

If the mapping can be satisfied from another page in memory, it is called a minor fault. This may occur for mapping a new page from available memory, during memory growth of the process (as pictured). It can also occur for mapping to another existing page, such as reading a page from a mapped shared library.

Page faults that require storage device access (not shown in this figure), such as accessing an uncached memory-mapped file, are called major faults.

The result of the virtual memory model and demand allocation is that any page of virtual memory may be in one of the following states:

1. Unallocated
2. Allocated, but unmapped (unpopulated and not yet faulted)
3. Allocated, and mapped to main memory (RAM)
4. Allocated, and mapped to the physical swap device (disk)

State (D) is reached if the page is paged out due to system memory pressure. A transition from (B) to (C) is a page fault. If it requires disk I/O, it is a major page fault; otherwise, a minor page fault.

From these states, two memory usage terms can also be defined:

* **Resident set size (RSS)**: The size of allocated main memory pages (C)
* **Virtual memory size**: The size of all allocated areas (B + C + D)

Demand paging was added to Unix via BSD, along with paged virtual memory. It has become the standard and is used by Linux.

### 7.2.4 Overcommit

Linux supports the notion of overcommit, which allows more memory to be allocated than the system can possibly store—more than physical memory and swap devices combined. It relies on demand paging and the tendency of applications to not use much of the memory they have allocated.

With overcommit, application requests for memory (e.g., malloc(3)) will succeed when they would otherwise have failed. Instead of allocating memory conservatively to remain within virtual memory limits, an application programmer can allocate memory generously and later use it sparsely on demand.

On Linux, the behavior of overcommit can be configured with a tunable parameter. See Section 7.6, Tuning, for details. The consequences of overcommit depend on how the kernel manages memory pressure; see the discussion of the OOM killer in Section 7.3, Architecture.

### 7.2.5 Process Swapping

Process swapping is the movement of entire processes between main memory and the physical swap device or swap file. This is the original Unix technique for managing main memory and is the origin of the term swap [Thompson 78].

To swap out a process, all of its private data must be written to the swap device, including the process heap (anonymous data), its open file table, and other metadata that is only needed when the process is active. Data that originated from file systems and has not been modified can be dropped and read from the original locations again when needed.

Process swapping severely hurts performance, as a process that has been swapped out requires numerous disk I/O to run again. It made more sense on early Unix for the machines of the time, such as the PDP-11, which had a maximum process size of 64 Kbytes [Bach 86]. (Modern systems allow process sizes measured in the Gbytes.)

This description is provided for historical background. Linux systems do not swap processes at all and rely only on paging.

### 7.2.6 File System Cache Usage

It is normal for memory usage to grow after system boot as the operating system uses available memory to cache the file system, improving performance. The principle is: If there is spare main memory, use it for something useful. This can distress naïve users who see the available free memory shrink to near zero sometime after boot. But it does not pose a problem for applications, as the kernel should be able to quickly free memory from the file system cache when applications need it.

For more about the various file system caches that can consume main memory, see Chapter 8, File Systems.

### 7.2.7 Utilization and Saturation

Main memory utilization can be calculated as used memory versus total memory. Memory used by the file system cache can be treated as unused, as it is available for reuse by applications.

If demands for memory exceed the amount of main memory, main memory becomes saturated. The operating system may then free memory by employing paging, process swapping (if supported), and, on Linux, the OOM killer (described later). Any of these activities is an indicator of main memory saturation.

Virtual memory can also be studied in terms of capacity utilization, if the system imposes a limit on the amount of virtual memory it is willing to allocate (Linux overcommit does not). If so, once virtual memory is exhausted, the kernel will fail allocations; for example, malloc(3) fails with errno set to ENOMEM.

Note that the currently available virtual memory on a system is sometimes (confusingly) called available swap.

### 7.2.8 Allocators

While virtual memory handles multitasking of physical memory, the actual allocation and placement within a virtual address space are often handled by allocators. These are either user-land libraries or kernel-based routines, which provide the software programmer with an easy interface for memory usage (e.g., malloc(3), free(3)).

Allocators can have a significant effect on performance, and a system may provide multiple user-level allocator libraries to pick from. They can improve performance by use of techniques including per-thread object caching, but they can also hurt performance if allocation becomes fragmented and wasteful. Specific examples are covered in Section 7.3, Architecture.

### 7.2.9 Shared Memory

Memory can be shared between processes. This is commonly used for system libraries to save memory by sharing one copy of their read-only instruction text with all processes that use it.

This presents difficulties for observability tools that show per-process main memory usage. Should shared memory be included when reporting the total memory size of a process? One technique in use by Linux is to provide an additional measure, the proportional set size (PSS), which includes private memory (not shared) plus shared memory divided by the number of users. See Section 7.5.9, pmap, for a tool that can show PSS.

### 7.2.10 Working Set Size

Working set size (WSS) is the amount of main memory a process frequently uses to perform work. It is a useful concept for memory performance tuning: performance should greatly improve if the WSS can fit into the CPU caches, rather than main memory. Also, performance will greatly degrade if the WSS exceeds the main memory size, and the application must swap to perform work.

While useful as a concept, it is difficult to measure in practice: there is no WSS statistic in observability tools (they commonly report RSS, not WSS). Section 7.4.10, Memory Shrinking, describes an experimental methodology for WSS estimation, and Section 7.5.12, wss, shows an experimental working set size estimation tool, wss(8).

### 7.2.11 Word Size

As introduced in Chapter 6, CPUs, processors may support multiple word sizes, such as 32-bit and 64-bit, allowing software for either to run. As the address space size is bounded by the addressable range from the word size, applications requiring more than 4 Gbytes of memory are too large for a 32-bit address space and need to be compiled for 64 bits or higher.2

> <sup>2</sup>There is also the Physical Address Extension (PAE) feature (workaround) for x86 allowing 32-bit processors to access larger memory ranges (but not in a single process).

Depending on the kernel and processor, some of the address space may be reserved for kernel addresses and is unavailable for application use. An extreme case is Windows with a 32-bit word size, where by default 2 Gbytes is reserved for the kernel, leaving only 2 Gbytes for the application [Hall 09]. On Linux (or Windows with the /3GB option enabled) the kernel reservation is 1 Gbyte. With a 64-bit word size (if the processor supports it) the address space is so much larger that the kernel reservation should not be an issue.

Depending on the CPU architecture, memory performance may also be improved by using larger bit widths, as instructions can operate on larger word sizes. A small amount of memory may be wasted, in cases where a data type has unused bits at the larger bit width.

## 7.3 ARCHITECTURE

This section introduces memory architecture, both hardware and software, including processor and operating system specifics.

These topics have been summarized as background for performance analysis and tuning. For more details, see the vendor processor manuals and texts on operating system internals listed at the end of this chapter.

### 7.3.1 Hardware

Memory hardware includes main memory, buses, CPU caches, and the MMU.

#### Main Memory

The common type of main memory in use today is *dynamic random-access memory* (DRAM). This is a type of volatile memory—its contents are lost when power is lost. DRAM provides high-density storage, as each bit is implemented using only two logical components: a capacitor and a transistor. The capacitor requires a periodic refresh to maintain charge.

Enterprise servers are configured with different amounts of DRAM depending on their purpose, typically ranging from one Gbyte to one Tbyte and larger. Cloud computing instances are typically smaller, ranging between 512 Mbytes and 256 Gbytes each.3 However, cloud computing is designed to spread load over a pool of instances, so they can collectively bring much more DRAM online for a distributed application, although at a much higher coherency cost.

> <sup>3</sup>Exceptions include the AWS EC2 high memory instances, which reach 24 Tbytes of memory [Amazon 20].

#### Latency

The access time of main memory can be measured as the column address strobe (CAS) latency: the time between sending a memory module the desired address (column) and when the data is available to be read. This varies depending on the type of memory (for DDR4 it is around 10 to 20ns [Crucial 18]). For memory I/O transfers, this latency may occur multiple times for a memory bus (e.g., 64 bits wide) to transfer a cache line (e.g., at 64 bytes wide). There are also other latencies involved with the CPU and MMU for then reading the newly available data. Read instructions avoid these latencies when they return from a CPU cache; write instructions may avoid them as well, if the processor supports write-back caching (e.g., Intel processors).

#### Main Memory Architecture

An example main memory architecture for a generic two-processor uniform memory access (UMA) system is shown in Figure 7.3.

![Figure 7.3 Example UMA main memory architecture, two-processor](./chapter-07/07-03.png)
*<center>Figure 7.3 Example UMA main memory architecture, two-processor</center>*

Each CPU has uniform access latency to all of memory, via a shared system bus. When managed by a single operating system kernel instance that runs uniformly across all processors, this is also a symmetric multiprocessing (SMP) architecture.

For comparison, an example two-processor non-uniform memory access (NUMA) system is shown in Figure 7.4, which uses a CPU interconnect that becomes part of the memory architecture. For this architecture, the access time for main memory varies based on its location relative to the CPU.

![Figure 7.4 Example NUMA main memory architecture, two-processor](./chapter-07/07-04.png)
*<center>Figure 7.4 Example NUMA main memory architecture, two-processor</center>*

CPU 1 can perform I/O to DRAM A directly, via its memory bus. This is referred to as local memory. CPU 1 performs I/O to DRAM B via CPU 2 and the CPU interconnect (two hops). This is referred to as remote memory and has a higher access latency.

The banks of memory connected to each CPU are referred to as memory nodes, or just nodes. The operating system may be aware of the memory node topology based on information provided by the processor. This then allows it to assign memory and schedule threads based on memory locality, favoring local memory as much as possible to improve performance.

#### Buses

How main memory is physically connected to the system depends on the main memory architecture, as previously pictured. The actual implementation may involve additional controllers and buses between the CPUs and memory. Main memory may be accessed in one of the following ways:

* **Shared system bus**: Single or multiprocessor, via a shared system bus, a memory bridge controller, and finally a memory bus. This was pictured as the UMA example, Figure 7.3, and as the Intel front-side bus example, Figure 6.9 in Chapter 6, CPUs. The memory controller in that example was a Northbridge.
* **Direct**: Single processor with directly attached memory via a memory bus.
* **Interconnect**: Multiprocessor, each with directly attached memory via a memory bus, and processors connected via a CPU interconnect. This was pictured earlier as the NUMA example in Figure 7.4; CPU interconnects are discussed in Chapter 6, CPUs.

If you suspect your system is none of the above, find a system functional diagram and follow the data path between CPUs and memory, noting all components along the way.

#### DDR SDRAM

The speed of the memory bus, for any architecture, is often dictated by the memory interface standard supported by the processor and system board. A common standard in use since 1996 is double data rate synchronous dynamic random-access memory (DDR SDRAM). The term double data rate refers to the transfer of data on both the rise and fall of the clock signal (also called double-pumped). The term synchronous refers to the memory being clocked synchronously with the CPUs.

Example DDR SDRAM standards are shown in Table 7.1.

*<center>Table 7.1 <b>Example DDR bandwidths</b></center>*

|Standard| Specification Year |Memory Clock (MHz) | Data Rate (MT/s) | Peak Bandwidth (MB/s)
|---------|-------------|------------|------------------|--
|DDR-200  |2000|100|200|1,600|
|DDR-333  |2000|167|333|2,667|
|DDR2-667 |2003|167|667|5,333|
|DDR2-800 |2003|200|800|6,400|
|DDR3-1333|2007|167|1,333|10,667|
|DDR3-1600|2007|200|1,600|12,800|
|DDR4-3200|2012|200|3,200|25,600|
|DDR5-4800|2020|200|4,800|38,400|
|DDR5-6400|2020|200|6,400|51,200|


The DDR5 standard is expected to be released during 2020 by the JEDEC Solid State Technology Association. These standards are also named using “PC-” followed by the data transfer rate in megabytes per second, for example, PC-1600.

#### Multichannel

System architectures may support the use of multiple memory buses in parallel, to improve bandwidth. Common multiples are dual-, triple-, and quad-channel. For example, the Intel Core i7 processors support up to quad-channel DDR3-1600, for a maximum memory bandwidth of 51.2 Gbytes/s.

#### CPU Caches

Processors typically include on-chip hardware caches to improve memory access performance. The caches may include the following levels, of decreasing speed and increasing size:

* **Level 1**: Usually split into a separate instruction cache and data cache
* **Level 2**: A cache for both instructions and data
* **Level 3**: Another larger level of cache

Depending on the processor, Level 1 is typically referenced by virtual memory addresses, and Level 2 onward by physical memory addresses.

These caches were discussed in more depth in Chapter 6, CPUs. An additional type of hardware cache, the TLB, is discussed in this chapter.

#### MMU

The MMU (memory management unit) is responsible for virtual-to-physical address translations. These are performed per page, and offsets within a page are mapped directly. The MMU was introduced in Chapter 6, CPUs, in the context of nearby CPU caches.

A generic MMU is pictured in Figure 7.5, with levels of CPU caches and main memory.

![Figure 7.5 Memory management unit](./chapter-07/07-05.png)
*Figure 7.5 Memory management unit*

#### Multiple Page Sizes

Modern processors support multiple page sizes, which allow different page sizes (e.g., 4 Kbytes, 2 Mbytes, 1 Gbyte) to be used by the operating system and the MMU. The Linux huge pages feature supports larger page sizes, such as 2 Mbytes or 1 Gbyte.

#### TLB

The MMU pictured in Figure 7.5 uses a TLB (translation lookaside buffer) as the first level of address translation cache, followed by the page tables in main memory. The TLB may be divided into separate caches for instruction and data pages.

Because the TLB has a limited number of entries for mappings, the use of larger page sizes increases the range of memory that can be translated from its cache (its reach), which reduces TLB misses and improves system performance. The TLB may be further divided into separate caches for each of these page sizes, improving the probability of retaining larger mappings in cache.

As an example of TLB sizes, a typical Intel Core i7 processor provides the four TLBs shown in Table 7.2 [Intel 19a].

*Table 7.2 TLBs for a typical Intel Core i7 processor*

|Type|Page Size|Entries|
|----|---------|-------|
|Instruction|4 K|64 per thread, 128 per core|
|Instruction|large|7 per thread|
|Data|4 K|64|
|Data|large|32|

This processor has one level of data TLB. The Intel Core microarchitecture supports two levels, similar to the way CPUs provide multiple levels of main memory cache.

The exact makeup of the TLB is specific to the processor type. Refer to the vendor processor manuals for details on the TLBs in your processor and further information on their operation.

### 7.3.2 Software

Software for memory management includes the virtual memory system, address translation, swapping, paging, and allocation. The topics most related to performance are included in this section: freeing memory, the free list, page scanning, swapping, the process address space, and memory allocators.

#### Freeing Memory

When the available memory on the system becomes low, there are various methods that the kernel can use to free up memory, adding it to the free list of pages. These methods are pictured in Figure 7.6 for Linux, in the general order in which they are used as available memory decreases.

![Figure 7.6 Linux memory availability management](./chapter-07/07-06.png)
*Figure 7.6 Linux memory availability management*

These methods are:

* **Free list**: A list of pages that are unused (also called idle memory) and available for immediate allocation. This is usually implemented as multiple free page lists, one for each locality group (NUMA).
* **Page cache**: The file system cache. A tunable parameter called swappiness sets the degree to which the system should favor freeing memory from the page cache instead of swapping.
* **Swapping**: This is paging by the page-out daemon, kswapd, which finds not recently used pages to add to the free list, including application memory. These are paged out, which may involve writing to either a file system-based swap file or a swap device. Naturally, this is available only if a swap file or device has been configured.
* **Reaping**: When a low-memory threshold is crossed, kernel modules and the kernel slab allocator can be instructed to immediately free any memory that can easily be freed. This is also known as shrinking.
* **OOM killer**: The out-of-memory killer will free memory by finding and killing a sacrificial process, found using select_bad_process() and then killed by calling oom_kill_process(). This may be logged in the system log (/var/log/messages) as an “Out of memory: Kill process” message.

The Linux swappiness parameter controls whether to favor freeing memory by paging applications or by reclaiming it from the page cache. It is a number between 0 and 100 (the default value is 60), where higher values favor freeing memory by paging. Controlling the balance between these memory freeing techniques allows system throughput to be improved by preserving warm file system cache while paging out cold application memory [Corbet 04].

It is also interesting to ask what happens if no swap device or swap file is configured. This limits virtual memory size, so if overcommit has been disabled, memory allocations will fail sooner. On Linux, this may also mean that the OOM killer is used sooner.

Consider an application with an issue of endless memory growth. With swap, this is likely to first become a performance issue due to paging, which is an opportunity to debug the issue live. Without swap, there is no paging grace period, so either the application hits an “Out of memory” error or the OOM killer terminates it. This may delay debugging the issue if it is seen only after hours of usage.

In the Netflix cloud, instances typically do not use swap, so applications are OOM killed if they exhaust memory. Applications are distributed across a large pool of instances, and having one OOM killed causes traffic to be immediately redirected to other healthy instances. This is considered preferable to allowing one instance to run slowly due to swapping.

When memory cgroups are used, similar memory freeing techniques can be used as those shown in Figure 7.6 to manage cgroup memory. A system may have an abundance of free memory, but is swapping or encountering the OOM killer because a container has exhausted its cgroup-controlled limit [Evans 17]. For more on cgroups and containers, see Chapter 11, Cloud Computing.

The following sections describe free lists, reaping, and the page-out daemon.

#### Free List(s)

The original Unix memory allocator used a memory map and a first-fit scan. With the introduction of paged virtual memory in BSD, a free list and a page-out daemon were added [Babaoglu 79]. The free list, pictured in Figure 7.7, allows available memory to be located immediately.

![Figure 7.7 Free list operations](./chapter-07/07-07.png)
*Figure 7.7 Free list operations*

Memory freed is added to the head of the list for future allocations. Memory that is freed by the page-out daemon—and that may still contain useful cached file system pages—is added to the tail. Should a future request for one of these pages occur before the useful page has been reused, it can be reclaimed and removed from the free list.

A form of free list is still in use by Linux-based systems, as pictured in Figure 7.6. Free lists are typically consumed via allocators, such as the slab allocator for the kernel, and libc malloc() for user-space (which has its own free lists). These in turn consume pages and then expose them via their allocator API.

Linux uses the buddy allocator for managing pages. This provides multiple free lists for different- sized memory allocations, following a power-of-two scheme. The term buddy refers to finding neighboring pages of free memory so that they can be allocated together. For historical background, see [Peterson 77].

The buddy free lists are at the bottom of the following hierarchy, beginning with the per-memory node pg_data_t:

* Nodes: Banks of memory, NUMA-aware
* Zones: Ranges of memory for certain purposes (direct memory access [DMA],4 normal, highmem)
>> <sup>4</sup>Although ZONE_DMA may be removed [Corbet 18a].

* Migration types: Unmovable, reclaimable, movable, etc.
* Sizes: Power-of-two number of pages

Allocating within the node free lists improves memory locality and performance. For the most common allocation, single pages, the buddy allocator keeps lists of single pages for each CPU to reduce CPU lock contention.

#### Reaping

Reaping mostly involves freeing memory from the kernel slab allocator caches. These caches contain unused memory in slab-size chunks, ready for reuse. Reaping returns this memory to the system for page allocations.

On Linux, kernel modules can also call register_shrinker() to register specific functions for reaping their own memory.

#### Page Scanning

Freeing memory by paging is managed by the kernel page-out daemon. When available main memory in the free list drops below a threshold, the page-out daemon begins page scanning. Page scanning occurs only when needed. A normally balanced system may not page scan very often and may do so only in short bursts.

On Linux, the page-out daemon is called kswapd, which scans LRU page lists of inactive and active memory to free pages. It is woken up based on free memory and two thresholds to provide hysteresis, as shown in Figure 7.8.

![Figure 7.8 kswapd wake-ups and modes](./chapter-07/07-08.png)
*Figure 7.8 kswapd wake-ups and modes*

Once free memory has reached the lowest threshold, kswapd runs in the foreground, synchronously freeing pages of memory as they are requested, a method sometimes known as direct-reclaim [Gorman 04]. This lowest threshold is tunable (vm.min_free_kbytes), and the others are scaled based on it (by 2x for low, 3x for high). For workloads with high allocation bursts that outpace kswap reclamation, Linux provides additional tunables for more aggressive scanning, vm.watermark_scale_factor and vm.watermark_boost_factor: see Section 7.6.1, Tunable Parameters.

The page cache has separate lists for inactive pages and active pages. These operate in an LRU fashion, allowing kswapd to find free pages quickly. They are shown in Figure 7.9.

![Figure 7.9 kswapd lists](./chapter-07/07-09.png)
*Figure 7.9 kswapd lists*

kswapd scans the inactive list first, and then the active list, if needed. The term scanning refers to checking of pages as the list is walked: a page may be ineligible to be freed if it is locked/dirty. The term scanning as used by kswapd has a different meaning than the scanning done by the original UNIX page-out daemon, which scans all of memory.

### 7.3.3 Process Virtual Address Space

Managed by both hardware and software, the process virtual address space is a range of virtual pages that are mapped to physical pages as needed. The addresses are split into areas called segments for storing the thread stacks, process executable, libraries, and heap. Examples for 32-bit processes on Linux are shown in Figure 7.10, for both x86 and SPARC processors.

![Figure 7.10 Example process virtual memory address space](./chapter-07/07-10.png)
*Figure 7.10 Example process virtual memory address space*

On SPARC the kernel resides in a separate full address space (which is not shown in Figure 7.10). Note that on SPARC it is not possible to distinguish between a user and kernel address based only on the pointer value; x86 employs a different scheme where the user and kernel addresses are non-overlapping.<sup>5</sup>

> <sup>5</sup>Note that for 64-bit addresses, the full 64-bit range may not be supported by the processor: the AMD specification allows implementations to only support 48-bit addresses, where the unused higher-order bits are set to the last bit: this creates two usable address ranges, called canonical address, of 0 to `0x00007fffffffffff`, used for user space, and `0xffff800000000000` to `0xffffffffffffffff`, used for kernel space. This is why x86 kernel addresses begin with `0xffff`.

The program executable segment contains separate text and data segments. Libraries are also composed of separate executable text and data segments. These different segment types are:

* **Executable text**: Contains the executable CPU instructions for the process. This is mapped from the text segment of the binary program on the file system. It is read-only with the execute permission.
* **Executable data**: Contains initialized variables mapped from the data segment of the binary program. This has read/write permissions so that the variables can be modified while the program is running. It also has a private flag so that modifications are not flushed to disk.
* **Heap**: This is the working memory for the program and is anonymous memory (no file system location). It grows as needed and is allocated via malloc(3).
* **Stack**: Stacks of the running threads, mapped read/write.

The library text segments may be shared by other processes that use the same library, each of which has a private copy of the library data segment.

#### Heap Growth

A common source of confusion is the endless growth of heap. Is it a memory leak? For simple allocators, a free(3) does not return memory to the operating system; rather, memory is kept to serve future allocations. This means that the process resident memory can only grow, which is normal. Methods for processes to reduce system memory use include:

* **Re-exec**: Calling execve(2) to begin from an empty address space
* **Memory mapping**: Using mmap(2) and munmap(2), which will return memory to the system

Memory-mapped files are described in Chapter 8, File Systems, Section 8.3.10, Memory-Mapped Files.

Glibc, commonly used on Linux, is an advanced allocator that supports an mmap mode of operation, as well as a malloc_trim(3) function to release free memory to the system. malloc_trim(3) is automatically called by free(3) when the top-of-heap free memory becomes large,<sup>6</sup> and frees it using sbrk(2) syscalls.

> <sup>6</sup>Larger than the `M_TRIM_THRESHOLD` `mallopt(3)` parameter, which is 128 Kbytes by default.

#### Allocators

There are a variety of user- and kernel-level allocators for memory allocation. Figure 7.11 shows the role of allocators, including some common types.

![Figure 7.11 User- and kernel-level memory allocators](./chapter-07/07-11.png)
*Figure 7.11 User- and kernel-level memory allocators*

Page management was described earlier in Section 7.3.2, Software, under Free List(s).

Memory allocator features can include:

* **Simple API**: For example, malloc(3), free(3).
* **Efficient memory usage**: When servicing memory allocations of a variety of sizes, memory usage can become fragmented, where there are many unused regions that waste memory. Allocators can strive to coalesce the unused regions, so that larger allocations can make use of them, improving efficiency.
* **Performance**: Memory allocations can be frequent, and on multithreaded environments they can perform poorly due to contention for synchronization primitives. Allocators can be designed to use locks sparingly, and can also make use of per-thread or per-CPU caches to improve memory locality.
* **Observability**: An allocator may provide statistics and debug modes to show how it is being used, and which code paths are responsible for allocations.

The sections that follow describe kernel-level allocators—slab and SLUB—and user-level allocators—glibc, TCMalloc, and jemalloc.

#### Slab

The kernel slab allocator manages caches of objects of a specific size, allowing them to be recycled quickly without the overhead of page allocation. This is especially effective for kernel allocations, which are frequently for fixed-size structs.

As a kernel example, the following two lines are from ZFS `arc.c`<sup>7</sup>:

> <sup>7</sup>The only reason these came to mind as examples is because I developed the code.

```c
    df = kmem_alloc(sizeof (l2arc_data_free_t), KM_SLEEP);
    head = kmem_cache_alloc(hdr_cache, KM_PUSHPAGE);
```

The first, kmem_alloc(), shows a traditional-style kernel allocation whose size is passed as an argument. The kernel maps this to a slab cache based on that size (very large sizes are handled differently, by an oversize arena). The second, kmem_cache_alloc(), operates directly on a custom slab allocator cache, in this case (kmem_cache_t *)hdr_cache.

Developed for Solaris 2.4 [Bonwick 94], the slab allocator was later enhanced with per-CPU caches called magazines [Bonwick 01]:

Our basic approach is to give each CPU an M-element cache of objects called a magazine, by analogy with automatic weapons. Each CPU’s magazine can satisfy M allocations before the CPU needs to reload—that is, exchange its empty magazine for a full one.

Apart from high performance, the original slab allocator featured debug and analysis facilities including auditing to trace allocation details and stack traces.

Slab allocation has been adopted by various operating systems. BSD has a kernel slab allocator called the universal memory allocator (UMA), which is efficient and NUMA-aware. A slab allocator was also introduced to Linux in version 2.2, where it was the default option for many years. Linux has since moved to SLUB as an option or as the default.

#### SLUB

The Linux kernel SLUB allocator is based on the slab allocator and is designed to address various concerns, especially regarding the complexity of the slab allocator. Improvements include the removal of object queues, and per-CPU caches—leaving NUMA optimization to the page allocator (see the earlier Free List(s) section).

The SLUB allocator was made the default option in Linux 2.6.23 [Lameter 07].

#### glibc

The user-level GNU libc allocator is based on dlmalloc by Doug Lea. Its behavior depends on the allocation request size. Small allocations are served from bins of memory, containing units of a similar size, which can be coalesced using a buddy-like algorithm. Larger allocations can use a tree lookup to find space efficiently. Very large allocations switch to using mmap(2). The net result is a high-performing allocator that benefits from multiple allocation policies.

#### TCMalloc

TCMalloc is the user-level thread caching malloc, which uses a per-thread cache for small allocations, reducing lock contention and improving performance [Ghemawat 07]. Periodic garbage collection migrates memory back to a central heap for allocations.

#### jemalloc

Originating as the FreeBSD user-level libc allocator, libjemalloc is also available for Linux. It uses techniques such as multiple arenas, per-thread caching, and small object slabs to improve scalability and reduce memory fragmentation. It can use both mmap(2) and sbrk(2) to obtain system memory, preferring mmap(2). Facebook use jemalloc and have added profiling and other optimizations [Facebook 11].