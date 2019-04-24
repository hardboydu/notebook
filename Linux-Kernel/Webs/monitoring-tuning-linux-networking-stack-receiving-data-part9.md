

#### Data arrives

At long last; network data arrives!

Assuming that the RX queue has enough available descriptors, the packet is written to RAM via DMA. The device then raises the interrupt that is assigned to it (or in the case of MSI-X, the interrupt tied to the rx queue the packet arrived on).

##### Interrupt handler

In general, the interrupt handler which runs when an interrupt is raised should try to defer as much processing as possible to happen outside the interrupt context. This is crucial because while an interrupt is being processed, other interrupts may be blocked.

Let’s take a look at the source for the MSI-X interrupt handler; it will really help illustrate the idea that the interrupt handler does as little work as possible.

From drivers/net/ethernet/intel/igb/igb_main.c:

```c
static irqreturn_t igb_msix_ring(int irq, void *data)
{
    struct igb_q_vector *q_vector = data;

    /* Write the ITR value calculated from the previous interrupt. */
    igb_write_itr(q_vector);

    napi_schedule(&q_vector->napi);

    return IRQ_HANDLED;
}
```

This interrupt handler is very short and performs 2 very quick operations before returning.

First, this function calls igb_write_itr which simply updates a hardware specific register. In this case, the register that is updated is one which is used to track the rate hardware interrupts are arriving.

This register is used in conjunction with a hardware feature called “Interrupt Throttling” (also called “Interrupt Coalescing”) which can be used to to pace the delivery of interrupts to the CPU. We’ll see soon how ethtool provides a mechanism for adjusting the rate at which IRQs fire.

Secondly, napi_schedule is called which wakes up the NAPI processing loop if it was not already active. Note that the NAPI processing loop executes in a softirq; the NAPI processing loop does not execute from the interrupt handler. The interrupt handler simply causes it to start executing if it was not already.

The actual code showing exactly how this works is important; it will guide our understanding of how network data is processed on multi-CPU systems.

##### NAPI and `napi_schedule`

Let’s figure out how the napi_schedule call from the hardware interrupt handler works.

Remember, NAPI exists specifically to harvest network data without needing interrupts from the NIC to signal that data is ready for processing. As mentioned earlier, the NAPI poll loop is bootstrapped by receiving a hardware interrupt. In other words: NAPI is enabled, but off, until the first packet arrives at which point the NIC raises an IRQ and NAPI is started. There are a few other cases, as we’ll see soon, where NAPI can be disabled and will need a hardware interrupt to be raised before it will be started again.

The NAPI poll loop is started when the interrupt handler in the driver calls napi_schedule. napi_schedule is actually just a wrapper function defined in a header file which calls down to __napi_schedule.

From net/core/dev.c:

```c
/**
 * __napi_schedule - schedule for receive
 * @n: entry to schedule
 *
 * The entry's receive function will be scheduled to run
 */
void __napi_schedule(struct napi_struct *n)
{
  unsigned long flags;

  local_irq_save(flags);
  ____napi_schedule(&__get_cpu_var(softnet_data), n);
  local_irq_restore(flags);
}
EXPORT_SYMBOL(__napi_schedule);
```

This code is using __get_cpu_var to get the softnet_data structure that is registered to the current CPU. This softnet_data structure and the struct napi_struct structure handed up from the driver are passed into `____napi_schedule`. Wow, that’s a lot of underscores ;)

Let’s take a look at `____napi_schedule`, from net/core/dev.c:

```c
/* Called with irq disabled */
static inline void ____napi_schedule(struct softnet_data *sd,
                                     struct napi_struct *napi)
{
  list_add_tail(&napi->poll_list, &sd->poll_list);
  __raise_softirq_irqoff(NET_RX_SOFTIRQ);
}
```

This code does two important things:

1. The struct napi_struct handed up from the device driver’s interrupt handler code is added to the poll_list attached to the softnet_data structure associated with the current CPU.
2. __raise_softirq_irqoff is used to “raise” (or trigger) a NET_RX_SOFTIRQ softirq. This will cause the net_rx_action registered during the network device subsystem initialization to be executed, if it’s not currently being executed.

As we’ll see shortly, the softirq handler function net_rx_action will call the NAPI poll function to harvest packets.

##### A note about CPU and network data processing

Note that all the code we’ve seen so far to defer work from a hardware interrupt handler to a softirq has been using structures associated with the current CPU.

While the driver’s IRQ handler itself does very little work itself, the softirq handler will execute on the same CPU as the driver’s IRQ handler.

This why setting the CPU a particular IRQ will be handled by is important: that CPU will be used not only to execute the interrupt handler in the driver, but the same CPU will also be used when harvesting packets in a softirq via NAPI.

As we’ll see later, things like Receive Packet Steering can distribute some of this work to other CPUs further up the network stack.

##### Monitoring network data arrival

###### Hardware interrupt requests

> Note: monitoring hardware IRQs does not give a complete picture of packet processing health. Many drivers turn off hardware IRQs while NAPI is running, as we'll see later. It is one important part of your whole monitoring solution.

Check hardware interrupt stats by reading /proc/interrupts.

```sh
$ cat /proc/interrupts
            CPU0       CPU1       CPU2       CPU3
   0:         46          0          0          0 IR-IO-APIC-edge      timer
   1:          3          0          0          0 IR-IO-APIC-edge      i8042
  30: 3361234770          0          0          0 IR-IO-APIC-fasteoi   aacraid
  64:          0          0          0          0 DMAR_MSI-edge      dmar0
  65:          1          0          0          0 IR-PCI-MSI-edge      eth0
  66:  863649703          0          0          0 IR-PCI-MSI-edge      eth0-TxRx-0
  67:  986285573          0          0          0 IR-PCI-MSI-edge      eth0-TxRx-1
  68:         45          0          0          0 IR-PCI-MSI-edge      eth0-TxRx-2
  69:        394          0          0          0 IR-PCI-MSI-edge      eth0-TxRx-3
 NMI:    9729927    4008190    3068645    3375402  Non-maskable interrupts
 LOC: 2913290785 1585321306 1495872829 1803524526  Local timer interrupts
```

You can monitor the statistics in /proc/interrupts to see how the number and rate of hardware interrupts change as packets arrive and to ensure that each RX queue for your NIC is being handled by an appropriate CPU. As we’ll see shortly, this number only tells us how many hardware interrupts have happened, but it is not necessarily a good metric for understanding how much data has been received or processed as many drivers will disable NIC IRQs as part of their contract with the NAPI subsystem. Further, using interrupt coalescing will also affect the statistics gathered from this file. Monitoring this file can help you determine if the interrupt coalescing settings you select are actually working.

To get a more complete picture of your network processing health, you’ll need to monitor /proc/softirqs (as mentioned above) and additional files in /proc that we’ll cover below.

##### Tuning network data arrival

###### Interrupt coalescing

Interrupt coalescing is a method of preventing interrupts from being raised by a device to a CPU until a specific amount of work or number of events are pending.

This can help prevent interrupt storms and can help increase throughput or latency, depending on the settings used. Fewer interrupts generated result in higher throughput, increased latency, and lower CPU usage. More interrupts generated result in the opposite: lower latency, lower throughput, but also increased CPU usage.

Historically, earlier versions of the igb, e1000, and other drivers included support for a parameter called InterruptThrottleRate. This parameter has been replaced in more recent drivers with a generic ethtool function.

Get the current IRQ coalescing settings with ethtool -c.

```sh
$ sudo ethtool -c eth0
Coalesce parameters for eth0:
Adaptive RX: off  TX: off
stats-block-usecs: 0
sample-interval: 0
pkt-rate-low: 0
pkt-rate-high: 0
...
```

ethtool provides a generic interface for setting various coalescing settings. Keep in mind, however, that not every device or driver will support every setting. You should check your driver documentation or driver source code to determine what is, or is not, supported. As per the ethtool documentation: “Anything not implemented by the driver causes these values to be silently ignored.”

One interesting option that some drivers support is “adaptive RX/TX IRQ coalescing.” This option is typically implemented in hardware. The driver usually needs to do some work to inform the NIC that this feature is enabled and some bookkeeping as well (as seen in the igb driver code above).

The result of enabling adaptive RX/TX IRQ coalescing is that interrupt delivery will be adjusted to improve latency when packet rate is low and also improve throughput when packet rate is high.

Enable adaptive RX IRQ coalescing with ethtool -C

```sh
$ sudo ethtool -C eth0 adaptive-rx on
```

You can also use ethtool -C to set several options. Some of the more common options to set are:

* rx-usecs: How many usecs to delay an RX interrupt after a packet arrives.
* rx-frames: Maximum number of data frames to receive before an RX interrupt.
* rx-usecs-irq: How many usecs to delay an RX interrupt while an interrupt is being serviced by the host.
* rx-frames-irq: Maximum number of data frames to receive before an RX interrupt is generated while the system is servicing an interrupt.

And many, many more.

>Reminder that your hardware and driver may only support a subset of the options listed above. You should consult your driver source code and your hardware data sheet for more information on supported coalescing options.

Unfortunately, the options you can set aren’t well documented anywhere except in a header file. Check the source of include/uapi/linux/ethtool.h to find an explanation of each option supported by ethtool (but not necessarily your driver and NIC).

>Note: while interrupt coalescing seems to be a very useful optimization at first glance, the rest of the networking stack internals also come into the fold when attempting to optimize. Interrupt coalescing can be useful in some cases, but you should ensure that the rest of your networking stack is also tuned properly. Simply modifying your coalescing settings alone will likely provide minimal benefit in and of itself.

###### Adjusting IRQ affinities

If your NIC supports RSS / multiqueue or if you are attempting to optimize for data locality, you may wish to use a specific set of CPUs for handling interrupts generated by your NIC.

Setting specific CPUs allows you to segment which CPUs will be used for processing which IRQs. These changes may affect how upper layers operate, as we’ve seen for the networking stack.

If you do decide to adjust your IRQ affinities, you should first check if you running the irqbalance daemon. This daemon tries to automatically balance IRQs to CPUs and it may overwrite your settings. If you are running irqbalance, you should either disable irqbalance or use the --banirq in conjunction with IRQBALANCE_BANNED_CPUS to let irqbalance know that it shouldn’t touch a set of IRQs and CPUs that you want to assign yourself.

Next, you should check the file /proc/interrupts for a list of the IRQ numbers for each network RX queue for your NIC.

Finally, you can adjust the which CPUs each of those IRQs will be handled by modifying /proc/irq/IRQ_NUMBER/smp_affinity for each IRQ number.

You simply write a hexadecimal bitmask to this file to instruct the kernel which CPUs it should use for handling the IRQ.

Example: Set the IRQ affinity for IRQ 8 to CPU 0

```sh
$ sudo bash -c 'echo 1 > /proc/irq/8/smp_affinity'
```
