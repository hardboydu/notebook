## Monitoring network devices

There are several different ways to monitor your network devices offering different levels of granularity and complexity. Let’s start with most granular and move to least granular.

有几种不同的方法可以监控您的网络设备，提供不同级别的粒度和复杂性。让我们从最细粒度开始，然后移动到最小粒度。

### Using `ethtool -S`

You can install `ethtool` on an Ubuntu system by running: `sudo apt-get install ethtool`.

您可以通过运行以下命令在Ubuntu系统上安装`ethtool`：`sudo apt-get install ethtool`。

Once it is installed, you can access the statistics by passing the `-S` flag along with the name of the network device you want statistics about.

安装后，您可以通过传递 `-S` 标志以及要统计的网络设备的名称来访问统计信息。

Monitor detailed NIC device statistics (e.g., packet drops) with `ethtool -S`.

使用 `ethtool -S` 监视详细的NIC设备统计信息（例如，数据包丢弃）。

```sh
$ sudo ethtool -S p1p1
NIC statistics:
     rx_packets: 0
     tx_packets: 0
     rx_bytes: 0
     tx_bytes: 0
     rx_broadcast: 0
     tx_broadcast: 0
     rx_multicast: 0
     tx_multicast: 0
     multicast: 0
     collisions: 0
     rx_crc_errors: 0
     rx_no_buffer_count: 0
     rx_missed_errors: 0
     tx_aborted_errors: 0
     tx_carrier_errors: 0
     tx_window_errors: 0
     tx_abort_late_coll: 0
     tx_deferred_ok: 0
     tx_single_coll_ok: 0
     tx_multi_coll_ok: 0
     tx_timeout_count: 0
     rx_long_length_errors: 0
     rx_short_length_errors: 0
     rx_align_errors: 0
     tx_tcp_seg_good: 0
     tx_tcp_seg_failed: 0
     rx_flow_control_xon: 0
     rx_flow_control_xoff: 0
     tx_flow_control_xon: 0
     tx_flow_control_xoff: 0
     rx_long_byte_count: 0
     tx_dma_out_of_sync: 0
     tx_smbus: 0
     rx_smbus: 0
     dropped_smbus: 0
     os2bmc_rx_by_bmc: 0
     os2bmc_tx_by_bmc: 0
     os2bmc_tx_by_host: 0
     os2bmc_rx_by_host: 0
     tx_hwtstamp_timeouts: 0
     tx_hwtstamp_skipped: 0
     rx_hwtstamp_cleared: 0
     rx_errors: 0
     tx_errors: 0
     tx_dropped: 0
     rx_length_errors: 0
     rx_over_errors: 0
     rx_frame_errors: 0
     rx_fifo_errors: 0
     tx_fifo_errors: 0
     tx_heartbeat_errors: 0
     tx_queue_0_packets: 0
     tx_queue_0_bytes: 0
     tx_queue_0_restart: 0
     tx_queue_1_packets: 0
     tx_queue_1_bytes: 0
     tx_queue_1_restart: 0
     tx_queue_2_packets: 0
     tx_queue_2_bytes: 0
     tx_queue_2_restart: 0
     tx_queue_3_packets: 0
     tx_queue_3_bytes: 0
     tx_queue_3_restart: 0
     tx_queue_4_packets: 0
     tx_queue_4_bytes: 0
     tx_queue_4_restart: 0
     tx_queue_5_packets: 0
     tx_queue_5_bytes: 0
     tx_queue_5_restart: 0
     tx_queue_6_packets: 0
     tx_queue_6_bytes: 0
     tx_queue_6_restart: 0
     tx_queue_7_packets: 0
     tx_queue_7_bytes: 0
     tx_queue_7_restart: 0
     rx_queue_0_packets: 0
     rx_queue_0_bytes: 0
     rx_queue_0_drops: 0
     rx_queue_0_csum_err: 0
     rx_queue_0_alloc_failed: 0
     rx_queue_1_packets: 0
     rx_queue_1_bytes: 0
     rx_queue_1_drops: 0
     rx_queue_1_csum_err: 0
     rx_queue_1_alloc_failed: 0
     rx_queue_2_packets: 0
     rx_queue_2_bytes: 0
     rx_queue_2_drops: 0
     rx_queue_2_csum_err: 0
     rx_queue_2_alloc_failed: 0
     rx_queue_3_packets: 0
     rx_queue_3_bytes: 0
     rx_queue_3_drops: 0
     rx_queue_3_csum_err: 0
     rx_queue_3_alloc_failed: 0
     rx_queue_4_packets: 0
     rx_queue_4_bytes: 0
     rx_queue_4_drops: 0
     rx_queue_4_csum_err: 0
     rx_queue_4_alloc_failed: 0
     rx_queue_5_packets: 0
     rx_queue_5_bytes: 0
     rx_queue_5_drops: 0
     rx_queue_5_csum_err: 0
     rx_queue_5_alloc_failed: 0
     rx_queue_6_packets: 0
     rx_queue_6_bytes: 0
     rx_queue_6_drops: 0
     rx_queue_6_csum_err: 0
     rx_queue_6_alloc_failed: 0
     rx_queue_7_packets: 0
     rx_queue_7_bytes: 0
     rx_queue_7_drops: 0
     rx_queue_7_csum_err: 0
     rx_queue_7_alloc_failed: 0
```

Monitoring this data can be difficult. It is easy to obtain, but there is no standardization of the field values. Different drivers, or even different versions of the same driver might produce different field names that have the same meaning.

监控这些数据可能很困难。它很容易获得，但没有字段值的标准化。不同的驱动程序，甚至同一驱动程序的不同版本可能会生成具有相同含义的不同字段名称。

You should look for values with "drop", "buffer", "miss", etc in the label. Next, you will have to read your driver source. You’ll be able to determine which values are accounted for totally in software (e.g., incremented when there is no memory) and which values come directly from hardware via a register read. In the case of a register value, you should consult the data sheet for your hardware to determine what the meaning of the counter really is; many of the labels given via `ethtool` can be misleading.

您应该在标签中查找带有“drop”，“buffer”，“miss”等的值。接下来，您将必须阅读您的驱动程序源。您将能够确定哪些值完全在软件中占用（例如，在没有存储器时递增）以及哪些值通过寄存器读取直接来自硬件。在寄存器值的情况下，您应该查阅硬件的数据表，以确定计数器的含义是什么;许多通过`ethtool` 提供的标签可能会产生误导。

### Using `sysfs`

`sysfs` also provides a lot of statistics values, but they are slightly higher level than the direct NIC level stats provided.

`sysfs` 还提供了许多统计信息值，但它们的级别略高于提供的直接NIC级别统计信息。

You can find the number of dropped incoming network data frames for, e.g. eth0 by using cat on a file.

您可以找到丢弃的传入网络数据帧的数量，例如， p1p1 通过在文件上使用 `cat`。

Monitor higher level NIC statistics with sysfs.

使用 `sysfs` 监控更高级别的NIC统计信息。

```sh
$ cat /sys/class/net/p1p1/statistics/rx_dropped
0
```

The counter values will be split into files like `collisions`, `rx_dropped`, `rx_errors`, `rx_missed_errors`, etc.

计数器值将被分成诸如`collisions`，`rx_dropped`，`rx_errors`，`rx_missed_errors`等文件。

Unfortunately, it is up to the drivers to decide what the meaning of each field is, and thus, when to increment them and where the values come from. You may notice that some drivers count a certain type of error condition as a drop, but other drivers may count the same as a miss.

不幸的是，驱动程序需要决定每个字段的含义，以及何时增加它们以及值的来源。您可能会注意到某些驱动程序将某种类型的错误情况视为丢弃，但其他驱动程序可能会计为与未命中相同。

If these values are critical to you, you will need to read your driver source to understand exactly what your driver thinks each of these values means.

如果这些值对您至关重要，则需要阅读驱动程序源代码，以准确了解驱动程序认为每个值的含义。

### Using `/proc/net/dev`

An even higher level file is `/proc/net/dev` which provides high-level summary-esque information for each network adapter on the system.

更高级别的文件是 `/proc/net/dev`，它为系统上的每个网络适配器提供高级摘要信息。

Monitor high level NIC statistics by reading `/proc/net/dev`.

通过读取 `/proc/net/dev` 监视高级NIC统计信息。

```sh
$ sudo cat /proc/net/dev
Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
  p3p1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  p3p2:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  p4p1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  p4p2:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    lo: 52125068    4417    0    0    0     0          0         0 52125068    4417    0    0    0     0       0          0
virbr0-nic:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
virbr0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
bridge0:       0       0    0    0    0     0          0         0      126       3    0    0    0     0       0          0
   em3:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
   em1: 11516396  107828    0  474    0     0          0     21769 30315552  108121    0    0    0     0       0          0
   em4:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
   em2: 4856055   33553    0  474    0     0          0     21770        0       0    0    0    0     0       0          0
  p1p1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  p1p2:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0

```

This file shows a subset of the values you’ll find in the `sysfs` files mentioned above, but it may serve as a useful general reference.

此文件显示了您在上面提到的 `sysfs` 文件中找到的值的子集，但它可以作为有用的一般参考。

The caveat mentioned above applies here, as well: if these values are important to you, you will still need to read your driver source to understand exactly when, where, and why they are incremented to ensure your understanding of an error, drop, or fifo are the same as your driver.

上面提到的警告也适用于此：如果这些值对您很重要，您仍需要阅读驱动程序源代码，以确切了解它们的确切时间，位置和原因，以确保您理解错误，丢弃或 fifo 和你的驱动程序一样。