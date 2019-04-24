### Linux network device subsystem

Now that we’ve taken a look in to how network drivers and softirqs work, let’s see how the Linux network device subsystem is initialized. Then, we can follow the path of a packet starting with its arrival.

#### Initialization of network device subsystem

The network device (netdev) subsystem is initialized in the function net_dev_init. Lots of interesting things happen in this initialization function.

##### Initialization of `struct softnet_data` structures

net_dev_init creates a set of struct softnet_data structures for each CPU on the system. These structures will hold pointers to several important things for processing network data:

* List for NAPI structures to be registered to this CPU.
* A backlog for data processing.
* The processing weight.
* The receive offload structure list.
* Receive packet steering settings.
* And more.

Each of these will be examined in greater detail later as we progress up the stack.

##### Initialization of softirq handlers

net_dev_init registers a transmit and receive softirq handler which will be used to process incoming or outgoing network data. The code for this is pretty straight forward:

```c
static int __init net_dev_init(void)
{
    /* ... */

    open_softirq(NET_TX_SOFTIRQ, net_tx_action);
    open_softirq(NET_RX_SOFTIRQ, net_rx_action);

    /* ... */
}
```

We’ll see soon how the driver’s interrupt handler will “raise” (or trigger) the net_rx_action function registered to the NET_RX_SOFTIRQ softirq.