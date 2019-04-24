### Receive Packet Steering (RPS)

Recall earlier how we discussed that network device drivers register a NAPI poll function. Each NAPI poller instance is executed in the context of a softirq of which there is one per CPU. Further recall that the CPU which the driver’s IRQ handler runs on will wake its softirq processing loop to process packets.

In other words: a single CPU processes the hardware interrupt and polls for packets to process incoming data.

Some NICs (like the Intel I350) support multiple queues at the hardware level. This means incoming packets can be DMA’d to a separate memory region for each queue, with a separate NAPI structure to manage polling this region, as well. Thus multiple CPUs will handle interrupts from the device and also process packets.

This feature is typically called Receive Side Scaling (RSS).

Receive Packet Steering (RPS) is a software implementation of RSS. Since it is implemented in software, this means it can be enabled for any NIC, even NICs which have only a single RX queue. However, since it is in software, this means that RPS can only enter into the flow after a packet has been harvested from the DMA memory region.

This means that you wouldn’t notice a decrease in CPU time spent handling IRQs or the NAPI poll loop, but you can distribute the load for processing the packet after it’s been harvested and reduce CPU time from there up the network stack.

RPS works by generating a hash for incoming data to determine which CPU should process the data. The data is then enqueued to the per-CPU receive network backlog to be processed. An Inter-processor Interrupt (IPI) is delivered to the CPU owning the backlog. This helps to kick-start backlog processing if it is not currently processing data on the backlog. The /proc/net/softnet_stat contains a count of the number of times each softnet_data struct has received an IPI (the received_rps field).

Thus, netif_receive_skb will either continue sending network data up the networking stack, or hand it over to RPS for processing on a different CPU.

#### Tuning: Enabling RPS

For RPS to work, it must be enabled in the kernel configuration (it is on Ubuntu for kernel 3.13.0), and a bitmask describing which CPUs should process packets for a given interface and RX queue.

You can find some documentation about these bitmasks in the kernel documentation.

In short, the bitmasks to modify are found in:

```sh
/sys/class/net/DEVICE_NAME/queues/QUEUE/rps_cpus
```

So, for eth0 and receive queue 0, you would modify the file: /sys/class/net/eth0/queues/rx-0/rps_cpus with a hexadecimal number indicating which CPUs should process packets from eth0’s receive queue 0. As the documentation points out, RPS may be unnecessary in certain configurations.

Note: enabling RPS to distribute packet processing to CPUs which were previously not processing packets will cause the number of `NET_RX` softirqs to increase for that CPU, as well as the `si` or `sitime` in the CPU usage graph. You can compare before and after of your softirq and CPU usage graphs to confirm that RPS is configured properly to your liking.

### Receive Flow Steering (RFS)

Receive flow steering (RFS) is used in conjunction with RPS. RPS attempts to distribute incoming packet load amongst multiple CPUs, but does not take into account any data locality issues for maximizing CPU cache hit rates. You can use RFS to help increase cache hit rates by directing packets for the same flow to the same CPU for processing.

#### Tuning: Enabling RFS

For RFS to work, you must have RPS enabled and configured.

RFS keeps track of a global hash table of all flows and the size of this hash table can be adjusted by setting the net.core.rps_sock_flow_entries sysctl.

Increase the size of the RFS socket flow hash by setting a sysctl.

```sh
$ sudo sysctl -w net.core.rps_sock_flow_entries=32768
```

Next, you can also set the number of flows per RX queue by writing this value to the sysfs file named rps_flow_cnt for each RX queue.

Example: increase the number of flows for RX queue 0 on eth0 to 2048.

```sh
$ sudo bash -c 'echo 2048 > /sys/class/net/eth0/queues/rx-0/rps_flow_cnt'
```

### Hardware accelerated Receive Flow Steering (aRFS)

RFS can be sped up with the use of hardware acceleration; the NIC and the kernel can work together to determine which flows should be processed on which CPUs. To use this feature, it must be supported by the NIC and your driver.

Consult your NIC’s data sheet to determine if this feature is supported. If your NIC’s driver exposes a function called ndo_rx_flow_steer, then the driver has support for accelerated RFS.

#### Tuning: Enabling accelerated RFS (aRFS)

Assuming that your NIC and driver support it, you can enable accelerated RFS by enabling and configuring a set of things:

1. Have RPS enabled and configured.
2. Have RFS enabled and configured.
3. Your kernel has CONFIG_RFS_ACCEL enabled at compile time. The Ubuntu kernel 3.13.0 does.
4. Have ntuple support enabled for the device, as described previously. You can use ethtool to verify that ntuple support is enabled for the device.
5. Configure your IRQ settings to ensure each RX queue is handled by one of your desired network processing CPUs.

Once the above is configured, accelerated RFS will be used to automatically move data to the RX queue tied to a CPU core that is processing data for that flow and you won’t need to specify an ntuple filter rule manually for each flow.

