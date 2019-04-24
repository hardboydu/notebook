#### Higher level protocol registration

This blog post will examine UDP, but the TCP protocol handler is registered the same way and at the same time as the UDP protocol handler.

In net/ipv4/af_inet.c, the structure definitions which contain the handler functions for connecting the UDP, TCP , and ICMP protocols to the IP protocol layer can be found. From net/ipv4/af_inet.c:

```c
static const struct net_protocol tcp_protocol = {
        .early_demux    =       tcp_v4_early_demux,
        .handler        =       tcp_v4_rcv,
        .err_handler    =       tcp_v4_err,
        .no_policy      =       1,
        .netns_ok       =       1,
};

static const struct net_protocol udp_protocol = {
        .early_demux =  udp_v4_early_demux,
        .handler =      udp_rcv,
        .err_handler =  udp_err,
        .no_policy =    1,
        .netns_ok =     1,
};

static const struct net_protocol icmp_protocol = {
        .handler =      icmp_rcv,
        .err_handler =  icmp_err,
        .no_policy =    1,
        .netns_ok =     1,
};
```

These structures are registered in the initialization code of the inet address family. From net/ipv4/af_inet.c:

```c
 /*
  *      Add all the base protocols.
  */

 if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0)
         pr_crit("%s: Cannot add ICMP protocol\n", __func__);
 if (inet_add_protocol(&udp_protocol, IPPROTO_UDP) < 0)
         pr_crit("%s: Cannot add UDP protocol\n", __func__);
 if (inet_add_protocol(&tcp_protocol, IPPROTO_TCP) < 0)
         pr_crit("%s: Cannot add TCP protocol\n", __func__);
```

We’re going to be looking at the UDP protocol layer. As seen above, the handler function for UDP is called udp_rcv.

This is the entry point into the UDP layer where the IP layer hands data. Let’s continue our journey there.