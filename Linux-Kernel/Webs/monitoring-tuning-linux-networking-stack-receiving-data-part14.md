### Protocol layer registration

Now that we know how data is delivered to the protocol stacks from the network device subsystem, let’s see how a protocol layer registers itself.

This blog post is going to examine the IP protocol stack as it is a commonly used protocol and will be relevant to most readers.

#### IP protocol layer

The IP protocol layer plugs itself into the ptype_base hash table so that data will be delivered to it from the network device layer described in previous sections.

This happens in the function inet_init from net/ipv4/af_inet.c:

```c
dev_add_pack(&ip_packet_type);
```

This registers the IP packet type structure defined at net/ipv4/af_inet.c:

```c
static struct packet_type ip_packet_type __read_mostly = {
        .type = cpu_to_be16(ETH_P_IP),
        .func = ip_rcv,
};
```

__netif_receive_skb_core calls deliver_skb (as seen in the above section), which calls func (in this case, ip_rcv).

##### ip_rcv

The ip_rcv function is pretty straight-forward at a high level. There are several integrity checks to ensure the data is valid. Statistics counters are bumped, as well.

ip_rcv ends by passing the packet to ip_rcv_finish by way of netfilter. This is done so that any iptables rules that should be matched at the IP protocol layer can take a look at the packet before it continues on.

We can see the code which hands the data over to netfilter at the end of ip_rcv in net/ipv4/ip_input.c:

```c
return NF_HOOK(NFPROTO_IPV4, NF_INET_PRE_ROUTING, skb, dev, NULL, ip_rcv_finish);
```

##### netfilter and iptables

In the interest of brevity (and my RSI), I’ve decided to skip my deep dive into netfilter, iptables, and conntrack.

The short version is that NF_HOOK_THRESH will check if any filters are installed and attempt to return execution back to the IP protocol layer to avoid going deeper into netfilter and anything that hooks in below that like iptables and conntrack.

Keep in mind: if you have numerous or very complex netfilter or iptables rules, those rules will be executed in the softirq context and can lead to latency in your network stack. This may be unavoidable, though, if you need to have a particular set of rules installed.

##### ip_rcv_finish

Once net filter has had a chance to take a look at the data and decide what to do with it, ip_rcv_finish is called. This only happens if the data is not being dropped by netfilter, of course.

ip_rcv_finish begins with an optimization. In order to deliver the packet to proper place, a dst_entry from the routing system needs to be in place. In order to obtain one, the code initially attempts to call the early_demux function from the higher level protocol that this data is destined for.

The early_demux routine is an optimization which attempts to find the dst_entry needed to deliver the packet by checking if a dst_entry is cached on the socket structure.

Here’s what that looks like from net/ipv4/ip_input.c:

```c
if (sysctl_ip_early_demux && !skb_dst(skb) && skb->sk == NULL) {
  const struct net_protocol *ipprot;
  int protocol = iph->protocol;

  ipprot = rcu_dereference(inet_protos[protocol]);
  if (ipprot && ipprot->early_demux) {
    ipprot->early_demux(skb);
    /* must reload iph, skb->head might have changed */
    iph = ip_hdr(skb);
  }
}
```

As you can see above, this code is guarded by a sysctl sysctl_ip_early_demux. By default early_demux is enabled. The next section includes information about how to disable it and why you might want to.

If the optimization is enabled and there is no cached entry (because this is the first packet arriving), the packet will be handed off to the routing system in the kernel where the dst_entry will be computed and assigned.

Once the routing layer completes, statistics counters are updated and the function ends by calling dst_input(skb) which in turn calls the input function pointer on the packet’s dst_entry structure that was affixed by the routing system.

If the packet’s final destination is the local system, the routing system will attach the function ip_local_deliver to the input function pointer in the dst_entry structure on the packet.

###### Tuning: adjusting IP protocol early demux

Disable the early_demux optimization by setting a sysctl.

```sh
$ sudo sysctl -w net.ipv4.ip_early_demux=0
```

The default value is 1; early_demux is enabled.

This sysctl was added as some users saw a ~5% decrease in throughput with the early_demux optimization in some cases.

##### ip_local_deliver

Recall how we saw the following pattern in the IP protocol layer:

1. Calls to ip_rcv do some initial bookkeeping.
2. Packet is handed off to netfilter for processing, with a pointer to a callback to be executed when processing finishes.
3. ip_rcv_finish is the callback which finished processing and continued working toward pushing the packet up the networking stack.

ip_local_deliver has the same pattern. From net/ipv4/ip_input.c:

```c
/*
 *      Deliver IP Packets to the higher protocol layers.
 */
int ip_local_deliver(struct sk_buff *skb)
{
        /*
         *      Reassemble IP fragments.
         */

        if (ip_is_fragment(ip_hdr(skb))) {
                if (ip_defrag(skb, IP_DEFRAG_LOCAL_DELIVER))
                        return 0;
        }

        return NF_HOOK(NFPROTO_IPV4, NF_INET_LOCAL_IN, skb, skb->dev, NULL,
                       ip_local_deliver_finish);
}
```

Once netfilter has had a chance to take a look at the data, ip_local_deliver_finish will be called, assuming the data is not dropped first by netfilter.

##### ip_local_deliver_finish

ip_local_deliver_finish obtains the protocol from the packet, looks up a net_protocol structure registered for that protocol, and calls the function pointed to by handler in the net_protocol structure.

This hands the packet up to the higher level protocol layer.

##### Monitoring: IP protocol layer statistics

Monitor detailed IP protocol statistics by reading /proc/net/snmp.

```sh
$ cat /proc/net/snmp
Ip: Forwarding DefaultTTL InReceives InHdrErrors InAddrErrors ForwDatagrams InUnknownProtos InDiscards InDelivers OutRequests OutDiscards OutNoRoutes ReasmTimeout ReasmReqds ReasmOKs ReasmFails FragOKs FragFails FragCreates
Ip: 1 64 25922988125 0 0 15771700 0 0 25898327616 22789396404 12987882 51 1 10129840 2196520 1 0 0 0
...
```

This file contains statistics for several protocol layers. The IP protocol layer appears first. The first line contains space separate names for each of the corresponding values in the next line.

In the IP protocol layer, you will find statistics counters being bumped. Those counters are referenced by a C enum. All of the valid enum values and the field names they correspond to in /proc/net/snmp can be found in include/uapi/linux/snmp.h:

```c
enum
{
  IPSTATS_MIB_NUM = 0,
/* frequently written fields in fast path, kept in same cache line */
  IPSTATS_MIB_INPKTS,     /* InReceives */
  IPSTATS_MIB_INOCTETS,     /* InOctets */
  IPSTATS_MIB_INDELIVERS,     /* InDelivers */
  IPSTATS_MIB_OUTFORWDATAGRAMS,   /* OutForwDatagrams */
  IPSTATS_MIB_OUTPKTS,      /* OutRequests */
  IPSTATS_MIB_OUTOCTETS,      /* OutOctets */

  /* ... */
```

Monitor extended IP protocol statistics by reading /proc/net/netstat.

```sh
$ cat /proc/net/netstat | grep IpExt
IpExt: InNoRoutes InTruncatedPkts InMcastPkts OutMcastPkts InBcastPkts OutBcastPkts InOctets OutOctets InMcastOctets OutMcastOctets InBcastOctets OutBcastOctets InCsumErrors InNoECTPkts InECT0Pktsu InCEPkts
IpExt: 0 0 0 0 277959 0 14568040307695 32991309088496 0 0 58649349 0 0 0 0 0
```

The format is similar to /proc/net/snmp, except the lines are prefixed with IpExt.

Some interesting statistics:

* InReceives: The total number of IP packets that reached ip_rcv before any data integrity checks.
* InHdrErrors: Total number of IP packets with corrupted headers. The header was too short, too long, non-existent, had the wrong IP protocol version number, etc.
* InAddrErrors: Total number of IP packets where the host was unreachable.
* ForwDatagrams: Total number of IP packets that have been forwarded.
* InUnknownProtos: Total number of IP packets with unknown or unsupported protocol specified in the header.
* InDiscards: Total number of IP packets discarded due to memory allocation failure or checksum failure when packets are trimmed.
* InDelivers: Total number of IP packets successfully delivered to higher protocol layers. Keep in mind that those protocol layers may drop data even if the IP layer does not.
* InCsumErrors: Total number of IP Packets with checksum errors.

Note that each of these is incremented in really specific locations in the IP layer. Code gets moved around from time to time and double counting errors or other accounting bugs can sneak in. If these statistics are important to you, you are strongly encouraged to read the IP protocol layer source code for the metrics that are important to you so you understand when they are (and are not) being incremented.