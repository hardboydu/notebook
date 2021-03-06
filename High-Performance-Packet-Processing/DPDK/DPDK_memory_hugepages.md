# DPDK Memory hugepages

## Enable hugepages

```sh
#!/bin/bash
mkdir -p /mnt/huge
(mount | grep /mnt/huge) > /dev/null || mount -t hugetlbfs hugetlbfs /mnt/huge
for i in {0..7}
do
        if [[ -e "/sys/devices/system/node/node$i" ]]
        then
                echo 512 > /sys/devices/system/node/node$i/hugepages/hugepages-2048kB/nr_hugepages
        fi
done
```

## EAL initialization

The allocation of large contiguous physical memory is done using the hugetlbfs kernel filesystem. The EAL provides an API to reserve named memory zones in this contiguous memory. The physical address of the reserved memory for that memory zone is also returned to the user by the memory zone reservation API.

使用 `hugetlbfs` 内核文件系统完成巨大连续物理内存的分配。EAL 提供了一个 API 来保留此连续内存中的命名内存区域。该存储区的保留存储器的物理地址也由存储区保留 API 返回给用户。

There are two modes in which DPDK memory subsystem can operate: dynamic mode, and legacy mode. Both modes are explained below.

DPDK 内存子系统可以使用两种模式：动态模式和传统模式。两种模式都在下面解释。

> Memory reservations done using the APIs provided by rte_malloc are also backed by pages from the hugetlbfs filesystem.
> 使用rte_malloc提供的API完成的内存预留也由hugetlbfs文件系统的页面支持。

### Dynamic memory mode

Currently, this mode is only supported on Linux.

目前，此模式仅能在Linux上使用。

In this mode, usage of hugepages by DPDK application will grow and shrink based on application’s requests. Any memory allocation through `rte_malloc()`, `rte_memzone_reserve()` or other methods, can potentially result in more hugepages being reserved from the system. Similarly, any memory deallocation can potentially result in hugepages being released back to the system.

在这种模式下，DPDK应用程序对大页面的使用将根据应用程序的请求增长和缩小。通过 `rte_malloc()`，`rte_memzone_reserve()` 或其他方法进行的任何内存分配都可能导致从系统中保留更多大页面。同样，任何内存释放都可能导致大页面被释放回系统。

Memory allocated in this mode is not guaranteed to be IOVA-contiguous. If large chunks of IOVA-contiguous are required (with "large" defined as "more than one page"), it is recommended to either use VFIO driver for all physical devices (so that IOVA and VA addresses can be the same, thereby bypassing physical addresses entirely), or use legacy memory mode.

在此模式下分配的内存不保证是**IOVA-连续**的。如果需要大块**IOVA-连续**（“大” 定义为 “多页”），建议对所有物理设备使用 VFIO 驱动程序（以便 IOVA 和 VA 地址可以相同，从而绕过物理地址完全），或使用传统内存模式。

For chunks of memory which must be IOVA-contiguous, it is recommended to use `rte_memzone_reserve()` function with `RTE_MEMZONE_IOVA_CONTIG` flag specified. This way, memory allocator will ensure that, whatever memory mode is in use, either reserved memory will satisfy the requirements, or the allocation will fail.

对于必须是**IOVA-连续**的内存块，建议使用 `rte_memzone_reserve()`函数并指定 `RTE_MEMZONE_IOVA_CONTIG` 标志。这样，内存分配器将确保无论使用何种内存模式，预留内存都将满足要求，否则分配将失败。

There is no need to preallocate any memory at startup using `-m` or `--socket-mem` command-line parameters, however it is still possible to do so, in which case preallocate memory will be “pinned” (i.e. will never be released by the application back to the system). It will be possible to allocate more hugepages, and deallocate those, but any preallocated pages will not be freed. If neither -m nor --socket-mem were specified, no memory will be preallocated, and all memory will be allocated at runtime, as needed.

使用 `-m` 或 `--socket-mem` 命令行参数时无需在启动时预分配任何内存，但是仍然可以这样做，在这种情况下，preallocate 内存将被“固定”（即永远不会被释放）应用程序返回系统）。可以分配更多大页面并释放它们，但不会释放任何预分配的页面。如果既未指定 `-m `也未指定 `--socket-mem`，则不会预分配任何内存，并且将根据需要在运行时分配所有内存。

Another available option to use in dynamic memory mode is `--single-file-segments` command-line option. This option will put pages in single files (per memseg list), as opposed to creating a file per page. This is normally not needed, but can be useful for use cases like userspace vhost, where there is limited number of page file descriptors that can be passed to VirtIO.

在动态内存模式下使用的另一个可用选项是 `--single-file-segments` 命令行选项。此选项将页面放在单个文件（每个memseg列表）中，而不是每页创建一个文件。这通常是不需要的，但对于用户空间vhost这样的用例非常有用，其中可以传递给VirtIO的页面文件描述符数量有限。

If the application (or DPDK-internal code, such as device drivers) wishes to receive notifications about newly allocated memory, it is possible to register for memory event callbacks via `rte_mem_event_callback_register()` function. This will call a callback function any time DPDK’s memory map has changed.

如果应用程序（或DPDK内部代码，例如设备驱动程序）希望接收有关新分配的内存的通知，则可以通过 `rte_mem_event_callback_register()` 函数注册内存事件回调。这将在DPDK的内存映射发生变化时调用回调函数。

If the application (or DPDK-internal code, such as device drivers) wishes to be notified about memory allocations above specified threshold (and have a chance to deny them), allocation validator callbacks are also available via rte_mem_alloc_validator_callback_register() function.

如果应用程序（或DPDK内部代码，例如设备驱动程序）希望收到有关高于指定阈值的内存分配的通知（并且有机会拒绝它们），则还可以通过`rte_mem_alloc_validator_callback_register()` 函数获得分配验证器回调。

A default validator callback is provided by EAL, which can be enabled with a --socket-limit command-line option, for a simple way to limit maximum amount of memory that can be used by DPDK application.

EAL提供了一个默认的验证器回调，它可以使用 `--socket-limit` 命令行选项启用，这是一种限制 DPDK 应用程序可以使用的最大内存量的简单方法。

### Legacy memory mode

This mode is enabled by specifying `--legacy-mem` command-line switch to the EAL. This switch will have no effect on FreeBSD as FreeBSD only supports legacy mode anyway.

通过指定 `--legacy-mem` 命令行切换到EAL来启用此模式。这个开关对FreeBSD没有任何影响，因为FreeBSD只支持传统模式。

This mode mimics historical behavior of EAL. That is, EAL will reserve all memory at startup, sort all memory into large IOVA-contiguous chunks, and will not allow acquiring or releasing hugepages from the system at runtime.

此模式模仿 EAL 的历史行为。也就是说，EAL将在启动时保留所有内存，将所有内存排序为大型 IOVA-连续块，并且不允许在运行时从系统中获取或释放大页面。

If neither `-m` nor `--socket-mem` were specified, the entire available hugepage memory will be preallocated.

如果既未指定 `-m` 也未指定 `--socket-mem`，则将预先分配整个可用的hugepage内存。

### Hugepage allocation matching

This behavior is enabled by specifying the `--match-allocations` command-line switch to the EAL. This switch is Linux-only and not supported with `--legacy-mem` nor `--no-huge`.

通过指定 `-match-allocations` 命令行切换到 EAL 来启用此行为。此开关仅限 Linux，不支持 `--legacy-mem` 和 `--no-huge`。

Some applications using memory event callbacks may require that hugepages be freed exactly as they were allocated. These applications may also require that any allocation from the malloc heap not span across allocations associated with two different memory event callbacks. Hugepage allocation matching can be used by these types of applications to satisfy both of these requirements. This can result in some increased memory usage which is very dependent on the memory allocation patterns of the application.

某些使用内存事件回调的应用程序可能需要在分配时释放大页面。这些应用程序还可能要求来自malloc堆的任何分配不跨越与两个不同的内存事件回调相关联的分配。这些类型的应用程序可以使用Hugepage分配匹配来满足这两个要求。这可能导致一些增加的内存使用量，这非常依赖于应用程序的内存分配模式。

### Maximum amount of memory

All possible virtual memory space that can ever be used for hugepage mapping in a DPDK process is preallocated at startup, thereby placing an upper limit on how much memory a DPDK application can have. DPDK memory is stored in segment lists, each segment is strictly one physical page. It is possible to change the amount of virtual memory being preallocated at startup by editing the following config variables:

在 DPDK 进程中可以用于大页面映射的所有可能的虚拟内存空间在启动时预先分配，从而对DPDK应用程序可以具有多少内存设置上限。DPDK存储器存储在段列表中，每个段严格地是一个物理页面。通过编辑以下配置变量，可以更改启动时预分配的虚拟内存量：

* `CONFIG_RTE_MAX_MEMSEG_LISTS` controls how many segment lists can DPDK have
* `CONFIG_RTE_MAX_MEM_MB_PER_LIST` controls how much megabytes of memory each segment list can address
* `CONFIG_RTE_MAX_MEMSEG_PER_LIST` controls how many segments each segment can have
* `CONFIG_RTE_MAX_MEMSEG_PER_TYPE` controls how many segments each memory type can have (where “type” is defined as “page size NUMA node” combination)
* `CONFIG_RTE_MAX_MEM_MB_PER_TYPE` controls how much megabytes of memory each memory type can address
* `CONFIG_RTE_MAX_MEM_MB` places a global maximum on the amount of memory DPDK can reserve

Normally, these options do not need to be changed.

通常，不需要更改这些选项。

>Preallocated virtual memory is not to be confused with preallocated hugepage memory! All DPDK processes preallocate virtual memory at startup. Hugepages can later be mapped into that preallocated VA space (if dynamic memory mode is enabled), and can optionally be mapped into it at startup.
预分配的虚拟内存不要与预先分配的大页面内存混淆！所有DPDK进程在启动时预分配虚拟内存。以后可以将Hugepages映射到预分配的VA空间（如果启用了动态内存模式），并且可以选择在启动时映射到它。

>*参考 [Programmer’s Guide : 3.1.4. Memory Mapping Discovery and Memory Reservation](https://doc.dpdk.org/guides/prog_guide/env_abstraction_layer.html)*

## Multi-process Support

On application start-up in a primary or standalone process, the DPDK records to memory-mapped files the details of the memory configuration it is using - hugepages in use, the virtual addresses they are mapped at, the number of memory channels present, etc. When a secondary process is started, these files are read and the EAL recreates the same memory configuration in the secondary process so that all memory zones are shared between processes and all pointers to that memory are valid, and point to the same objects, in both processes.

在主应用程序或独立进程中启动应用程序时，DPDK会将内存配置的详细信息记录到内存映射文件中 - 正在使用的大页面，它们映射的虚拟地址，存在的内存通道数等等。当启动辅助进程时，将读取这些文件，并且EAL在辅助进程中重新创建相同的内存配置，以便所有内存区域在进程之间共享，并且指向该内存的所有指针都有效，并指向相同的对象，这两个进程。

>Refer to Multi-process Limitations for details of how Linux kernel Address-Space Layout Randomization (ASLR) can affect memory sharing.
>有关Linux内核地址空间布局随机化（ASLR）如何影响内存共享的详细信息，请参阅多进程限制。
>
>If the primary process was run with --legacy-mem or --single-file-segments switch, secondary processes must be run with the same switch specified. Otherwise, memory corruption may occur.
>如果主进程使用--legacy-mem或--single-file-segments开关运行，则必须使用指定的同一开关运行辅助进程。否则，可能会发生内存损坏。

An alternative deployment model that can be used for multi-process applications is to have a single primary process instance that acts as a load-balancer or server distributing received packets among worker or client threads, which are run as secondary processes. In this case, extensive use of rte_ring objects is made, which are located in shared hugepage memory.

可用于多进程应用程序的备用部署模型是具有单个主要进程实例，该实例充当负载平衡器或服务器，在工作线程或客户端线程之间分发接收的数据包，这些线程作为辅助进程运行。在这种情况下，广泛使用rte_ring对象，它们位于共享的大页面内存中。

By default, the EAL creates hugepage files on each hugetlbfs filesystem using the rtemap_X filename, where X is in the range 0 to the maximum number of hugepages -1. Similarly, it creates shared configuration files, memory mapped in each process, using the /var/run/.rte_config filename, when run as root (or $HOME/.rte_config when run as a non-root user; if filesystem and device permissions are set up to allow this). The rte part of the filenames of each of the above is configurable using the file-prefix parameter.

默认情况下，EAL使用rtemap_X文件名在每个hugetlbfs文件系统上创建巨大的页面文件，其中X的范围是 0 到最大的largepages - 1。类似地，它以root身份运行时创建共享配置文件，每个进程映射的内存，使用/var/run/.rte_config文件名（或者作为非root用户运行时为 $HOME/.rte_config; 如果是文件系统和设备权限设置为允许此项）。上述每个文件名的rte部分可使用file-prefix参数进行配置。

In addition to specifying the file-prefix parameter, any DPDK applications that are to be run side-by-side must explicitly limit their memory use. This is less of a problem on Linux, as by default, applications will not allocate more memory than they need. However if --legacy-mem is used, DPDK will attempt to preallocate all memory it can get to, and memory use must be explicitly limited. This is done by passing the -m flag to each process to specify how much hugepage memory, in megabytes, each process can use (or passing --socket-mem to specify how much hugepage memory on each socket each process can use).

除了指定file-prefix参数之外，要并行运行的任何DPDK应用程序都必须明确限制其内存使用。这在Linux上不是问题，因为默认情况下，应用程序不会分配超出需要的内存。但是，如果使用--legacy-mem，DPDK将尝试预分配它可以获得的所有内存，并且必须明确限制内存使用。这是通过将-m标志传递给每个进程来完成的，以指定每个进程可以使用多少巨大页面内存（以兆字节为单位）（或者传递--socket-mem来指定每个进程可以使用的每个套接字上的巨大页面内存量）。

The multi-process feature requires that the exact same hugepage memory mappings be present in all applications. This makes secondary process startup process generally unreliable. Disabling Linux security feature - Address-Space Layout Randomization (ASLR) may help getting more consistent mappings, but not necessarily more reliable - if the mappings are wrong, they will be consistently wrong!

多进程功能要求在所有应用程序中都存在完全相同的大页面内存映射。这使得二次过程启动过程通常不可靠。禁用Linux安全功能 - 地址空间布局随机化（ASLR）可能有助于获得更一致的映射，但不一定更可靠 - 如果映射错误，它们将始终是错误的！

>*参考 [Programmer’s Guide : 32. Multi-process Support](https://doc.dpdk.org/guides/prog_guide/multi_proc_support.html)*
