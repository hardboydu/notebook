## Bringing a network device up

Recall the `net_device_ops` structure we saw earlier which registered a set of functions for bringing the network device up, transmitting packets, setting the MAC address, etc.

回想一下我们之前看到的 `net_device_ops` 结构，它注册了一组用于启动网络设备，传输数据包，设置MAC地址等的功能。

When a network device is brought up (for example, with `ifconfig eth0 up`), the function attached to the `ndo_open` field of the `net_device_ops` structure is called.

当启动网络设备时（例如，使用`ifconfig eth0 up`），将调用附加到 `net_device_ops` 结构的`ndo_open` 字段的函数。

The `ndo_open` function will typically do things like:

`ndo_open` 函数通常会执行以下操作：

1. Allocate RX and TX queue memory <br> 分配 RX 和 TX 队列内存
2. Enable NAPI <br> 启用 NAPI
3. Register an interrupt handler <br> 注册中断处理程序
4. Enable hardware interrupts <br> 启用硬件中断
5. And more. <br> 还有更多。

In the case of the `igb` driver, the function attached to the ndo_open field of the `net_device_ops` structure is called `igb_open` .

对于 `igb` 驱动程序，附加到 `net_device_ops` 结构的 `ndo_open` 字段的函数称为`igb_open` 。

### Preparing to receive data from the network

Most NICs you’ll find today will use DMA to write data directly into RAM where the OS can retrieve the data for processing. The data structure most NICs use for this purpose resembles a queue built on circular buffer (or a ring buffer).

您今天发现的大多数 NIC 都将使用 DMA 将数据直接写入 RAM，操作系统可以在其中检索数据以进行处理。大多数 NIC 用于此目的的数据结构类似于构建在循环缓冲区（或环形缓冲区）上的队列。

In order to do this, the device driver must work with the OS to reserve a region of memory that the NIC hardware can use. Once this region is reserved, the hardware is informed of its location and incoming data will be written to RAM where it will later be picked up and processed by the networking subsystem.

为此，设备驱动程序必须与 OS 一起保留 NIC 硬件可以使用的内存区域。保留此区域后，将通知硬件其位置，并将传入的数据写入RAM，稍后将由网络子系统拾取和处理。

This seems simple enough, but what if the packet rate was high enough that a single CPU was not able to properly process all incoming packets? The data structure is built on a fixed length region of memory, so incoming packets would be dropped.

这看起来很简单，但是如果数据包速率足够高以至于单个 CPU 无法正确处理所有传入的数据包呢？数据结构建立在固定长度的内存区域上，因此传入的数据包将被丢弃。

This is where something known as known as Receive Side Scaling (RSS) or multiqueue can help.

这就是所谓的接收端缩放（RSS）或多队列可以提供帮助的地方。

Some devices have the ability to write incoming packets to several different regions of RAM simultaneously; each region is a separate queue. This allows the OS to use multiple CPUs to process incoming data in parallel, starting at the hardware level. This feature is not supported by all NICs.

一些设备能够同时将输入数据包写入 RAM 的几个不同区域；每个区域都是一个单独的队列。这允许操作系统使用多个 CPU 并行处理输入数据，从硬件级别开始。并不是所有NIC都支持此功能。

The Intel I350 NIC does support multiple queues. We can see evidence of this in the `igb` driver. One of the first things the `igb` driver does when it is brought up is call a function named `igb_setup_all_rx_resources`. This function calls another function, `igb_setup_rx_resources`, once for each `RX queue` to arrange for DMA-able memory where the device will write incoming data.

Intel I350 NIC 支持多个队列。我们可以在 igb 驱动程序中看到这方面的实现。igb 驱动程序在启动时所做的第一件事就是调用一个名为 `igb_setup_all_rx_resources` 的函数。此函数为每个 RX队列调用另一个函数 `igb_setup_rx_resources`，以安排DMA存储器，设备将在此处写入传入数据。

If you are curious how exactly this works, please see the Linux kernel’s DMA API HOWTO.

如果您很好奇这是如何工作的，请参阅 Linux 内核的 DMA API HOWTO。

It turns out the number and size of the RX queues can be tuned by using `ethtool`. Tuning these values can have a noticeable impact on the number of frames which are processed vs the number of frames which are dropped.

事实证明，可以使用 `ethtool` 调整 RX 队列的数量和大小。调整这些值会对处理的帧数与丢弃的帧数产生明显影响。

The NIC uses a hash function on the packet header fields (like source, destination, port, etc) to determine which RX queue the data should be directed to.

NIC 在数据包标头字段（如源，目标，端口等）上使用散列函数来确定数据应该指向哪个 RX 队列。

Some NICs let you adjust the weight of the RX queues, so you can send more traffic to specific queues.

某些 NIC 允许您调整 RX 队列的权重，因此您可以向特定队列发送更多流量。

Fewer NICs let you adjust this hash function itself. If you can adjust the hash function, you can send certain flows to specific RX queues for processing or even drop the packets at the hardware level, if desired.

较少的 NIC 允许您自己调整此哈希函数。如果您可以调整哈希函数，则可以将特定流发送到特定的RX队列进行处理，甚至可以根据需要在硬件级丢弃数据包。

We’ll take a look at how to tune these settings shortly.

我们将简要介绍如何调整这些设置。

### Enable NAPI

When a network device is brought up, a driver will usually enable NAPI.

启动网络设备时，驱动程序通常会启用 NAPI。

We saw earlier how drivers register `poll` functions with NAPI, but NAPI is not usually enabled until the device is brought up.

我们之前看到过驱动程序如何使用 NAPI 注册 poll 函数，但是在启动设备之前通常不会启用 NAPI。

Enabling NAPI is relatively straight forward. A call to `napi_enable` will flip a bit in the `struct napi_struct` to indicate that it is now enabled. As mentioned above, while NAPI will be enabled it will be in the off position.

启用 NAPI 相对简单。对 `napi_enable` 的调用将在 `struct napi_struct` 中翻转一下，表明它现在已启用。如上所述，虽然将启用NAPI，但它将处于关闭位置。

In the case of the `igb` driver, NAPI is enabled for each `q_vector` that was initialized when the driver was loaded or when the queue count or size are changed with `ethtool`.

对于 `igb` 驱动程序，为加载驱动程序时初始化的每个 `q_vector` 或使用 `ethtool` 更改队列计数或大小时启用NAPI。

From `drivers/net/ethernet/intel/igb/igb_main.c` :

来自 `drivers/net/ethernet/intel/igb/igb_main.c` :

```c
static int __igb_open(struct net_device *netdev, bool resuming)
{
    /* ... */

    for (i = 0; i < adapter->num_q_vectors; i++)
        napi_enable(&(adapter->q_vector[i]->napi));

    /* ... */
}
```

### Register an interrupt handler

After enabling NAPI, the next step is to register an interrupt handler. There are different methods a device can use to signal an interrupt: MSI-X, MSI, and legacy interrupts. As such, the code differs from device to device depending on what the supported interrupt methods are for a particular piece of hardware.

启用 NAPI 后，下一步是注册中断处理程序。设备可以使用不同的方法来发出中断信号：MSI-X，MSI和传统中断。因此，代码因设备而异，具体取决于支持的中断方法对于特定硬件的要求。

The driver must determine which method is supported by the device and register the appropriate handler function that will execute when the interrupt is received.

驱动程序必须确定设备支持哪种方法，并注册在收到中断时将执行的相应处理函数。

Some drivers, like the `igb` driver, will try to register an interrupt handler with each method, falling back to the next untested method on failure.

某些驱动程序（如`igb`驱动程序）将尝试使用每种方法注册中断处理程序，并在失败时返回到下一个未经测试的方法。

MSI-X interrupts are the preferred method, especially for NICs that support multiple RX queues. This is because each RX queue can have its own hardware interrupt assigned, which can then be handled by a specific CPU (with `irqbalance` or by modifying `/proc/irq/IRQ_NUMBER/smp_affinity`). As we’ll see shortly, the CPU that handles the interrupt will be the CPU that processes the packet. In this way, arriving packets can be processed by separate CPUs from the hardware interrupt level up through the networking stack.

MSI-X 中断是首选方法，尤其适用于支持多个 RX 队列的 NIC。这是因为每个 RX 队列都可以分配自己的硬件中断，然后由特定的 CPU 处理（使用 `irqbalance` 或通过修改 `/proc/irq/IRQ_NUMBER/smp_affinity`）。我们很快就会看到，处理中断的CPU将是处理数据包的CPU。通过这种方式，到达的数据包可以由单独的CPU从硬件中断级别通过网络堆栈处理。

If MSI-X is unavailable, MSI still presents advantages over legacy interrupts and will be used by the driver if the device supports it. Read this useful wiki page for more information about MSI and MSI-X.

如果 MSI-X 不可用，MSI 仍然具有优于传统中断的优势，并且如果设备支持，则驱动程序将使用MSI。有关 MSI 和 MSI-X 的更多信息，请阅读此有用的 Wiki 页面。

In the `igb` driver, the functions `igb_msix_ring`, `igb_intr_msi`, `igb_intr` are the interrupt handler methods for the MSI-X, MSI, and legacy interrupt modes, respectively.

在 `igb` 驱动程序中，函数`igb_msix_ring`， `igb_intr_msi`， `igb_intr` 分别是MSI-X，MSI和传统中断模式的中断处理程序方法。

You can find the code in the driver which attempts each interrupt method in `drivers/net/ethernet/intel/igb/igb_main.c`:

您可以在驱动程序中找到尝试每个中断方法的代码在`drivers/net/ethernet/intel/igb/igb_main.c` 中 ：

```c
/**
 *  igb_request_irq - initialize interrupts
 *  @adapter: board private structure to initialize
 *
 *  Attempts to configure interrupts using the best available
 *  capabilities of the hardware and kernel.
 **/
static int igb_request_irq(struct igb_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;
    int err = 0;

    if (adapter->flags & IGB_FLAG_HAS_MSIX) {
        err = igb_request_msix(adapter);
        if (!err)
            goto request_done;
        /* fall back to MSI */
        igb_free_all_tx_resources(adapter);
        igb_free_all_rx_resources(adapter);

        igb_clear_interrupt_scheme(adapter);
        err = igb_init_interrupt_scheme(adapter, false);
        if (err)
            goto request_done;

        igb_setup_all_tx_resources(adapter);
        igb_setup_all_rx_resources(adapter);
        igb_configure(adapter);
    }

    igb_assign_vector(adapter->q_vector[0], 0);

    if (adapter->flags & IGB_FLAG_HAS_MSI) {
        err = request_irq(pdev->irq, igb_intr_msi, 0,
                  netdev->name, adapter);
        if (!err)
            goto request_done;

        /* fall back to legacy interrupts */
        igb_reset_interrupt_capability(adapter);
        adapter->flags &= ~IGB_FLAG_HAS_MSI;
    }

    err = request_irq(pdev->irq, igb_intr, IRQF_SHARED,
                      netdev->name, adapter);

    if (err)
        dev_err(&pdev->dev, "Error %d getting interrupt\n",
                err);

request_done:
    return err;
}
```

As you can see in the abbreviated code above, the driver first attempts to set an MSI-X interrupt handler with `igb_request_msix`, falling back to MSI on failure. Next, `request_irq` is used to register `igb_intr_msi`, the MSI interrupt handler. If this fails, the driver falls back to legacy interrupts. `request_irq` is used again to register the legacy interrupt handler `igb_intr`.

正如您在上面的缩写代码中所看到的，驱动程序首先尝试使用 `igb_request_msix` 设置 MSI-X 中断处理程序，在失败时回退到 MSI。接下来，`request_irq` 用于注册MSI中断处理程序`igb_intr_msi`。如果此操作失败，则驱动程序将回退到传统中断。 `request_irq` 再次用于注册传统中断处理程序 `igb_intr` 。

And this is how the igb driver registers a function that will be executed when the NIC raises an interrupt signaling that data has arrived and is ready for processing.

这就是 `igb` 驱动程序如何注册一个函数，该函数将在 NIC 发出数据已到达并准备好处理的中断信号时执行。

### Enable Interrupts

At this point, almost everything is setup. The only thing left is to enable interrupts from the NIC and wait for data to arrive. Enabling interrupts is hardware specific, but the `igb` driver does this in `__igb_open` by calling a helper function named `igb_irq_enable` .

此时，几乎所有东西都已设置好。剩下的唯一事情就是启用来自NIC的中断并等待数据到达。启用中断是特定于硬件的，但是 `igb` 驱动程序通过调用名为 `igb_irq_enable` 的辅助函数在`__igb_open` 中执行此操作。

Interrupts are enabled for this device by writing to registers:

通过写入寄存器为该器件启用中断：

```c
/**
 *  igb_irq_enable - Enable default interrupt generation settings
 *  @adapter: board private structure
 **/
static void igb_irq_enable(struct igb_adapter *adapter)
{
    struct e1000_hw *hw = &adapter->hw;

    if (adapter->flags & IGB_FLAG_HAS_MSIX) {
        u32 ims = E1000_IMS_LSC | E1000_IMS_DOUTSYNC | E1000_IMS_DRSTA;
        u32 regval = rd32(E1000_EIAC);

        wr32(E1000_EIAC, regval | adapter->eims_enable_mask);
        regval = rd32(E1000_EIAM);
        wr32(E1000_EIAM, regval | adapter->eims_enable_mask);
        wr32(E1000_EIMS, adapter->eims_enable_mask);
        if (adapter->vfs_allocated_count) {
            wr32(E1000_MBVFIMR, 0xFF);
            ims |= E1000_IMS_VMMB;
        }
        wr32(E1000_IMS, ims);
    } else {
        wr32(E1000_IMS, IMS_ENABLE_MASK | E1000_IMS_DRSTA);
        wr32(E1000_IAM, IMS_ENABLE_MASK | E1000_IMS_DRSTA);
    }
}
```

### The network device is now up

Drivers may do a few more things like start timers, work queues, or other hardware-specific setup. Once that is completed. the network device is up and ready for use.

驱动程序可能会执行更多操作，如启动计时器，工作队列或其他特定于硬件的设置。一旦完成。网络设备已启动并可供使用。

Let’s take a look at monitoring and tuning settings for network device drivers.

我们来看看网络设备驱动程序的监视和调整设置。