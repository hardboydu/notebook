# Linux Performance Tuning : Network TCP

Network tunable parameters are usually already tuned to provide high performance. The network stack is also usually designed to respond dynamically to different workloads, providing optimum performance.

Before trying tunable parameters, it can be worthwhile to first understand network usage. This may also identify unnecessary work that can be eliminated, leading to much greater performance wins. Try the workload characterization and static performance tuning methodologies, using the tools in the previous section.

Available tunables vary between versions of an operating system. See their documentation. The sections that follow provide an idea of what may be available and how they are tuned; they should be treated as a starting point to revise based on your workload and environment.

## Linux

Tunable parameters can be viewed and set using the `sysctl(8)` command and written to `/etc/sysctl.conf`. They can also be read and written from the `/proc` file system, under `/proc/sys/net`.

For example, to see what is currently available for TCP, the parameters can be searched for the text tcp from `sysctl(8)` :

```log
# sysctl -a | grep tcp
[...]
net.ipv4.tcp_timestamps = 1
net.ipv4.tcp_window_scaling = 1
net.ipv4.tcp_sack = 1
net.ipv4.tcp_retrans_collapse = 1
net.ipv4.tcp_syn_retries = 5
net.ipv4.tcp_synack_retries = 5
net.ipv4.tcp_max_orphans = 65536
net.ipv4.tcp_max_tw_buckets = 65536
net.ipv4.tcp_keepalive_time = 7200
[...]
```

On this `kernel (3.2.6-3)` there are 63 containing tcp and many more under net., including parameters for IP, Ethernet, routing, and network interfaces.

Examples of specific tuning are in the following sections.

### Socket and TCP Buffers

The maximum socket buffer size for all protocol types, for both reads (rmem_max) and writes (wmem_max), can be set using

```log
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
```

The value is in bytes. This may need to be set to 16 Mbytes or higher to support full-speed 10 GbE connections.

Enabling autotuning of the TCP receive buffer:

```log
tcp_moderate_rcvbuf = 1
```

Setting the auto-tuning parameters for the TCP read and write buffers:

```log
net.ipv4.tcp_rmem = 4096 87380 16777216
net.ipv4.tcp_wmem = 4096 65536 16777216
```

Each has three values: the minimum, default, and maximum number of bytes to use. The size used is autotuned from the default. To improve TCP throughput, try increasing the maximum value. Increasing minimum and default will consume more memory per connection, which may not be necessary.

### TCP Backlog

First backlog queue, for half-open connections:

```log
tcp_max_syn_backlog = 4096
```

Second backlog queue, the listen backlog, for passing connections to `accept()`:

```log
net.core.somaxconn = 1024
```

Both of these may need to be increased from their defaults, for example, to 4,096 and 1,024, or higher, to better handle bursts of load.

### Device Backlog

Increasing the length of the network device backlog queue, per CPU:

```log
net.core.netdev_max_backlog = 10000
```

This may need to be increased, such as to 10,000, for 10 GbE NICs.

### TCP Congestion Control

Linux supports pluggable congestion-control algorithms. Listing those currently available:

```log
# sysctl net.ipv4.tcp_available_congestion_control
net.ipv4.tcp_available_congestion_control = cubic reno
```

Some may be available but not currently loaded. For example, adding htcp:

```log
# modprobe tcp_htcp
# sysctl net.ipv4.tcp_available_congestion_control
net.ipv4.tcp_available_congestion_control = cubic reno htcp
```

The current algorithm may be selected using

```log
net.ipv4.tcp_congestion_control = cubic
```

### TCP Options

Other TCP parameters that may be set include

```log
net.ipv4.tcp_sack = 1
net.ipv4.tcp_fack = 1
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_tw_recycle = 0
```

`SACK` and the `FACK` extensions may improve throughput performance over high-latency networks, at the cost of some CPU.

The `tcp_tw_reuse` tunable allows a `TIME_WAIT` session to be reused when it appears safe to do so. This can allow higher rates of connections between two hosts, such as between a web server and a database, without hitting the 16-bit ephemeral port limit with sessions in `TIME_WAIT`.

`tcp_tw_recycle` is another way to reuse `TIME_WAIT` sessions, although not as safe as `tcp_tw_reuse`.

### Network Interface

The TX queue length may be increased using ifconfig(8), for example:

```log
ifconfig eth0 txqueuelen 10000
```

This may be necessary for 10 GbE NICs. The setting can be added to `/etc/rc.local` so that it is applied during boot.

### Resource Controls

The container groups (cgroups) network priority (net_prio) subsystem can be used to apply a priority to outgoing network traffic, for processes or groups of processes. This can be used to favor high-priority network traffic, such as production load, over low-priority traffic, such as backups or monitoring. The configured priority value is translated to an IP ToS level (or updated scheme that uses the same bits) and included in the packets.
