

#### Generic Receive Offloading (GRO)

Generic Receive Offloading (GRO) is a software implementation of a hardware optimization that is known as Large Receive Offloading (LRO).

The main idea behind both methods is that reducing the number of packets passed up the network stack by combining “similar enough” packets together can reduce CPU usage. For example, imagine a case where a large file transfer is occurring and most of the packets contain chunks of data in the file. Instead of sending small packets up the stack one at a time, the incoming packets can be combined into one packet with a huge payload. That packet can then be passed up the stack. This allows the protocol layers to process a single packet’s headers while delivering bigger chunks of data to the user program.

The problem with this sort of optimization is, of course, information loss. If a packet had some important option or flag set, that option or flag could be lost if the packet is coalesced into another. And this is exactly why most people don’t use or encourage the use of LRO. LRO implementations, generally speaking, had very lax rules for coalescing packets.

GRO was introduced as an implementation of LRO in software, but with more strict rules around which packets can be coalesced.

By the way: if you have ever used tcpdump and seen unrealistically large incoming packet sizes, it is most likely because your system has GRO enabled. As you’ll see soon, packet capture taps are inserted further up the stack, after GRO has already happened.

##### Tuning: Adjusting GRO settings with ethtool

You can use ethtool to check if GRO is enabled and also to adjust the setting.

Use ethtool -k to check your GRO settings.

```sh
$ ethtool -k eth0 | grep generic-receive-offload
generic-receive-offload: on
```

As you can see, on this system I have generic-receive-offload set to on.

Use ethtool -K to enable (or disable) GRO.

```sh
$ sudo ethtool -K eth0 gro on
```

> Note: making these changes will, for most drivers, take the interface down and then bring it back up; connections to this interface will be interrupted. This may not matter much for a one-time change, though.

###### napi_gro_receive

The function napi_gro_receive deals processing network data for GRO (if GRO is enabled for the system) and sending the data up the stack toward the protocol layers. Much of this logic is handled in a function called dev_gro_receive.

####### dev_gro_receive

This function begins by checking if GRO is enabled and, if so, preparing to do GRO. In the case where GRO is enabled, a list of GRO offload filters is traversed to allow the higher level protocol stacks to act on a piece of data which is being considered for GRO. This is done so that the protocol layers can let the network device layer know if this packet is part of a network flow that is currently being receive offloaded and handle anything protocol specific that should happen for GRO. For example, the TCP protocol will need to decide if/when to ACK a packet that is being coalesced into an existing packet.

Here’s the code from net/core/dev.c which does this:

```c
list_for_each_entry_rcu(ptype, head, list) {
  if (ptype->type != type || !ptype->callbacks.gro_receive)
    continue;

  skb_set_network_header(skb, skb_gro_offset(skb));
  skb_reset_mac_len(skb);
  NAPI_GRO_CB(skb)->same_flow = 0;
  NAPI_GRO_CB(skb)->flush = 0;
  NAPI_GRO_CB(skb)->free = 0;

  pp = ptype->callbacks.gro_receive(&napi->gro_list, skb);
  break;
}
```

If the protocol layers indicated that it is time to flush the GRO’d packet, that is taken care of next. This happens with a call to napi_gro_complete, which calls a gro_complete callback for the protocol layers and then passes the packet up the stack by calling netif_receive_skb.

Here’s the code from net/core/dev.c which does this:

```c
if (pp) {
  struct sk_buff *nskb = *pp;

  *pp = nskb->next;
  nskb->next = NULL;
  napi_gro_complete(nskb);
  napi->gro_count--;
}
```

Next, if the protocol layers merged this packet to an existing flow, napi_gro_receive simply returns as there’s nothing else to do.

If the packet was not merged and there are fewer than MAX_GRO_SKBS (8) GRO flows on the system, a new entry is added to the gro_list on the NAPI structure for this CPU.

Here’s the code from net/core/dev.c which does this:

```c
if (NAPI_GRO_CB(skb)->flush || napi->gro_count >= MAX_GRO_SKBS)
  goto normal;

napi->gro_count++;
NAPI_GRO_CB(skb)->count = 1;
NAPI_GRO_CB(skb)->age = jiffies;
skb_shinfo(skb)->gso_size = skb_gro_len(skb);
skb->next = napi->gro_list;
napi->gro_list = skb;
ret = GRO_HELD;
```

And that is how the GRO system in the Linux networking stack works.

###### napi_skb_finish

Once dev_gro_receive completes, napi_skb_finish is called which either frees unneeded data structures because a packet has been merged, or calls netif_receive_skb to pass the data up the network stack (because there were already MAX_GRO_SKBS flows being GRO’d).

Next, it’s time for netif_receive_skb to see how data is handed off to the protocol layers. Before this can be examined, we’ll need to take a look at Receive Packet Steering (RPS) first.

