### Extras

There are a few extra things worth mentioning that are worth mentioning which didn’t seem quite right anywhere else.

#### Timestamping

As mentioned in the above blog post, the networking stack can collect timestamps of incoming data. There are sysctl values controlling when/how to collect timestamps when used in conjunction with RPS; see the above post for more information on RPS, timestamping, and where, exactly, in the network stack receive timestamping happens. Some NICs even support timestamping in hardware, too.

This is a useful feature if you’d like to try to determine how much latency the kernel network stack is adding to receiving packets.

The kernel documentation about timestamping is excellent and there is even an included sample program and Makefile you can check out!.

Determine which timestamp modes your driver and device support with ethtool -T.

```sh
$ sudo ethtool -T eth0
Time stamping parameters for eth0:
Capabilities:
  software-transmit     (SOF_TIMESTAMPING_TX_SOFTWARE)
  software-receive      (SOF_TIMESTAMPING_RX_SOFTWARE)
  software-system-clock (SOF_TIMESTAMPING_SOFTWARE)
PTP Hardware Clock: none
Hardware Transmit Timestamp Modes: none
Hardware Receive Filter Modes: none
```

This NIC, unfortunately, does not support hardware receive timestamping, but software timestamping can still be used on this system to help me determine how much latency the kernel is adding to my packet receive path.

#### Busy polling for low latency sockets

It is possible to use a socket option called SO_BUSY_POLL which will cause the kernel to busy poll for new data when a blocking receive is done and there is no data.

IMPORTANT NOTE: For this option to work, your device driver must support it. Linux kernel 3.13.0’s igb driver does not support this option. The ixgbe driver, however, does. If your driver has a function set to the ndo_busy_poll field of its struct net_device_ops structure (mentioned in the above blog post), it supports SO_BUSY_POLL.

A great paper explaining how this works and how to use it is available from Intel.

When using this socket option for a single socket, you should pass a time value in microseconds as the amount of time to busy poll in the device driver’s receive queue for new data. When you issue a blocking read to this socket after setting this value, the kernel will busy poll for new data.

You can also set the sysctl value net.core.busy_poll to a time value in microseconds of how long calls with poll or select should busy poll waiting for new data to arrive, as well.

This option can reduce latency, but will increase CPU usage and power consumption.

#### Netpoll: support for networking in critical contexts

The Linux kernel provides a way for device drivers to be used to send and receive data on a NIC when the kernel has crashed. The API for this is called Netpoll and it is used by a few things, but most notably: kgdb, netconsole.

Most drivers support Netpoll; your driver needs to implement the ndo_poll_controller function and attach it to the struct net_device_ops that is registered during probe (as seen above).

When the networking device subsystem performs operations on incoming or outgoing data, the netpoll system is checked first to determine if the packet is destined for netpoll.

For example, we can see the following code in __netif_receive_skb_core from net/dev/core.c:

```c
static int __netif_receive_skb_core(struct sk_buff *skb, bool pfmemalloc)
{

  /* ... */

  /* if we've gotten here through NAPI, check netpoll */
  if (netpoll_receive_skb(skb))
    goto out;

  /* ... */
}
```

The Netpoll checks happen early in most of the Linux network device subsystem code that deals with transmitting or receiving network data.

Consumers of the Netpoll API can register struct netpoll structures by calling netpoll_setup. The struct netpoll structure has function pointers for attaching receive hooks, and the API exports a function for sending data.

If you are interested in using the Netpoll API, you should take a look at the netconsole driver, the Netpoll API header file, ‘include/linux/netpoll.h`, and this excellent talk.

#### SO_INCOMING_CPU

The SO_INCOMING_CPU flag was not added until Linux 3.19, but it is useful enough that it should be included in this blog post.

You can use getsockopt with the SO_INCOMING_CPU option to determine which CPU is processing network packets for a particular socket. Your application can then use this information to hand sockets off to threads running on the desired CPU to help increase data locality and CPU cache hits.

The mailing list message introducing SO_INCOMING_CPU provides a short example architecture where this option is useful.

#### DMA Engines

A DMA engine is a piece of hardware that allows the CPU to offload large copy operations. This frees the CPU to do other tasks while memory copies are done with hardware. Enabling the use of a DMA engine and running code that takes advantage of it, should yield reduced CPU usage.

The Linux kernel has a generic DMA engine interface that DMA engine driver authors can plug into. You can read more about the Linux DMA engine interface in the kernel source Documentation.

While there are a few DMA engines that the kernel supports, we’re going to discuss one in particular that is quite common: the Intel IOAT DMA engine.

##### Intel’s I/O Acceleration Technology (IOAT)

Many servers include the Intel I/O AT bundle, which is comprised of a series of performance changes.

One of those changes is the inclusion of a hardware DMA engine. You can check your dmesg output for ioatdma to determine if the module is being loaded and if it has found supported hardware.

The DMA offload engine is used in a few places, most notably in the TCP stack.

Support for the Intel IOAT DMA engine was included in Linux 2.6.18, but was disabled later in 3.13.11.10 due to some unfortunate data corruption bugs.

Users on kernels before 3.13.11.10 may be using the ioatdma module on their servers by default. Perhaps this will be fixed in future kernel releases.

###### Direct cache access (DCA)

Another interesting feature included with the Intel I/O AT bundle is Direct Cache Access (DCA).

This feature allows network devices (via their drivers) to place network data directly in the CPU cache. How this works, exactly, is driver specific. For the igb driver, you can check the code for the function igb_update_dca, as well as the code for igb_update_rx_dca. The igb driver uses DCA by writing a register value to the NIC.

To use DCA, you will need to ensure that DCA is enabled in your BIOS, the dca module is loaded, and that your network card and driver both support DCA.

###### Monitoring IOAT DMA engine

If you are using the ioatdma module despite the risk of data corruption mentioned above, you can monitor it by examining some entries in sysfs.

Monitor the total number of offloaded memcpy operations for a DMA channel.

```sh
$ cat /sys/class/dma/dma0chan0/memcpy_count
123205655
```

Similarly, to get the number of bytes offloaded by this DMA channel, you’d run a command like:

Monitor total number of bytes transferred for a DMA channel.

```
$ cat /sys/class/dma/dma0chan0/bytes_transferred
131791916307
```

###### Tuning IOAT DMA engine

The IOAT DMA engine is only used when packet size is above a certain threshold. That threshold is called the copybreak. This check is in place because for small copies, the overhead of setting up and using the DMA engine is not worth the accelerated transfer.

Adjust the DMA engine copybreak with a sysctl.

```sh
$ sudo sysctl -w net.ipv4.tcp_dma_copybreak=2048
```

The default value is 4096.

## Conclusion

The Linux networking stack is complicated.

It is impossible to monitor or tune it (or any other complex piece of software) without understanding at a deep level exactly what’s going on. Often, out in the wild of the Internet, you may stumble across a sample sysctl.conf that contains a set of sysctl values that should be copied and pasted on to your computer. This is probably not the best way to optimize your networking stack.

Monitoring the networking stack requires careful accounting of network data at every layer. Starting with the drivers and proceeding up. That way you can determine where exactly drops and errors are occurring and then adjust settings to determine how to reduce the errors you are seeing.

There is, unfortunately, no easy way out.

## Help with Linux networking or other systems

Need some extra help navigating the network stack? Have questions about anything in this post or related things not covered? Send us an email and let us know how we can help.

## Related posts

If you enjoyed this post, you may enjoy some of our other low-level technical posts:

* Monitoring and Tuning the Linux Networking Stack: Sending Data
* The Definitive Guide to Linux System Calls
* How does strace work?
* How does ltrace work?
* APT Hash sum mismatch
* HOWTO: GPG sign and verify deb packages and APT repositories
* HOWTO: GPG sign and verify RPM packages and yum repositories
