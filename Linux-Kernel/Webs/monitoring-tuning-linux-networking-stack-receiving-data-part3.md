# [Monitoring and Tuning the Linux Networking Stack: Receiving Data Part3 Network Device Initialization](https://blog.packagecloud.io/eng/2016/06/22/monitoring-tuning-linux-networking-stack-receiving-data/)

## Network device initialization

The `igb_probe` function does some important network device initialization. In addition to the PCI specific work, it will do more general networking and network device work:

`igb_probe` 函数执行一些重要的网络设备初始化。除了 PCI 特定的工作，它还将做更多的一般网络和网络设备工作：

1. The `struct net_device_ops` is registered. <br> `struct net_device_ops` 已注册。
2. `ethtool` operations are registered. <br> 注册 `ethtool` 操作接口。
3. The default MAC address is obtained from the NIC. <br> 从 NIC 获取默认 MAC 地址。
4. net_device feature flags are set. <br> 设置 `net_device` 功能标志。
5. And lots more. <br> 还有更多。

Let’s take a look at each of these as they will be interesting later.

让我们来看看这些中的每一个，因为它们稍后会很有趣。

### `struct net_device_ops`

The `struct net_device_ops` contains function pointers to lots of important operations that the network subsystem needs to control the device. We’ll be mentioning this structure many times throughout the rest of this post.

`struct net_device_ops` 包含指向网络子系统控制设备所需的许多重要操作的函数指针。在本文的其余部分我们将多次提到这种结构。

This `net_device_ops` structure is attached to a `struct net_device` in `igb_probe`. From [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L2090) :

此 `net_device_ops` 结构附加到 `igb_probe` 中的 `struct net_device` 。参考 [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L2090) ：

```c
static int igb_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    /*...*/

    netdev->netdev_ops = &igb_netdev_ops;

    /*...*/
}
```

And the functions that this `net_device_ops` structure holds pointers to are set in the same file. From [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L1905-L1913) :

并且此 `net_device_ops` 结构保存指针的函数在同一文件中设置。来自 [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L1905-L1913) ：

```c
static const struct net_device_ops igb_netdev_ops = {
  .ndo_open               = igb_open,
  .ndo_stop               = igb_close,
  .ndo_start_xmit         = igb_xmit_frame,
  .ndo_get_stats64        = igb_get_stats64,
  .ndo_set_rx_mode        = igb_set_rx_mode,
  .ndo_set_mac_address    = igb_set_mac,
  .ndo_change_mtu         = igb_change_mtu,
  .ndo_do_ioctl           = igb_ioctl,

  /* ... */
```

As you can see, there are several interesting fields in this struct like `ndo_open`, `ndo_stop`, `ndo_start_xmit`, and `ndo_get_stats64` which hold the addresses of functions implemented by the `igb` driver.

如您所见，此结构中有几个有趣的字段，如 `ndo_open`， `ndo_stop`， `ndo_start_xmit` 和 `ndo_get_stats64`， 它们包含 `igb` 驱动程序实现的函数的地址。

We’ll be looking at some of these in more detail later.

稍后我们将更详细地介绍其中的一些内容。

### `ethtool` registration

[`ethtool`](https://www.kernel.org/pub/software/network/ethtool/) is a command line program you can use to get and set various driver and hardware options. You can install it on Ubuntu by running `apt-get install ethtool`.

[`ethtool`](https://www.kernel.org/pub/software/network/ethtool/) 是一个命令行程序，可用于获取和设置各种驱动程序和硬件选项。您可以通过运行 `apt-get install ethtool` 在Ubuntu上安装它。

A common use of ethtool is to gather detailed statistics from network devices. Other ethtool settings of interest will be described later.

`ethtool` 的一个常见用途是从网络设备收集详细的统计信息。其他感兴趣的 `ethtool` 设置将在后面描述。

The `ethtool` program talks to device drivers by using the [`ioctl`](http://man7.org/linux/man-pages/man2/ioctl.2.html) system call. The device drivers register a series of functions that run for the `ethtool` operations and the kernel provides the glue.

`ethtool` 程序通过使用 `ioctl` 系统调用与设备驱动程序进行通信。设备驱动程序注册了一系列为`ethtool` 操作运行的函数，内核提供了粘合层。

When an `ioctl` call is made from `ethtool`, the kernel finds the `ethtool` structure registered by the appropriate driver and executes the functions registered. The driver’s `ethtool` function implementation can do anything from change a simple software flag in the driver to adjusting how the actual NIC hardware works by writing register values to the device.

当从 `ethtool` 进行 `ioctl` 调用时，内核会找到由相应驱动程序注册的 `ethtool` 结构并执行已注册的函数。驱动程序的 `ethtool` 函数实现可以做任何事情，从更改驱动程序中的简单软件标志到通过将寄存器值写入设备来调整实际 NIC 硬件的工作方式。

The `igb` driver registers its `ethtool` operations in `igb_probe` by calling `igb_set_ethtool_ops` :

`igb` 驱动程序通过调用 `igb_set_ethtool_ops` 在 `igb_probe` 中注册其 `ethtool` 操作：

```c
static int igb_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    /* ... */

    igb_set_ethtool_ops(netdev);

    /* ... */
}
```

All of the `igb` driver’s `ethtool` code can be found in the file [`drivers/net/ethernet/intel/igb/igb_ethtool.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_ethtool.c) along with the `igb_set_ethtool_ops` function.

所有 `igb` 驱动程序的 `ethtool` 代码都可以在文件 [`drivers/net/ethernet/intel/igb/igb_ethtool.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_ethtool.c) 中找到，同时还有 `igb_set_ethtool_ops` 函数。

From [`drivers/net/ethernet/intel/igb/igb_ethtool.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_ethtool.c#L3012-L3015) :

参考 [`drivers/net/ethernet/intel/igb/igb_ethtool.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_ethtool.c#L3012-L3015) :

```c
void igb_set_ethtool_ops(struct net_device *netdev)
{
    netdev->ethtool_ops = &igb_ethtool_ops;
}
```

Above that, you can find the igb_ethtool_ops structure with the ethtool functions the igb driver supports set to the appropriate fields.

在此之上，您可以找到具有 `igb` 驱动程序支持的 `ethtool` 函数的 `igb_ethtool_ops` 结构设置到适当的字段。

From [`drivers/net/ethernet/intel/igb/igb_ethtool.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_ethtool.c#L2970-L2979) :

来自 [`drivers/net/ethernet/intel/igb/igb_ethtool.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_ethtool.c#L2970-L2979) :

```c
static const struct ethtool_ops igb_ethtool_ops = {
  .get_settings           = igb_get_settings,
  .set_settings           = igb_set_settings,
  .get_drvinfo            = igb_get_drvinfo,
  .get_regs_len           = igb_get_regs_len,
  .get_regs               = igb_get_regs,
  /* ... */
```

It is up to the individual drivers to determine which `ethtool` functions are relevant and which should be implemented. Not all drivers implement all `ethtool` functions, unfortunately.

由各个驱动程序决定哪些 `ethtool` 功能是相关的以及哪些应该实现。不幸的是，并非所有驱动程序都实现了所有 `ethtool` 功能。

One interesting `ethtool` function is `get_ethtool_stats`, which (if implemented) produces detailed statistics counters that are tracked either in software in the driver or via the device itself.

一个有趣的 `ethtool` 函数是 `get_ethtool_stats` ， 它（如果实现的话）生成详细的统计计数器，这些计数器可以通过驱动程序中的软件或通过设备本身进行跟踪。

The monitoring section below will show how to use ethtool to access these detailed statistics.

下面的监控部分将说明如何使用 `ethtool` 访问这些详细的统计信息。

### IRQs

When a data frame is written to RAM via DMA, how does the NIC tell the rest of the system that data is ready to be processed?

当数据帧通过 DMA 写入 RAM 时，NIC 如何告诉系统的其余部分数据是否已准备好处理？

Traditionally, a NIC would generate an [interrupt request (IRQ)](https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture)) indicating data had arrived. There are three common types of IRQs: MSI-X, MSI, and legacy IRQs. These will be touched upon shortly. A device generating an IRQ when data has been written to RAM via DMA is simple enough, but if large numbers of data frames arrive this can lead to a large number of IRQs being generated. The more IRQs that are generated, the less CPU time is available for higher level tasks like user processes.

传统上，NIC 会生成一个[中断请求（IRQ）](https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture))，指示数据已到达。 IRQ有三种常见类型：MSI-X，MSI和传统IRQ。这些将很快被触及。当数据通过 DMA 写入 RAM 时产生 IRQ 的设备很简单，但如果大量数据帧到达，则会导致生成大量 IRQ 。生成的 IRQ 越多，用户进程等更高级别任务的 CPU 时间就越少。

The [New Api (NAPI)](http://www.linuxfoundation.org/collaborate/workgroups/networking/napi) was created as a mechanism for reducing the number of IRQs generated by network devices on packet arrival. While NAPI reduces the number of IRQs, it cannot eliminate them completely.

[New Api (NAPI)](http://www.linuxfoundation.org/collaborate/workgroups/networking/napi)被创建为一种机制，用于减少网络设备在数据包到达时生成的 IRQ 数量。虽然 NAPI 减少了 IRQ 的数量，但它无法完全消除它们。

We’ll see why that is, exactly, in later sections.

我们将在后面的章节中详细说明原因。

### NAPI

[NAPI](http://www.linuxfoundation.org/collaborate/workgroups/networking/napi) differs from the legacy method of harvesting data in several important ways. NAPI allows a device driver to register a `poll` function that the NAPI subsystem will call to harvest data frames.

[NAPI](http://www.linuxfoundation.org/collaborate/workgroups/networking/napi) 与传统的以几种重要方式收集数据的方法不同。 NAPI 允许设备驱动程序注册 NAPI 子系统将调用以收集数据帧的轮询（`poll`）功能。

The intended use of NAPI in network device drivers is as follows:

NAPI 在网络设备驱动程序中的预期用途如下：

1. NAPI is enabled by the driver, but is in the off position initially. <br> NAPI由驱动程序启用，但最初处于关闭位置。
2. A packet arrives and is DMA’d to memory by the NIC. <br> 数据包到达并由 NIC 进行内存 DMA 。
3. An IRQ is generated by the NIC which triggers the IRQ handler in the driver. <br> 由 NIC 生成 IRQ，触发驱动程序中的 IRQ 处理程序。
4. The driver wakes up the NAPI subsystem using a softirq (more on these later). This will begin harvesting packets by calling the driver’s registered poll function in a separate thread of execution. <br> 驱动程序使用 `softirq` 唤醒 NAPI 子系统（稍后将详细介绍）。这将通过在单独的执行线程中调用驱动程序的已注册轮询函数来开始收集数据包。
5. The driver should disable further IRQs from the NIC. This is done to allow the NAPI subsystem to process packets without interruption from the device. <br> 驱动程序应禁用 NIC 中的其他 IRQ 。这样做是为了允许 NAPI 子系统在不中断设备的情况下处理数据包。
6. Once there is no more work to do, the NAPI subsystem is disabled and IRQs from the device are re-enabled. <br> 一旦没有其他工作要做，就会禁用 NAPI 子系统，并重新启用设备的 IRQ。
7. The process starts back at step 2. <br> 该过程从第2步开始。

This method of gathering data frames has reduced overhead compared to the legacy method because many data frames can be consumed at a time without having to deal with processing each of them one IRQ at a time.

与传统方法相比，这种收集数据帧的方法减少了开销，因为一次可以消耗许多数据帧，而不必一次处理每个 IRQ 处理它们中的每一个。

The device driver implements a `poll` function and registers it with NAPI by calling `netif_napi_add`. When registering a NAPI `poll` function with `netif_napi_add`, the driver will also specify the `weight`. Most of the drivers hardcode a value of `64`. This value and its meaning will be described in more detail below.

设备驱动程序实现 `poll` 函数，并通过调用 `netif_napi_add` 将其注册到NAPI。使用`netif_napi_add` 注册 NAPI 轮询功能时，驱动程序还将指定权重。大多数驱动程序硬编码 `64` 的值。该值及其含义将在下面更详细地描述。

Typically, drivers register their NAPI poll functions during driver initialization.

通常，驱动程序在驱动程序初始化期间注册其 NAPI `poll` 函数。

### NAPI initialization in the `igb` driver

The `igb` driver does this via a long call chain:

`igb` 驱动程序通过一个长调用链来完成此操作：

1. `igb_probe` calls `igb_sw_init` .
2. `igb_sw_init` calls `igb_init_interrupt_scheme` .
3. `igb_init_interrupt_scheme` calls `igb_alloc_q_vectors` .
4. `igb_alloc_q_vectors` calls `igb_alloc_q_vector` .
5. `igb_alloc_q_vector` calls `netif_napi_add`.

This call trace results in a few high level things happening:

此调用跟踪会导致发生一些高级别事情：

1. If [MSI-X](https://en.wikipedia.org/wiki/Message_Signaled_Interrupts#MSI-X) is supported, it will be enabled with a call to `pci_enable_msix` . <br> 如果支持 [MSI-X](https://en.wikipedia.org/wiki/Message_Signaled_Interrupts#MSI-X)，则通过调用`pci_enable_msix`启用它。
2. Various settings are computed and initialized; most notably the number of transmit and receive queues that the device and driver will use for sending and receiving packets. <br> 计算和初始化各种设置; 最值得注意的是设备和驱动程序用于发送和接收数据包的发送和接收队列的数量。
3. `igb_alloc_q_vector` is called once for every transmit and receive queue that will be created. <br> 对于将要创建的每个发送和接收队列，调用一次 `igb_alloc_q_vector`。
4. Each call to `igb_alloc_q_vector` calls `netif_napi_add` to register a `poll` function for that queue and an instance of `struct napi_struct` that will be passed to `poll` when called to harvest packets. <br> 对 `igb_alloc_q_vector` 的每次调用都会调用 `netif_napi_add` 来为该队列注册一个`poll` 函数，以及一个 `struct napi_struct` 的实例，当被调用以收集数据包时，它将被传递给`poll`。

Let’s take a look at `igb_alloc_q_vector` to see how the `poll` callback and its private data are registered.

让我们看看 `igb_alloc_q_vector` ，看看如何注册 `poll` 回调及其私有数据。

From [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L1145-L1271) :

来自 [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L1145-L1271) :

```c
static int igb_alloc_q_vector(struct igb_adapter *adapter,
                              int v_count, int v_idx,
                              int txr_count, int txr_idx,
                              int rxr_count, int rxr_idx)
{
    /* ... */

    /* allocate q_vector and rings */
    q_vector = kzalloc(size, GFP_KERNEL);
    if (!q_vector)
        return -ENOMEM;

    /* initialize NAPI */
    netif_napi_add(adapter->netdev, &q_vector->napi, igb_poll, 64);

    /* ... */
}
```

The above code is allocation memory for a receive queue and registering the function `igb_poll` with the NAPI subsystem. It provides a reference to the `struct napi_struct` associated with this newly created RX queue (`&q_vector->napi` above). This will be passed into `igb_poll` when called by the NAPI subsystem when it comes time to harvest packets from this RX queue.

上面的代码为接收队列分配内存，并将函数 `igb_poll` 注册到 NAPI 子系统。它提供了与这个新创建的RX队列相关的 `struct napi_struct` 的引用（`&q_vector->napi`）。 当从该RX队列收集数据包时，当NAPI子系统调用时，这将被传递到 `igb_poll`。

This will be important later when we examine the flow of data from drivers up the network stack.

当我们检查来自网络堆栈的驱动程序的数据流时，这将非常重要。

