# [Monitoring and Tuning the Linux Networking Stack: Receiving Data Part2 Network Device Driver Initialization](https://blog.packagecloud.io/eng/2016/06/22/monitoring-tuning-linux-networking-stack-receiving-data/)

## Network Device Driver

### Initialization

A driver registers an initialization function which is called by the kernel when the driver is loaded. This function is registered by using the `module_init` macro.

驱动程序注册一个初始化函数，该函数在加载驱动程序时由内核调用。使用 `module_init` 宏注册此函数。

The `igb` initialization function (`igb_init_module`) and its registration with `module_init` can be found in [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L676-L697).

可以在 [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L676-L697) 中找到 `igb` 初始化函数（`igb_init_module`）及其对 `module_init` 的注册。

Both are fairly straightforward:

两者都相当简单：

```c
/**
 *  igb_init_module - Driver Registration Routine
 *
 *  igb_init_module is the first routine called when the driver is
 *  loaded. All it does is register with the PCI subsystem.
 **/
static int __init igb_init_module(void)
{
  int ret;
  pr_info("%s - version %s\n",
         igb_driver_string, igb_driver_version);

  pr_info("%s\n", igb_copyright);

#ifdef CONFIG_IGB_DCA
  dca_register_notify(&dca_notifier);
#endif
  ret = pci_register_driver(&igb_driver);
  return ret;
}


module_init(igb_init_module);
```

The bulk of the work to initialize the device happens with the call to `pci_register_driver` as we’ll see next.

初始化设备的大部分工作都是在调用 `pci_register_driver` 时发生的，我们将在下面看到。

#### PCI initialization

The *Intel I350* network card is a [*PCI express*](https://en.wikipedia.org/wiki/PCI_Express) device.

*Intel I350* 网卡是 [*PCI Express*](https://en.wikipedia.org/wiki/PCI_Express) 设备。

PCI devices identify themselves with a series of registers in the [PCI Configuration Space](https://en.wikipedia.org/wiki/PCI_configuration_space#Standardized_registers).

PCI 设备通过 [PCI 配置空间](https://en.wikipedia.org/wiki/PCI_configuration_space#Standardized_registers) 中的一系列寄存器识别自身。

When a device driver is compiled, a macro named `MODULE_DEVICE_TABLE` (from `include/module.h`) is used to export a table of PCI device IDs identifying devices that the device driver can control. The table is also registered as part of a structure, as we’ll see shortly.

编译设备驱动程序时，会使用名为 `MODULE_DEVICE_TABLE`（来自 [`include/module.h`](https://github.com/torvalds/linux/blob/v3.13/include/linux/module.h#L145-L146)）的宏来导出 PCI 设备 ID 表，以识别设备驱动程序可以控制的设备。该表也被注册为结构的一部分，我们很快就会看到。

The kernel uses this table to determine which device driver to load to control the device.

内核使用此表来确定要加载哪个设备驱动程序来控制设备。

That’s how the OS can figure out which devices are connected to the system and which driver should be used to talk to the device.

这就是操作系统如何确定哪些设备连接到系统以及应该使用哪个驱动程序与设备通信。

This table and the PCI device IDs for the igb driver can be found in [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L79-L117) and [`drivers/net/ethernet/intel/igb/e1000_hw.h`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/e1000_hw.h#L41-L75), respectively:

此表和 `igb` 驱动程序的 PCI 设备 ID 可分别在 [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L79-L117) 和 [`drivers/net/ethernet/intel/igb/e1000_hw.h`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/e1000_hw.h#L41-L75) 中找到：

```c
static DEFINE_PCI_DEVICE_TABLE(igb_pci_tbl) = {
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I354_BACKPLANE_1GBPS) },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I354_SGMII) },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I354_BACKPLANE_2_5GBPS) },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I211_COPPER), board_82575 },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_COPPER), board_82575 },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_FIBER), board_82575 },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_SERDES), board_82575 },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_SGMII), board_82575 },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_COPPER_FLASHLESS), board_82575 },
  { PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_SERDES_FLASHLESS), board_82575 },

  /* ... */
};
MODULE_DEVICE_TABLE(pci, igb_pci_tbl);
```

As seen in the previous section, `pci_register_driver` is called in the driver’s initialization function.

如上一节所示，在驱动程序的初始化函数中调用 `pci_register_driver`。

This function registers a structure of pointers. Most of the pointers are function pointers, but the PCI device ID table is also registered. The kernel uses the functions registered by the driver to bring the PCI device up.

此函数注册指针结构。大多数指针都是函数指针，但也注册了PCI设备ID表。内核使用驱动程序注册的功能来启动PCI设备。

From [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L238-L249) :

```c
static struct pci_driver igb_driver = {
  .name     = igb_driver_name,
  .id_table = igb_pci_tbl,
  .probe    = igb_probe,
  .remove   = igb_remove,

  /* ... */
};
```

#### PCI probe

Once a device has been identified by its `PCI IDs`, the kernel can then select the proper driver to use to control the device. Each PCI driver registers a probe function with the PCI system in the kernel. The kernel calls this function for devices which have not yet been claimed by a device driver. Once a device is claimed, other drivers will not be asked about the device. Most drivers have a lot of code that runs to get the device ready for use. The exact things done vary from driver to driver.

一旦设备通过其 `PCI IDs` 识别，内核就可以选择用于控制设备的适当驱动程序。每个 PCI 驱动程序都在内核中向 PCI 系统注册探测函数。内核为尚未被设备驱动程序认领的设备调用此函数。认领设备后，系统不会询问其他驱动程序。大多数驱动程序都有很多代码可以运行以使设备可以使用。确切的事情因驱动程序而异。

Some typical operations to perform include:

一些典型的操作包括：

1. Enabling the PCI device. <br> 启用PCI设备。
2. Requesting memory ranges and [IO ports](http://wiki.osdev.org/I/O_Ports). <br> 请求内存范围和[I/O 端口](http://wiki.osdev.org/I/O_Ports)。
3. Setting the [DMA](https://en.wikipedia.org/wiki/Direct_memory_access) mask. <br> 设置 [DMA](https://en.wikipedia.org/wiki/Direct_memory_access) 掩码。
4. The ethtool (described more below) functions the driver supports are registered. <br> 注册驱动程序支持的ethtool（下面将详细介绍）函数。
5. Any watchdog tasks needed (for example, e1000e has a watchdog task to check if the hardware is hung). <br> 需要任何监视任务（例如，e1000e有一个监视任务来检查硬件是否挂起）。
6. Other device specific stuff like workarounds or dealing with hardware specific quirks or similar. <br> 其他特定于设备的东西，如变通方法或处理硬件特定的怪癖或类似的东西。
7. The creation, initialization, and registration of a `struct net_device_ops` structure. This structure contains function pointers to the various functions needed for opening the device, sending data to the network, setting the MAC address, and more. <br> `struct net_device_ops` 结构的创建，初始化和注册。 此结构包含指向打开设备，向网络发送数据，设置MAC地址等所需的各种功能的函数指针。
8. The creation, initialization, and registration of a high level `struct net_device` which represents a network device. <br> 表示网络设备的高层 `struct net_device` 的创建，初始化和注册。

Let’s take a quick look at some of these operations in the `igb` driver in the function [`igb_probe`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L2005-L2429).

让我们快速浏览函数 [`igb_probe`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L2005-L2429) 中`igb`驱动程序中的一些操作。

##### A peek into PCI initialization

The following code from the `igb_probe` function does some basic PCI configuration. From [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L2038-L2059) :

`igb_probe` 函数中的以下代码执行一些基本的 PCI 配置。来自 [`drivers/net/ethernet/intel/igb/igb_main.c`](https://github.com/torvalds/linux/blob/v3.13/drivers/net/ethernet/intel/igb/igb_main.c#L2038-L2059) ：

```c
err = pci_enable_device_mem(pdev);

/* ... */

err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));

/* ... */

err = pci_request_selected_regions(pdev, pci_select_bars(pdev,
           IORESOURCE_MEM),
           igb_driver_name);

pci_enable_pcie_error_reporting(pdev);

pci_set_master(pdev);
pci_save_state(pdev);
```

First, the device is initialized with `pci_enable_device_mem`. This will wake up the device if it is suspended, enable memory resources, and more.

首先，使用 `pci_enable_device_mem` 初始化设备。如果设备被挂起，则会唤醒设备，启用内存资源等。

Next, the [DMA](https://en.wikipedia.org/wiki/Direct_memory_access) mask will be set. This device can read and write to 64bit memory addresses, so `dma_set_mask_and_coherent` is called with `DMA_BIT_MASK(64)`.

接下来，将设置 [DMA](https://en.wikipedia.org/wiki/Direct_memory_access) 掩码。该器件可以读写64位存储器地址，因此使用 `DMA_BIT_MASK(64)`调用`dma_set_mask_and_coherent`。

Memory regions will be reserved with a call to `pci_request_selected_regions`, [PCI Express Advanced Error Reporting](https://github.com/torvalds/linux/blob/v3.13/Documentation/PCI/pcieaer-howto.txt) is enabled (if the PCI AER driver is loaded), DMA is enabled with a call to `pci_set_master`, and the PCI configuration space is saved with a call to `pci_save_state`.

通过调用`pci_request_selected_regions`保留内存区域，启用 [PCI Express 高级错误报告](https://github.com/torvalds/linux/blob/v3.13/Documentation/PCI/pcieaer-howto.txt)（如果加载了PCI AER驱动程序），通过调用 `pci_set_master` 启用 **DMA**，并通过调用 `pci_save_state` 保存PCI配置空间。

Phew.

#### More Linux PCI driver information

Going into the full explanation of how PCI devices work is beyond the scope of this post, but this [excellent talk](https://bootlin.com/doc/legacy/pci-drivers/pci-drivers.pdf), [this wiki](http://wiki.osdev.org/PCI), and [this text file from the linux kernel](https://github.com/torvalds/linux/blob/v3.13/Documentation/PCI/pci.txt) are excellent resources.

详细解释PCI设备如何工作超出了本文的范围，但是这个优秀的[话题](https://bootlin.com/doc/legacy/pci-drivers/pci-drivers.pdf)，[这个wiki](http://wiki.osdev.org/PCI)以及来自[linux内核的这个文本文件](https://github.com/torvalds/linux/blob/v3.13/Documentation/PCI/pci.txt)都是很好的资源。