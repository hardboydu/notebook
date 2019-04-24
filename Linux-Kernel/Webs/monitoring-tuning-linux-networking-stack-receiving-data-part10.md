
#### Network data processing begins

Once the softirq code determines that a softirq is pending, begins processing, and executes net_rx_action, network data processing begins.

Let’s take a look at portions of the net_rx_action processing loop to understand how it works, which pieces are tunable, and what can be monitored.

##### `net_rx_action` processing loop

net_rx_action begins the processing of packets from the memory the packets were DMA’d into by the device.

The function iterates through the list of NAPI structures that are queued for the current CPU, dequeuing each structure, and operating on it.

The processing loop bounds the amount of work and execution time that can be consumed by the registered NAPI poll functions. It does this in two ways:

1. By keeping track of a work budget (which can be adjusted), and
2. Checking the elapsed time

From net/core/dev.c:

```c
  while (!list_empty(&sd->poll_list)) {
    struct napi_struct *n;
    int work, weight;

    /* If softirq window is exhausted then punt.
     * Allow this to run for 2 jiffies since which will allow
     * an average latency of 1.5/HZ.
     */
    if (unlikely(budget <= 0 || time_after_eq(jiffies, time_limit)))
      goto softnet_break;
```

This is how the kernel prevents packet processing from consuming the entire CPU. The budget above is the total available budget that will be spent among each of the available NAPI structures registered to this CPU.

This is another reason why multiqueue NICs should have the IRQ affinity carefully tuned. Recall that the CPU which handles the IRQ from the device will be the CPU where the softirq handler will execute and, as a result, will also be the CPU where the above loop and budget computation runs.

Systems with multiple NICs each with multiple queues can end up in a situation where multiple NAPI structs are registered to the same CPU. Data processing for all NAPI structs on the same CPU spend from the same budget.

If you don’t have enough CPUs to distribute your NIC’s IRQs, you can consider increasing the net_rx_action budget to allow for more packet processing for each CPU. Increasing the budget will increase CPU usage (specifically sitime or si in top or other programs), but should reduce latency as data will be processed more promptly.

> Note: the CPU will still be bounded by a time limit of 2 jiffies, regardless of the assigned budget.

##### NAPI poll function and weight

Recall that network device drivers use netif_napi_add for registering poll function. As we saw earlier in this post, the igb driver has a piece of code like this:

```c
  /* initialize NAPI */
  netif_napi_add(adapter->netdev, &q_vector->napi, igb_poll, 64);
```

This registers a NAPI structure with a hardcoded weight of 64. We’ll see now how this is used in the  net_rx_action processing loop.

From net/core/dev.c:

```c
weight = n->weight;

work = 0;
if (test_bit(NAPI_STATE_SCHED, &n->state)) {
        work = n->poll(n, weight);
        trace_napi_poll(n);
}

WARN_ON_ONCE(work > weight);

budget -= work;
```

This code obtains the weight which was registered to the NAPI struct (64 in the above driver code) and passes it into the poll function which was also registered to the NAPI struct (igb_poll in the above code).

The poll function returns the number of data frames that were processed. This amount is saved above as work, which is then subtracted from the overall budget.

So, assuming:

1. You are using a weight of 64 from your driver (all drivers were hardcoded with this value in Linux 3.13.0), and
2. You have your budget set to the default of 300

Your system would stop processing data when either:

1. The igb_poll function was called at most 5 times (less if no data to process as we’ll see next), OR
2. At least 2 jiffies of time have elapsed.

##### The NAPI / network device driver contract

One important piece of information about the contract between the NAPI subsystem and device drivers which has not been mentioned yet are the requirements around shutting down NAPI.

This part of the contract is as follows:

* If a driver’s poll function consumes its entire weight (which is hardcoded to 64) it must NOT modify NAPI state. The net_rx_action loop will take over.
* If a driver’s poll function does NOT consume its entire weight, it must disable NAPI. NAPI will be re-enabled next time an IRQ is received and the driver’s IRQ handler calls napi_schedule.

We’ll see how net_rx_action deals with the first part of that contract now. Next, the poll function is examined, we’ll see how the second part of that contract is handled.

##### Finishing the net_rx_action loop

The net_rx_action processing loop finishes up with one last section of code that deals with the first part of the NAPI contract explained in the previous section. From net/core/dev.c:

```c
/* Drivers must not modify the NAPI state if they
 * consume the entire weight.  In such cases this code
 * still "owns" the NAPI instance and therefore can
 * move the instance around on the list at-will.
 */
if (unlikely(work == weight)) {
  if (unlikely(napi_disable_pending(n))) {
    local_irq_enable();
    napi_complete(n);
    local_irq_disable();
  } else {
    if (n->gro_list) {
      /* flush too old packets
       * If HZ < 1000, flush all packets.
       */
      local_irq_enable();
      napi_gro_flush(n, HZ >= 1000);
      local_irq_disable();
    }
    list_move_tail(&n->poll_list, &sd->poll_list);
  }
}
```

If the entire work is consumed, there are two cases that net_rx_action handles:

* The network device should be shutdown (e.g. because the user ran ifconfig eth0 down),
* If the device is not being shutdown, check if there’s a generic receive offload (GRO) list. If the timer tick rate is >= 1000, all GRO’d network flows that were recently updated will be flushed. We’ll dig into GRO in detail later. Move the NAPI structure to the end of the list for this CPU so the next iteration of the loop will get the next NAPI structure registered.

And that is how the packet processing loop invokes the driver’s registered poll function to process packets. As we’ll see shortly, the poll function will harvest network data and send it up the stack to be processed.

##### Exiting the loop when limits are reached

The net_rx_action loop will exit when either:

* The poll list registered for this CPU has no more NAPI structures (!list_empty(&sd->poll_list)), or
* The remaining budget is <= 0, or
* The time limit of 2 jiffies has been reached

Here’s this code we saw earlier again:

```c
/* If softirq window is exhausted then punt.
 * Allow this to run for 2 jiffies since which will allow
 * an average latency of 1.5/HZ.
 */
if (unlikely(budget <= 0 || time_after_eq(jiffies, time_limit)))
  goto softnet_break;
```

If you follow the softnet_break label you stumble upon something interesting. From net/core/dev.c:

```c
softnet_break:
  sd->time_squeeze++;
  __raise_softirq_irqoff(NET_RX_SOFTIRQ);
  goto out;
```

The struct softnet_data structure has some statistics incremented and the softirq NET_RX_SOFTIRQ is shut down. The time_squeeze field is a measure of the number of times net_rx_action had more work to do but either the budget was exhausted or the time limit was reached before it could be completed. This is a tremendously useful counter for understanding bottlenecks in network processing. We’ll see shortly how to monitor this value. The NET_RX_SOFTIRQ is disabled to free up processing time for other tasks. This makes sense as this small stub of code is only executed when more work could have been done, but we don’t want to monopolize the CPU.

Execution is then transferred to the out label. Execution can also make it to the out label if there were no more NAPI structures to process, in other words, there is more budget than there is network activity and all the drivers have shut NAPI off and there is nothing left for net_rx_action to do.

The out section does one important thing before returning from net_rx_action: it calls net_rps_action_and_irq_enable. This function serves an important purpose if Receive Packet Steering is enabled; it wakes up remote CPUs to start processing network data.

We’ll see more about how RPS works later. For now, let’s see how to monitor the health of the net_rx_action processing loop and move on to the inner working of NAPI poll functions so we can progress up the network stack.

##### NAPI poll

Recall in previous sections that device drivers allocate a region of memory for the device to perform DMA to incoming packets. Just as it is the responsibility of the driver to allocate those regions, it is also the responsibility of the driver to unmap those regions, harvest the data, and send it up the network stack.

Let’s take a look at how the igb driver does this to get an idea of how this works in practice.

###### igb_poll

At long last, we can finally examine our friend igb_poll. It turns out the code for igb_poll is deceptively simple. Let’s take a look. From drivers/net/ethernet/intel/igb/igb_main.c:

```c
/**
 *  igb_poll - NAPI Rx polling callback
 *  @napi: napi polling structure
 *  @budget: count of how many packets we should handle
 **/
static int igb_poll(struct napi_struct *napi, int budget)
{
        struct igb_q_vector *q_vector = container_of(napi,
                                                     struct igb_q_vector,
                                                     napi);
        bool clean_complete = true;

#ifdef CONFIG_IGB_DCA
        if (q_vector->adapter->flags & IGB_FLAG_DCA_ENABLED)
                igb_update_dca(q_vector);
#endif

        /* ... */

        if (q_vector->rx.ring)
                clean_complete &= igb_clean_rx_irq(q_vector, budget);

        /* If all work not completed, return budget and keep polling */
        if (!clean_complete)
                return budget;

        /* If not enough Rx work done, exit the polling mode */
        napi_complete(napi);
        igb_ring_irq_enable(q_vector);

        return 0;
}
```

This code does a few interesting things:

* If Direct Cache Access (DCA) support is enabled in the kernel, the CPU cache is warmed so that accesses to the RX ring will hit CPU cache. You can read more about DCA in the Extras section at the end of this blog post.
* Next, igb_clean_rx_irq is called which does the heavy lifting, as we’ll see next.
* Next, clean_complete is checked to determine if there was still more work that could have been done. If so, the budget (remember, this was hardcoded to 64) is returned. As we saw earlier, net_rx_action will move this NAPI structure to the end of the poll list.
* Otherwise, the driver turns off NAPI by calling napi_complete and re-enables interrupts by calling igb_ring_irq_enable. The next interrupt that arrives will re-enable NAPI.

Let’s see how igb_clean_rx_irq sends network data up the stack.

###### igb_clean_rx_irq

The igb_clean_rx_irq function is a loop which processes one packet at a time until the budget is reached or no additional data is left to process.

The loop in this function does a few important things:

* Allocates additional buffers for receiving data as used buffers are cleaned out. Additional buffers are added IGB_RX_BUFFER_WRITE (16) at a time.
* Fetch a buffer from the RX queue and store it in an skb structure.
* Check if the buffer is an “End of Packet” buffer. If so, continue processing. Otherwise, continue fetching additional buffers from the RX queue, adding them to the skb. This is necessary if a received data frame is larger than the buffer size.
* Verify that the layout and headers of the data are correct.
* The number of bytes processed statistic counter is increased by skb->len.
* Set the hash, checksum, timestamp, VLAN id, and protocol fields of the skb. The hash, checksum, timestamp, and VLAN id are provided by the hardware. If the hardware is signaling a checksum error, the csum_error statistic is incremented. If the checksum succeeded and the data is UDP or TCP data, the skb is marked as CHECKSUM_UNNECESSARY. If the checksum failed, the protocol stacks are left to deal with this packet. The protocol is computed with a call to eth_type_trans and stored in the skb struct.
* The constructed skb is handed up the network stack with a call to napi_gro_receive.
* The number of packets processed statistics counter is incremented.
* The loop continues until the number of packets processed reaches the budget.

Once the loop terminates, the function assigns statistics counters for rx packets and bytes processed.

Now it’s time to take two detours prior to proceeding up the network stack. First, let’s see how to monitor and tune the network subsystem’s softirqs. Next, let’s talk about Generic Receive Offloading (GRO). After that, the rest of the networking stack will make more sense as we enter napi_gro_receive.

##### Monitoring network data processing

###### `/proc/net/softnet_stat`

As seen in the previous section, net_rx_action increments a statistic when exiting the net_rx_action loop and when additional work could have been done, but either the budget or the time limit for the softirq was hit. This statistic is tracked as part of the struct softnet_data associated with the CPU.

These statistics are output to a file in proc: /proc/net/softnet_stat for which there is, unfortunately, very little documentation. The fields in the file in proc are not labeled and could change between kernel releases.

In Linux 3.13.0, you can find which values map to which field in /proc/net/softnet_stat by reading the kernel source. From net/core/net-procfs.c:

```c
  seq_printf(seq,
       "%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n",
       sd->processed, sd->dropped, sd->time_squeeze, 0,
       0, 0, 0, 0, /* was fastroute */
       sd->cpu_collision, sd->received_rps, flow_limit_count);
```

Many of these statistics have confusing names and are incremented in places where you might not expect. An explanation of when and where each of these is incremented will be provided as the network stack is examined. Since the squeeze_time statistic was seen in net_rx_action, I thought it made sense to document this file now.

Monitor network data processing statistics by reading /proc/net/softnet_stat.

```sh
$ cat /proc/net/softnet_stat
6dcad223 00000000 00000001 00000000 00000000 00000000 00000000 00000000 00000000 00000000
6f0e1565 00000000 00000002 00000000 00000000 00000000 00000000 00000000 00000000 00000000
660774ec 00000000 00000003 00000000 00000000 00000000 00000000 00000000 00000000 00000000
61c99331 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
6794b1b3 00000000 00000005 00000000 00000000 00000000 00000000 00000000 00000000 00000000
6488cb92 00000000 00000001 00000000 00000000 00000000 00000000 00000000 00000000 00000000
```

Important details about /proc/net/softnet_stat:

* Each line of /proc/net/softnet_stat corresponds to a struct softnet_data structure, of which there is 1 per CPU.
* The values are separated by a single space and are displayed in hexadecimal
* The first value, sd->processed, is the number of network frames processed. This can be more than the total number of network frames received if you are using ethernet bonding. There are cases where the ethernet bonding driver will trigger network data to be re-processed, which would increment the sd->processed count more than once for the same packet.
* The second value, sd->dropped, is the number of network frames dropped because there was no room on the processing queue. More on this later.
* The third value, sd->time_squeeze, is (as we saw) the number of times the net_rx_action loop terminated because the budget was consumed or the time limit was reached, but more work could have been. Increasing the budget as explained earlier can help reduce this.
* The next 5 values are always 0.
* The ninth value, sd->cpu_collision, is a count of the number of times a collision occurred when trying to obtain a device lock when transmitting packets. This article is about receive, so this statistic will not be seen below.
* The tenth value, sd->received_rps, is a count of the number of times this CPU has been woken up to process packets via an Inter-processor Interrupt
* The last value, flow_limit_count, is a count of the number of times the flow limit has been reached. Flow limiting is an optional Receive Packet Steering feature that will be examined shortly.

If you decide to monitor this file and graph the results, you must be extremely careful that the ordering of these fields hasn’t changed and that the meaning of each field has been preserved. You will need to read the kernel source to verify this.

##### Tuning network data processing

###### Adjusting the net_rx_action budget

You can adjust the net_rx_action budget, which determines how much packet processing can be spent among all NAPI structures registered to a CPU by setting a sysctl value named net.core.netdev_budget.

Example: set the overall packet processing budget to 600.

```sh
$ sudo sysctl -w net.core.netdev_budget=600
```

You may also want to write this setting to your /etc/sysctl.conf file so that changes persist between reboots.

The default value on Linux 3.13.0 is 300.