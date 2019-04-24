### Moving up the network stack with netif_receive_skb

Picking up where we left off with netif_receive_skb, which is called from a few places. The two most common (and also the two we’ve already looked at):

napi_skb_finish if the packet is not going to be merged to an existing GRO’d flow, OR
napi_gro_complete if the protocol layers indicated that it’s time to flush the flow, OR

> Reminder: netif_receive_skb and its descendants are operating in the context of a the softirq processing loop and you'll see the time spent here accounted for as sitime or si with tools like top.

netif_receive_skb begins by first checking a sysctl value to determine if the user has requested receive timestamping before or after a packet hits the backlog queue. If this setting is enabled, the data is timestamped now, prior to it hitting RPS (and the CPU’s associated backlog queue). If this setting is disabled, it will be timestamped after it hits the queue. This can be used to distribute the load of timestamping amongst multiple CPUs if RPS is enabled, but will introduce some delay as a result.

#### Tuning: RX packet timestamping

You can tune when packets will be timestamped after they are received by adjusting a sysctl named  net.core.netdev_tstamp_prequeue:

Disable timestamping for RX packets by adjusting a sysctl

```sh
$ sudo sysctl -w net.core.netdev_tstamp_prequeue=0
```

The default value is 1. Please see the previous section for an explanation as to what this setting means, exactly.

### `netif_receive_skb`

After the timestamping is dealt with, netif_receive_skb operates differently depending on whether or not RPS is enabled. Let’s start with the simpler path first: RPS disabled.

#### Without RPS (default setting)

If RPS is not enabled, __netif_receive_skb is called which does some bookkeeping and then calls __netif_receive_skb_core to move data closer to the protocol stacks.

We’ll see precisely how __netif_receive_skb_core works, but first let’s see how the RPS enabled code path works, as that code will also call __netif_receive_skb_core.

#### With RPS enabled

If RPS is enabled, after the timestamping options mentioned above are dealt with, netif_receive_skb will perform some computations to determine which CPU’s backlog queue should be used. This is done by using the function get_rps_cpu. From net/core/dev.c:

```c
cpu = get_rps_cpu(skb->dev, skb, &rflow);

if (cpu >= 0) {
  ret = enqueue_to_backlog(skb, cpu, &rflow->last_qtail);
  rcu_read_unlock();
  return ret;
}
```

get_rps_cpu will take into account RFS and aRFS settings as described above to ensure the the data gets queued to the desired CPU’s backlog with a call to enqueue_to_backlog.

##### `enqueue_to_backlog`

This function begins by getting a pointer to the remote CPU’s softnet_data structure, which contains a pointer to the input_pkt_queue. Next, the queue length of the input_pkt_queue of the remote CPU is checked. From net/core/dev.c:

```c
qlen = skb_queue_len(&sd->input_pkt_queue);
if (qlen <= netdev_max_backlog && !skb_flow_limit(skb, qlen)) {
```

The length of input_pkt_queue is first compared to netdev_max_backlog. If the queue is longer than this value, the data is dropped. Similarly, the flow limit is checked and if it has been exceeded, the data is dropped. In both cases the drop count on the softnet_data structure is incremented. Note that this is the softnet_data structure of the CPU the data was going to be queued to. Read the section above about /proc/net/softnet_stat to learn how to get the drop count for monitoring purposes.

enqueue_to_backlog is not called in many places. It is called for RPS-enabled packet processing and also from netif_rx. Most drivers should not be using netif_rx and should instead be using netif_receive_skb. If you are not using RPS and your driver is not using netif_rx, increasing the backlog won’t produce any noticeable effect on your system as it is not used.

> Note: You need to check the driver you are using. If it calls netif_receive_skb and you are not using RPS, increasing the netdev_max_backlog will not yield any performance improvement because no data will ever make it to the input_pkt_queue.

Assuming that the input_pkt_queue is small enough and the flow limit (more about this, next) hasn’t been reached (or is disabled), the data can be queued. The logic here is a bit funny, but can be summarized as:

* If the queue is empty: check if NAPI has been started on the remote CPU. If not, check if an IPI is queued to be sent. If not, queue one and start the NAPI processing loop by calling ____napi_schedule. Proceed to queuing the data.
* If the queue is not empty, or the previously described operation has completed, enqueue the data.

The code is a bit tricky with its use of goto, so read it carefully. From net/core/dev.c:

```c
  if (skb_queue_len(&sd->input_pkt_queue)) {
enqueue:
         __skb_queue_tail(&sd->input_pkt_queue, skb);
         input_queue_tail_incr_save(sd, qtail);
         rps_unlock(sd);
         local_irq_restore(flags);
         return NET_RX_SUCCESS;
 }

 /* Schedule NAPI for backlog device
  * We can use non atomic operation since we own the queue lock
  */
 if (!__test_and_set_bit(NAPI_STATE_SCHED, &sd->backlog.state)) {
         if (!rps_ipi_queued(sd))
                 ____napi_schedule(sd, &sd->backlog);
 }
 goto enqueue;
```

##### Flow limits

RPS distributes packet processing load amongst multiple CPUs, but a single large flow can monopolize CPU processing time and starve smaller flows. Flow limits are a feature that can be used to limit the number of packets queued to the backlog for each flow to a certain amount. This can help ensure that smaller flows are processed even though much larger flows are pushing packets in.

The if statement above from net/core/dev.c checks the flow limit with a call to skb_flow_limit:

```c
if (qlen <= netdev_max_backlog && !skb_flow_limit(skb, qlen)) {
```

This code is checking that there is still room in the queue and that the flow limit has not been reached. By default, flow limits are disabled. In order to enable flow limits, you must specify a bitmap (similar to RPS’ bitmap).

##### Monitoring: Monitor drops due to full input_pkt_queue or flow limit

See the section above about monitoring /proc/net/softnet_stat. The dropped field is a counter that gets incremented each time data is dropped instead of queued to a CPU’s input_pkt_queue.

##### Tuning

###### Tuning: Adjusting netdev_max_backlog to prevent drops

Before adjusting this tuning value, see the note in the previous section.

You can help prevent drops in enqueue_to_backlog by increasing the netdev_max_backlog if you are using RPS or if your driver calls netif_rx.

Example: increase backlog to 3000 with sysctl.

```sh
$ sudo sysctl -w net.core.netdev_max_backlog=3000
```

The default value is 1000.

###### Tuning: Adjust the NAPI weight of the backlog poll loop

You can adjust the weight of the backlog’s NAPI poller by setting the net.core.dev_weight sysctl. Adjusting this value determines how much of the overall budget the backlog poll loop can consume (see the section above about adjusting net.core.netdev_budget):

Example: increase the NAPI poll backlog processing loop with sysctl.

```sh
$ sudo sysctl -w net.core.dev_weight=600
```

The default value is 64.

Remember, backlog processing runs in the softirq context similar to the device driver’s registered poll function and will be limited by the overall budget and a time limit, as described in previous sections.

###### Tuning: Enabling flow limits and tuning flow limit hash table size

Set the size of the flow limit table with a sysctl.

```sh
$ sudo sysctl -w net.core.flow_limit_table_len=8192
```

The default value is 4096.

This change only affects newly allocated flow hash tables. So, if you’d like to increase the table size, you should do it before you enable flow limits.

To enable flow limits you should specify a bitmask in /proc/sys/net/core/flow_limit_cpu_bitmap similar to the RPS bitmask which indicates which CPUs have flow limits enabled.

#### backlog queue NAPI poller
The per-CPU backlog queue plugs into NAPI the same way a device driver does. A poll function is provided that is used to process packets from the softirq context. A weight is also provided, just as a device driver would.

This NAPI struct is provided during initialization of the networking system. From net_dev_init in net/core/dev.c:

```c
sd->backlog.poll = process_backlog;
sd->backlog.weight = weight_p;
sd->backlog.gro_list = NULL;
sd->backlog.gro_count = 0;
```

The backlog NAPI structure differs from the device driver NAPI structure in that the weight parameter is adjustable, where as drivers hardcode their NAPI weight to 64. We’ll see in the tuning section below how to adjust the weight using a sysctl.

#### process_backlog

The process_backlog function is a loop which runs until its weight (as described in the previous section) has been consumed or no more data remains on the backlog.

Each piece of data on the backlog queue is removed from the backlog queue and passed on to __netif_receive_skb. The code path once the data hits __netif_receive_skb is the same as explained above for the RPS disabled case. Namely, __netif_receive_skb does some bookkeeping prior to calling __netif_receive_skb_core to pass network data up to the protocol layers.

process_backlog follows the same contract with NAPI that device drivers do, which is: NAPI is disabled if the total weight will not be used. The poller is restarted with the call to ____napi_schedule from enqueue_to_backlog as described above.

The function returns the amount of work done, which net_rx_action (described above) will subtract from the budget (which is adjusted with the net.core.netdev_budget, as described above).

#### __netif_receive_skb_core delivers data to packet taps and protocol layers

__netif_receive_skb_core performs the heavy lifting of delivering the data to protocol stacks. Before it does this, it checks if any packet taps have been installed which are catching all incoming packets. One example of something that does this is the AF_PACKET address family, typically used via the libpcap library.

If such a tap exists, the data is delivered there first then to the protocol layers next.

#### Packet tap delivery

If a packet tap is installed (usually via libpcap), the packet is delivered there with the following code from net/core/dev.c:

```c
list_for_each_entry_rcu(ptype, &ptype_all, list) {
  if (!ptype->dev || ptype->dev == skb->dev) {
    if (pt_prev)
      ret = deliver_skb(skb, pt_prev, orig_dev);
    pt_prev = ptype;
  }
}
```

If you are curious about how the path of the data through pcap, read net/packet/af_packet.c.

#### Protocol layer delivery
Once the taps have been satisfied, __netif_receive_skb_core delivers data to protocol layers. It does this by obtaining the protocol field from the data and iterating across a list of deliver functions registered for that protocol type.

This can be seen in __netif_receive_skb_core in net/core/dev.c:

```c
type = skb->protocol;
list_for_each_entry_rcu(ptype,
                &ptype_base[ntohs(type) & PTYPE_HASH_MASK], list) {
        if (ptype->type == type &&
            (ptype->dev == null_or_dev || ptype->dev == skb->dev ||
             ptype->dev == orig_dev)) {
                if (pt_prev)
                        ret = deliver_skb(skb, pt_prev, orig_dev);
                pt_prev = ptype;
        }
}
```

The ptype_base identifier above is defined as a hash table of lists in net/core/dev.c:

```c
struct list_head ptype_base[PTYPE_HASH_SIZE] __read_mostly;
```

Each protocol layer adds a filter to a list at a given slot in the hash table, computed with a helper function called ptype_head:

```c
static inline struct list_head *ptype_head(const struct packet_type *pt)
{
        if (pt->type == htons(ETH_P_ALL))
                return &ptype_all;
        else
                return &ptype_base[ntohs(pt->type) & PTYPE_HASH_MASK];
}
```

Adding a filter to the list is accomplished with a call to dev_add_pack. That is how protocol layers register themselves for network data delivery for their protocol type.

And now you know how network data gets from the NIC to the protocol layer.