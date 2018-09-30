# IXY 代码的阅读笔记

## 1 起点 `ixy-fwd.c`

### 1.1 初始化设备

```c
int main(int argc, char* argv[]) {
    /* 两个参数 分别为 端口1 和 端口 2 的总线地址 */
    if (argc != 3) {
        printf("%s forwards packets between two ports.\n", argv[0]);
        printf("Usage: %s <pci bus id2> <pci bus id1>\n", argv[0]);
        return 1;
    }

    /* 根据总线地址 为两个端口创建 ixy 设备对象 */
    struct ixy_device* dev1 = ixy_init(argv[1], 1, 1);
    struct ixy_device* dev2 = ixy_init(argv[2], 1, 1);
```

#### 1.1.1 创建并初始化 `ixy` 设备对象

```c
struct ixy_device* ixy_init(const char* pci_addr, uint16_t rx_queues, uint16_t tx_queues) {
    // Read PCI configuration space
    int config = pci_open_resource(pci_addr, "config");
    uint16_t vendor_id = read_io16(config, 0);
    uint16_t device_id = read_io16(config, 2);
    uint32_t class_id = read_io32(config, 8) >> 24;
    close(config);
    if (class_id != 2) {
        error("Device %s is not a NIC", pci_addr);
    }
    if (vendor_id == 0x1af4 && device_id >= 0x1000) {
        return virtio_init(pci_addr, rx_queues, tx_queues);
    } else {
        // Our best guess is to try ixgbe
        return ixgbe_init(pci_addr, rx_queues, tx_queues);
    }
}
```

在初始化 设备 对象之前，首先要读取 PCI 设备信息。

```c
int config = pci_open_resource(pci_addr, "config");
uint16_t vendor_id = read_io16(config, 0);
uint16_t device_id = read_io16(config, 2);
uint32_t class_id = read_io32(config, 8) >> 24;
close(config);
```

分别读取了三个变量 `vendor_id` 厂商ID，`device_id` 设备ID，`class_id` 类别的高 8 位（也就是分组信息），这里分组信息必须为 2 也就是 network 类：

```c
    if (class_id != 2) {
        error("Device %s is not a NIC", pci_addr);
    }
```

然后根据厂商 ID 和 设备 ID选择合适的驱动接口初始化：

```c
    if (vendor_id == 0x1af4 && device_id >= 0x1000) {
        return virtio_init(pci_addr, rx_queues, tx_queues);
    } else {
        // Our best guess is to try ixgbe
        return ixgbe_init(pci_addr, rx_queues, tx_queues);
    }
```

##### 1.1.1.1 读取 `PCIe` 设备的配置信息，识别设备类型

读取 `PCIe` 设备的配置信息，是通过读取 `sysfs` 中的文件 `/sys/bus/pci/devices/<bus id>/config` 实现的。

```c
int pci_open_resource(const char* pci_addr, const char* resource) {
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/sys/bus/pci/devices/%s/%s", pci_addr, resource);
    debug("Opening PCI resource at %s", path);
    int fd = check_err(open(path, O_RDWR), "open pci resource");
    return fd;
}
```

###### sysfs 中的 `PCI` 设备信息

首先先看一下在我的服务器上实际显示情况：

```sh
[root@localhost ~]# ll /sys/bus/pci/devices/0000\:02\:00.0/
total 0
-rw-r--r--. 1 root root    4096 Oct  1 11:23 broken_parity_status
-r--r--r--. 1 root root    4096 Oct  1 11:23 class
-rw-r--r--. 1 root root    4096 Oct  1 10:55 config
-r--r--r--. 1 root root    4096 Oct  1 11:23 consistent_dma_mask_bits
-rw-r--r--. 1 root root    4096 Oct  1 11:23 d3cold_allowed
-r--r--r--. 1 root root    4096 Oct  1 10:55 device
-r--r--r--. 1 root root    4096 Oct  1 11:23 dma_mask_bits
lrwxrwxrwx. 1 root root       0 Oct  1 10:55 driver -> ../../../../bus/pci/drivers/ixgbe
-rw-r--r--. 1 root root    4096 Oct  1 11:23 driver_override
-rw-r--r--. 1 root root    4096 Oct  1 11:23 enable
lrwxrwxrwx. 1 root root       0 Oct  1 11:23 firmware_node -> ../../../LNXSYSTM:00/device:00/PNP0A08:00/device:6c/device:6d
-r--r--r--. 1 root root    4096 Oct  1 11:23 irq
-r--r--r--. 1 root root    4096 Oct  1 11:23 local_cpulist
-r--r--r--. 1 root root    4096 Oct  1 11:23 local_cpus
-r--r--r--. 1 root root    4096 Oct  1 11:23 modalias
-rw-r--r--. 1 root root    4096 Oct  1 11:23 msi_bus
drwxr-xr-x. 2 root root       0 Oct  1 11:23 msi_irqs
drwxr-xr-x. 3 root root       0 Oct  1 10:55 net
-r--r--r--. 1 root root    4096 Oct  1 11:23 numa_node
drwxr-xr-x. 2 root root       0 Oct  1 11:23 power
--w--w----. 1 root root    4096 Oct  1 11:23 remove
--w--w----. 1 root root    4096 Oct  1 11:23 rescan
--w-------. 1 root root    4096 Oct  1 11:23 reset
-r--r--r--. 1 root root    4096 Oct  1 11:23 resource
-rw-------. 1 root root 8388608 Oct  1 11:23 resource0
-rw-------. 1 root root      32 Oct  1 11:23 resource2
-rw-------. 1 root root   16384 Oct  1 11:23 resource4
-rw-------. 1 root root 4194304 Oct  1 11:23 rom
-rw-rw-r--. 1 root root    4096 Oct  1 11:23 sriov_numvfs
-r--r--r--. 1 root root    4096 Oct  1 11:23 sriov_totalvfs
lrwxrwxrwx. 1 root root       0 Oct  1 10:55 subsystem -> ../../../../bus/pci
-r--r--r--. 1 root root    4096 Oct  1 11:23 subsystem_device
-r--r--r--. 1 root root    4096 Oct  1 11:23 subsystem_vendor
-rw-r--r--. 1 root root    4096 Oct  1 10:55 uevent
-r--r--r--. 1 root root    4096 Oct  1 10:55 vendor
-rw-------. 1 root root   32768 Oct  1 11:23 vpd
```

然后参考文档 `Documentation/filesystems/sysfs-pci.txt`

| file                 | function
|----------------------|------------------------------------------------------
| class                | PCI class (ascii, ro)
| config               | PCI config space (binary, rw)
| device               | PCI device (ascii, ro)
| enable               | Whether the device is enabled (ascii, rw)
| irq                  | IRQ number (ascii, ro)
| local_cpus           | nearby CPU mask (cpumask, ro)
| remove               | remove device from kernel's list (ascii, wo)
| resource             | PCI resource host addresses (ascii, ro)
| resource0..N         | PCI resource N, if present (binary, mmap, rw[1])
| resource0_wc..N_wc   | PCI WC map resource N, if prefetchable (binary, mmap)
| revision             | PCI revision (ascii, ro)
| rom                  | PCI ROM resource, if present (binary, ro)
| subsystem_device     | PCI subsystem device (ascii, ro)
| subsystem_vendor     | PCI subsystem vendor (ascii, ro)
| vendor               | PCI vendor (ascii, ro)

* `ro`      - read only file
* `rw`      - file is readable and writable
* `wo`      - write only file
* `mmap`    - file is mmapable
* `ascii`   - file contains ascii text
* `binary`  - file contains binary data
* `cpumask` - file contains a cpumask type

config 文件为PCI设备的 配置空间，二进制结构，可以读写。

**LDD3** 第12章的描述：

The file `config` is a binary file that allows the raw `PCI` config information to be read from the device (just like the `/proc/bus/pci/*/*` provides.) The files `vendor`, `device`, `subsystem_device`, `subsystem_vendor`, and `class` all refer to the specific values of this `PCI` device (all `PCI` devices provide this information.) The file `irq` shows the current `IRQ` assigned to this `PCI` device, and the file `resource` shows the current memory resources allocated by this device.

`config`文件是一个二进制文件，允许从设备读取原始的 `PCI` 配置信息（就像 `/proc/bus/pci/*/*` 提供的那样）。`vendor`，`device`，`subsystem_device`，`subsystem_vendor` 和 `class` 都表示该 `PCI` 设备的特定值（所有PCI设备都提供此信息）。文件 irq 显示分配给此 `PCI` 设备的当前IRQ，`resource` 显示此设备分配的当前内存资源。

![image1](image/ixy3img01.PNG)

*Figure 12-2. The standardized PCI configuration registers*

Three or five PCI registers identify a device: `vendorID` , `deviceID` , and `class` are the three that are always used. Every PCI manufacturer assigns proper values to these read-only registers, and the driver can use them to look for the device. Additionally, the fields `subsystem vendorID` and `subsystem deviceID` are sometimes set by the vendor to further differentiate similar devices.

用三个或五个PCI寄存器可标识一个设备：`vendorID` , `deviceID` , 和 `class`是常用的三个寄存器。每个PCI制造商会将正确的值赋予这三个只读寄存器，驱动程序可利用它们查询设备。此外，有时厂商利用 `subsystem vendorID` 和 `subsystem deviceID` 两个字段来进一步区分相似的设备。

**vendorID**

This 16-bit register identifies a hardware manufacturer. For instance, every Intel device is marked with the same vendor number, 0x8086 . There is a global registry of such numbers, maintained by the PCI Special Interest Group, and manufacturers must apply to have a unique number assigned to them.

这个 16 位的寄存器，用于标识硬件制造商。例如，每个 Intel 设备被标识为同一个厂商编号 `0x8086`，`PCI Special Interest Group` 维护有一个全球的厂商编号注册表，制造商必须申请一个唯一编号并赋予它们的寄存器。

**deviceID**

This is another 16-bit register, selected by the manufacturer; no official registration is required for the device ID. This ID is usually paired with the vendor ID to make a unique 32-bit identifier for a hardware device. We use the word signature to refer to the vendor and device ID pair. A device driver usually relies on the signature to identify its device; you can find what value to look for in the hardware manual for the target device.

这是另外一个 16 位寄存器，由制造商选择；无需对设备ID进行官方注册。该ID 通常和产商ID配对生成一个唯一的 32位硬件设备标识符。我们使用签名（signature）依次来表示一堆厂商和设备ID。设备驱动程序通常依靠该签名来识别其设备；可以从硬件手册中找到目标设备的签名值。

**class**

Every peripheral device belongs to a class. The class register is a 16-bit value whose top 8 bits identify the “base class” (or group). For example, “ethernet” and “token ring” are two classes belonging to the “network” group, while the “serial” and “parallel” classes belong to the “communication” group. Some drivers can support several similar devices, each of them featuring a different signature but all belonging to the same class; these drivers can rely on the class register to identify their peripherals, as shown later.

每个外部设备属于某个类（class）。class 寄存器是一个 16 位的值，其中，高 8 位标识了 “基类（base class）”，或者组。例如 “ethernet （以太网）”和 “token ring（令牌环）” 是同属 “network （网络）”组的两个类，而 “serial （串行）”和 “parallel（并行）”类同属 “communication（通信）” 组。某些驱动程序可支持多个相似的设备，每个具有不同的签名，但都属于同一个类；这些驱动程序可依靠 class 寄存器来识别他们的外设。如后所述。

**subsystem vendorID**
**subsystem deviceID**

These fields can be used for further identification of a device. If the chip is a generic interface chip to a local (onboard) bus, it is often used in several completely different roles, and the driver must identify the actual device it is talking with. The subsystem identifiers are used to this end.

这两个字段可用来进一步识别设备。如果设备中的芯片是一个连接到本地板载（onboard）总线上的通用接口芯片，则可能会用于完全不同的多种用途，这时，驱动程序必须识别它所关心的实际设备。子系统标识符就用于此目的。

Using these different identifiers, a PCI driver can tell the kernel what kind of devices it supports.

PCI 驱动程序可以使用这些不同的标识符来告诉内核它支持什么样的设备。

##### 1.1.1.2 初始化 `ixgbe` 设备

在厂商ID 为 0x8086 设备 ID 为 万兆网卡的设备ID 如，我的服务器 82599ES 的设备ID为 0x10fb,就会调用 `ixgbe_init`：

```c
struct ixy_device* ixgbe_init(const char* pci_addr, uint16_t rx_queues, uint16_t tx_queues) {
    if (getuid()) {
        warn("Not running as root, this will probably fail");
    }
    if (rx_queues > MAX_QUEUES) {
        error("cannot configure %d rx queues: limit is %d", rx_queues, MAX_QUEUES);
    }
    if (tx_queues > MAX_QUEUES) {
        error("cannot configure %d tx queues: limit is %d", tx_queues, MAX_QUEUES);
    }
    struct ixgbe_device* dev = (struct ixgbe_device*) malloc(sizeof(struct  ixgbe_device));
    dev->ixy.pci_addr = strdup(pci_addr);
    dev->ixy.driver_name = driver_name;
    dev->ixy.num_rx_queues = rx_queues;
    dev->ixy.num_tx_queues = tx_queues;
    dev->ixy.rx_batch = ixgbe_rx_batch;
    dev->ixy.tx_batch = ixgbe_tx_batch;
    dev->ixy.read_stats = ixgbe_read_stats;
    dev->ixy.set_promisc = ixgbe_set_promisc;
    dev->ixy.get_link_speed = ixgbe_get_link_speed;
    dev->addr = pci_map_resource(pci_addr);
    dev->rx_queues = calloc(rx_queues, sizeof(struct ixgbe_rx_queue) + sizeof(void*) * MAX_RX_QUEUE_ENTRIES);
    dev->tx_queues = calloc(tx_queues, sizeof(struct ixgbe_tx_queue) + sizeof(void*) * MAX_TX_QUEUE_ENTRIES);
    reset_and_init(dev);
    return &dev->ixy;
}
```

> 设备ID 的类型 需要参考 《[Intel ® 82599 10 GbE Controller Datasheet](https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/82599-10-gbe-controller-datasheet.pdf)》 和 《[Intel ®  82599 10 GbE Controller Specification Update](https://www.intel.com/content/dam/www/public/us/en/documents/specification-updates/82599-10-gbe-controller-spec-update.pdf)》，在规格说明中 `0x10fb` 的设备类型为 `82599 (SFI/SFP+)`

`device.h` 定义的 `struct ixy_device` 为 ixy 设备的通用结构：

```c
struct ixy_device {
    const char* pci_addr;
    const char* driver_name;
    uint16_t num_rx_queues;
    uint16_t num_tx_queues;
    uint32_t (*rx_batch) (struct ixy_device* dev, uint16_t queue_id, struct pkt_buf* bufs[], uint32_t num_bufs);
    uint32_t (*tx_batch) (struct ixy_device* dev, uint16_t queue_id, struct pkt_buf* bufs[], uint32_t num_bufs);
    void (*read_stats) (struct ixy_device* dev, struct device_stats* stats);
    void (*set_promisc) (struct ixy_device* dev, bool enabled);
    uint32_t (*get_link_speed) (const struct ixy_device* dev);
};
```

`ixgbe.h`  中定义了 `ixgbe` 设备的结构 `struct ixgbe_device` ，其中包含了 ixy 的设备对象，也就是继承了 ixy 的对象：

```c
struct ixgbe_device {
    struct ixy_device ixy;
    uint8_t* addr;
    void* rx_queues;
    void* tx_queues;
};
```

ixgbe设备通过pci_map_resource 将 BAR0 地址空间映射到 ixy 应用的地址空间，并由 ixgbe 设备结构中的 addr 成员变量指向这个地址空间。

```c
dev->addr = pci_map_resource(pci_addr);
```

```c
uint8_t* pci_map_resource(const char* pci_addr) {
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/sys/bus/pci/devices/%s/resource0", pci_addr);
    debug("Mapping PCI resource at %s", path);
    remove_driver(pci_addr);
    enable_dma(pci_addr);
    int fd = check_err(open(path, O_RDWR), "open pci resource");
    struct stat stat;
    check_err(fstat(fd, &stat), "stat pci resource");
    return (uint8_t*) check_err(mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0), "mmap pci resource");
}
```

以后 ixgbe 设备的操作都需要通过读写这个内存区域来实现。

初始化完毕相应接口后，ixgbe_init 需要申请 接收和发送 队列：

```c
    dev->rx_queues = calloc(rx_queues, sizeof(struct ixgbe_rx_queue) + sizeof(void*) * MAX_RX_QUEUE_ENTRIES);
    dev->tx_queues = calloc(tx_queues, sizeof(struct ixgbe_tx_queue) + sizeof(void*) * MAX_TX_QUEUE_ENTRIES);
```

最后 `ixgbe_init` 调用 `reset_and_init` 重置并重新初始化网卡：

```c
// see section 4.6.3
static void reset_and_init(struct ixgbe_device* dev) {
    info("Resetting device %s", dev->ixy.pci_addr);
    // section 4.6.3.1 - disable all interrupts
    set_reg32(dev->addr, IXGBE_EIMC, 0x7FFFFFFF);

    // section 4.6.3.2
    set_reg32(dev->addr, IXGBE_CTRL, IXGBE_CTRL_RST_MASK);
    wait_clear_reg32(dev->addr, IXGBE_CTRL, IXGBE_CTRL_RST_MASK);
    usleep(10000);

    // section 4.6.3.1 - disable interrupts again after reset
    set_reg32(dev->addr, IXGBE_EIMC, 0x7FFFFFFF);

    info("Initializing device %s", dev->ixy.pci_addr);

    // section 4.6.3 - Wait for EEPROM auto read completion
    wait_set_reg32(dev->addr, IXGBE_EEC, IXGBE_EEC_ARD);

    // section 4.6.3 - Wait for DMA initialization done (RDRXCTL.DMAIDONE)
    wait_set_reg32(dev->addr, IXGBE_RDRXCTL, IXGBE_RDRXCTL_DMAIDONE);

    // section 4.6.4 - initialize link (auto negotiation)
    init_link(dev);

    // section 4.6.5 - statistical counters
    // reset-on-read registers, just read them once
    ixgbe_read_stats(&dev->ixy, NULL);

    // section 4.6.7 - init rx
    init_rx(dev);

    // section 4.6.8 - init tx
    init_tx(dev);

    // enables queues after initializing everything
    for (uint16_t i = 0; i < dev->ixy.num_rx_queues; i++) {
        start_rx_queue(dev, i);
    }
    for (uint16_t i = 0; i < dev->ixy.num_tx_queues; i++) {
        start_tx_queue(dev, i);
    }

    // skip last step from 4.6.3 - don't want interrupts
    // finally, enable promisc mode by default, it makes testing less annoying
    ixgbe_set_promisc(&dev->ixy, true);

    // wait for some time for the link to come up
    wait_for_link(dev);
}
```
