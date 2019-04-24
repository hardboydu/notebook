# [Monitoring and Tuning the Linux Networking Stack: Receiving Data Part4 Monitoring and Tune](https://blog.packagecloud.io/eng/2016/06/22/monitoring-tuning-linux-networking-stack-receiving-data/)

## Tuning network devices

### Check the number of RX queues being used

If your NIC and the device driver loaded on your system support RSS / multiqueue, you can usually adjust the number of RX queues (also called RX channels), by using `ethtool`.

如果系统上加载的 NIC 和设备驱动程序支持 RSS / 多队列，则通常可以使用`ethtool`调整RX队列（也称为RX通道）的数量。

Check the number of NIC receive queues with `ethtool` :

使用 `ethtool` 检查NIC接收队列的数量：

```sh
$ sudo ethtool -l p1p1
Channel parameters for p1p1:
Pre-set maximums:
RX:             0
TX:             0
Other:          1
Combined:       8
Current hardware settings:
RX:             0
TX:             0
Other:          1
Combined:       8
```

This output is displaying the pre-set maximums (enforced by the driver and the hardware) and the current settings.

此输出显示预设的最大值（由驱动程序和硬件强制执行）和当前设置。

>**Note**: not all device drivers will have support for this operation.
>**注意**：并非所有设备驱动程序都支持此操作。

Error seen if your NIC doesn't support this operation.

如果您的NIC不支持此操作，则会出现错误。

```sh
$ sudo ethtool -l eth0
Channel parameters for eth0:
Cannot get device channel parameters
: Operation not supported
```

This means that your driver has not implemented the `ethtool` `get_channels` operation. This could be because the NIC doesn’t support adjusting the number of queues, doesn’t support RSS / multiqueue, or your driver has not been updated to handle this feature.

这意味着您的驱动程序尚未实现 `ethtool` `get_channels` 操作。这可能是因为 NIC 不支持调整队列数，不支持RSS / 多队列，或者您的驱动程序尚未更新以处理此功能。

### Adjusting the number of RX queues

Once you’ve found the current and maximum queue count, you can adjust the values by using `sudo ethtool -L`.

找到当前和最大队列数后，可以使用 `sudo ethtool -L` 调整值。

>**Note**: some devices and their drivers only support combined queues that are paired for transmit and receive, as in the example in the above section.
>**注意**：某些设备及其驱动程序仅支持为发送和接收配对的组合队列，如上一节中的示例所示。

Set combined NIC transmit and receive queues to `8` with `ethtool -L`:

使用 `ethtool -L` 将组合的NIC发送和接收队列设置为 `8`：

```sh
$ sudo ethtool -L eth0 combined 8
```

If your device and driver support individual settings for RX and TX and you’d like to change only the RX queue count to 8, you would run:

如果您的设备和驱动程序支持 RX 和 TX 的各个设置，并且您只想将 RX 队列计数更改为8，那么您将运行：

Set the number of NIC receive queues to `8` with `ethtool -L`.

使用 `ethtool -L` 将 NIC 接收队列的数量设置为 `8`。

```sh
$ sudo ethtool -L eth0 rx 8
```

>**Note**: making these changes will, for most drivers, take the interface down and then bring it back up; connections to this interface will be interrupted. This may not matter much for a one-time change, though.
>**注意**：对于大多数驱动程序，进行这些更改将使接口关闭，然后将其重新启动；与此接口的连接将被中断。不过，对于一次性改变而言，这可能并不重要。

### Adjusting the size of the RX queues

Some NICs and their drivers also support adjusting the size of the RX queue. Exactly how this works is hardware specific, but luckily ethtool provides a generic way for users to adjust the size. Increasing the size of the RX queue can help prevent network data drops at the NIC during periods where large numbers of data frames are received. Data may still be dropped in software, though, and other tuning is required to reduce or eliminate drops completely.

Check current NIC queue sizes with ethtool -g

```sh
$ sudo ethtool -g p1p1
Ring parameters for p1p1:
Pre-set maximums:
RX:             4096
RX Mini:        0
RX Jumbo:       0
TX:             4096
Current hardware settings:
RX:             256
RX Mini:        0
RX Jumbo:       0
TX:             256
```

the above output indicates that the hardware supports up to 4096 receive and transmit descriptors, but it is currently only using 512.

Increase size of each RX queue to 4096 with ethtool -G

```sh
$ sudo ethtool -G eth0 rx 4096
```

>**Note**: making these changes will, for most drivers, take the interface down and then bring it back up; connections to this interface will be interrupted. This may not matter much for a one-time change, though.

### Adjusting the processing weight of RX queues

Some NICs support the ability to adjust the distribution of network data among the RX queues by setting a weight.

You can configure this if:

* Your NIC supports flow indirection.
* Your driver implements the ethtool functions get_rxfh_indir_size and get_rxfh_indir.
* You are running a new enough version of ethtool that has support for the command line options -x and -X to show and set the indirection table, respectively.

Check the RX flow indirection table with ethtool -x

```sh
$ sudo ethtool -x p1p1
RX flow hash indirection table for p1p1 with 8 RX ring(s):
    0:      0     0     0     0     0     0     0     0
    8:      0     0     0     0     0     0     0     0
   16:      1     1     1     1     1     1     1     1
   24:      1     1     1     1     1     1     1     1
   32:      2     2     2     2     2     2     2     2
   40:      2     2     2     2     2     2     2     2
   48:      3     3     3     3     3     3     3     3
   56:      3     3     3     3     3     3     3     3
   64:      4     4     4     4     4     4     4     4
   72:      4     4     4     4     4     4     4     4
   80:      5     5     5     5     5     5     5     5
   88:      5     5     5     5     5     5     5     5
   96:      6     6     6     6     6     6     6     6
  104:      6     6     6     6     6     6     6     6
  112:      7     7     7     7     7     7     7     7
  120:      7     7     7     7     7     7     7     7
RSS hash key:
Operation not supported
RSS hash function:
    toeplitz: on
    xor: off
    crc32: off
```

This output shows packet hash values on the left, with receive queue 0 and 1 listed. So, a packet which hashes to 2 will be delivered to receive queue 0, while a packet which hashes to 3 will be delivered to receive queue 1.

Example: spread processing evenly between first 2 RX queues

```sh
$ sudo ethtool -X eth0 equal 2
```

If you want to set custom weights to alter the number of packets which hit certain receive queues (and thus CPUs), you can specify those on the command line, as well:

Set custom RX queue weights with ethtool -X

```sh
$ sudo ethtool -X eth0 weight 6 2
```

The above command specifies a weight of 6 for rx queue 0 and 2 for rx queue 1, pushing much more data to be processed on queue 0.

Some NICs will also let you adjust the fields which be used in the hash algorithm, as we’ll see now.

### Adjusting the rx hash fields for network flows

You can use ethtool to adjust the fields that will be used when computing a hash for use with RSS.

Check which fields are used for UDP RX flow hash with ethtool -n.

```sh
$ sudo ethtool -n p1p1 rx-flow-hash udp4
UDP over IPV4 flows use these fields for computing Hash flow key:
IP SA
IP DA

$ sudo ethtool -n p1p1 rx-flow-hash tcp4
TCP over IPV4 flows use these fields for computing Hash flow key:
IP SA
IP DA
L4 bytes 0 & 1 [TCP/UDP src port]
L4 bytes 2 & 3 [TCP/UDP dst port]
```

For eth0, the fields that are used for computing a hash on UDP flows is the IPv4 source and destination addresses. Let’s include the source and destination ports:

Set UDP RX flow hash fields with ethtool -N.

```sh
$ sudo ethtool -N eth0 rx-flow-hash udp4 sdfn
```

The sdfn string is a bit cryptic; check the ethtool man page for an explanation of each letter.

Adjusting the fields to take a hash on is useful, but ntuple filtering is even more useful for finer grained control over which flows will be handled by which RX queue.

### ntuple filtering for steering network flows

Some NICs support a feature known as “ntuple filtering.” This feature allows the user to specify (via ethtool) a set of parameters to use to filter incoming network data in hardware and queue it to a particular RX queue. For example, the user can specify that TCP packets destined to a particular port should be sent to RX queue 1.

On Intel NICs this feature is commonly known as Intel Ethernet Flow Director. Other NIC vendors may have other marketing names for this feature.

As we’ll see later, ntuple filtering is a crucial component of another feature called Accelerated Receive Flow Steering (aRFS), which makes using ntuple much easier if your NIC supports it. aRFS will be covered later.

This feature can be useful if the operational requirements of the system involve maximizing data locality with the hope of increasing CPU cache hit rates when processing network data. For example consider the following configuration for a webserver running on port 80:

* A webserver running on port 80 is pinned to run on CPU 2.
* IRQs for an RX queue are assigned to be processed by CPU 2.
* TCP traffic destined to port 80 is ‘filtered’ with ntuple to CPU 2.
* All incoming traffic to port 80 is then processed by CPU 2 starting at data arrival to the userland program.
* Careful monitoring of the system including cache hit rates and networking stack latency will be needed to determine effectiveness.

As mentioned, ntuple filtering can be configured with ethtool, but first, you’ll need to ensure that this feature is enabled on your device.

Check if ntuple filters are enabled with ethtool -k

```sh
$ sudo ethtool -k p1p1
Features for p1p1:
rx-checksumming: on
tx-checksumming: on
        tx-checksum-ipv4: off [fixed]
        tx-checksum-ip-generic: on
        tx-checksum-ipv6: off [fixed]
        tx-checksum-fcoe-crc: off [fixed]
        tx-checksum-sctp: on
scatter-gather: on
        tx-scatter-gather: on
        tx-scatter-gather-fraglist: off [fixed]
tcp-segmentation-offload: on
        tx-tcp-segmentation: on
        tx-tcp-ecn-segmentation: off [fixed]
        tx-tcp6-segmentation: on
        tx-tcp-mangleid-segmentation: off
udp-fragmentation-offload: off [fixed]
generic-segmentation-offload: on
generic-receive-offload: on
large-receive-offload: off [fixed]
rx-vlan-offload: on
tx-vlan-offload: on
ntuple-filters: off [fixed]
receive-hashing: on
highdma: on [fixed]
rx-vlan-filter: on [fixed]
vlan-challenged: off [fixed]
tx-lockless: off [fixed]
netns-local: off [fixed]
tx-gso-robust: off [fixed]
tx-fcoe-segmentation: off [fixed]
tx-gre-segmentation: on
tx-ipip-segmentation: on
tx-sit-segmentation: on
tx-udp_tnl-segmentation: on
fcoe-mtu: off [fixed]
tx-nocache-copy: off
loopback: off [fixed]
rx-fcs: off [fixed]
rx-all: off
tx-vlan-stag-hw-insert: off [fixed]
rx-vlan-stag-hw-parse: off [fixed]
rx-vlan-stag-filter: off [fixed]
busy-poll: off [fixed]
tx-gre-csum-segmentation: on
tx-udp_tnl-csum-segmentation: on
tx-gso-partial: on
tx-sctp-segmentation: off [fixed]
l2-fwd-offload: off [fixed]
hw-tc-offload: off [fixed]
rx-udp_tunnel-port-offload: off [fixed]
```

As you can see, ntuple-filters are set to off on this device.

Enable ntuple filters with ethtool -K

```sh
$ sudo ethtool -K eth0 ntuple on
```

Once you’ve enabled ntuple filters, or verified that it is enabled, you can check the existing ntuple rules by using ethtool:

Check existing ntuple filters with ethtool -u

```sh
$ sudo ethtool -u eth0
40 RX rings available
Total 0 rules
```

As you can see, this device has no ntuple filter rules. You can add a rule by specifying it on the command line to ethtool. Let’s add a rule to direct all TCP traffic with a destination port of 80 to RX queue 2:

Add ntuple filter to send TCP flows with destination port 80 to RX queue 2

```sh
$ sudo ethtool -U eth0 flow-type tcp4 dst-port 80 action 2
```

You can also use ntuple filtering to drop packets for particular flows at the hardware level. This can be useful for mitigating heavy incoming traffic from specific IP addresses. For more information about configuring ntuple filter rules, see the ethtool man page.

You can usually get statistics about the success (or failure) of your ntuple rules by checking values output from ethtool -S [device name]. For example, on Intel NICs, the statistics fdir_match and fdir_miss calculate the number of matches and misses for your ntuple filtering rules. Consult your device driver source and device data sheet for tracking down statistics counters (if available).