# Linux Performance Tuning : SoftIRQ

## SoftIRQ 分布不均衡

https 压力测试，10000 个会话连接，800M bps，软中断都集中在一个核上

```log
Cpu0  : 18.7%us, 10.3%sy,  0.0%ni, 52.7%id,  0.3%wa,  0.0%hi, 18.0%si,  0.0%st
Cpu1  : 18.0%us, 10.8%sy,  0.0%ni, 71.2%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Cpu2  : 18.4%us,  9.7%sy,  0.0%ni, 71.9%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Cpu3  : 17.5%us,  9.9%sy,  0.0%ni, 72.5%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Cpu4  :  5.7%us,  4.7%sy,  0.0%ni, 89.6%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Cpu5  :  5.0%us,  4.3%sy,  0.0%ni, 90.3%id,  0.0%wa,  0.0%hi,  0.3%si,  0.0%st
Cpu6  :  2.7%us,  2.0%sy,  0.0%ni, 95.3%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Cpu7  :  1.3%us,  1.0%sy,  0.0%ni, 97.7%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Mem:  49393904k total,  6155260k used, 43238644k free,   108248k buffers
Swap: 14843896k total,        0k used, 14843896k free,  3901160k cached

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
 2041 nobody    20   0  187m 142m 1648 R 16.0  0.3   5:22.21 nginx
 2035 nobody    20   0  188m 143m 1648 S 15.6  0.3   5:27.65 nginx
 2037 nobody    20   0  188m 143m 1648 S 15.6  0.3   5:27.38 nginx
 2040 nobody    20   0  188m 144m 1648 S 15.6  0.3   5:30.14 nginx
 2042 nobody    20   0  187m 142m 1648 S 15.6  0.3   6:04.83 nginx
 2043 nobody    20   0  188m 143m 1648 R 15.6  0.3   5:22.39 nginx
 2044 nobody    20   0  188m 143m 1644 S 15.6  0.3   5:31.99 nginx
 2036 nobody    20   0  187m 143m 1648 S 15.3  0.3   5:36.37 nginx
 2038 nobody    20   0  190m 145m 1648 S 15.3  0.3   5:34.34 nginx
 2039 nobody    20   0  191m 146m 1648 S 15.3  0.3   5:51.05 nginx
```

查看内核软中断的统计数据，可以看到，`CPU0` 的软中断都集中在 `NET_RX` 上：

```log
> watch -d -n 1 'cat /proc/softirqs'

                CPU0       CPU1       CPU2       CPU3       CPU4       CPU5       CPU6       CPU7
      HI:          0          0          0          0          0          0          0          0
   TIMER:    2938039    2726051    2680365    2638215    1694905    1447363    1111040     567419
  NET_TX:      84831       5432       4162       4008       3614       3474       3163       2445
  NET_RX:    9411024         41          9          0         35         46         16          0
   BLOCK:       9715         17         10          2      32088         22         16          6
BLOCK_IOPOLL:      0          0          0          0          0          0          0          0
 TASKLET:     345542          0          0          0       5514          9          0          2
   SCHED:     373765     792388     801127     821771     545912     480574     419944     339598
 HRTIMER:        199        706        304        183        303        262        192        120
     RCU:    3198661    2984717    2914805    2855993    1764504    1509337    1140271     584448
```

查看网卡总线地址

```log
> ethtool -i eth5

driver: e1000e
version: 2.3.2-k
firmware-version: 5.7-6
bus-info: 0000:07:00.1
supports-statistics: yes
supports-test: yes
supports-eeprom-access: yes
supports-register-dump: yes
supports-priv-flags: no
```

查看此网卡的内核启动信息

```log
> cat /var/log/dmesg | grep -e "07\:00\.1"

pci 0000:07:00.1: reg 10: [mem 0xdfb40000-0xdfb5ffff]
pci 0000:07:00.1: reg 14: [mem 0xdfb20000-0xdfb3ffff]
pci 0000:07:00.1: reg 18: [io  0x7000-0x701f]
pci 0000:07:00.1: reg 30: [mem 0xdfb00000-0xdfb1ffff pref]
pci 0000:07:00.1: PME# supported from D0 D3hot
pci 0000:07:00.1: PME# disabled
e1000e 0000:07:00.1: PCI INT B -> GSI 17 (level, low) -> IRQ 17
e1000e 0000:07:00.1: setting latency timer to 64
e1000e 0000:07:00.1: Interrupt Throttling Rate (ints/sec) set to dynamic conservative mode
e1000e 0000:07:00.1: irq 125 for MSI/MSI-X
e1000e 0000:07:00.1: eth3: (PCI Express:2.5GT/s:Width x4) 00:16:31:f1:9a:3d
e1000e 0000:07:00.1: eth3: Intel(R) PRO/1000 Network Connection
e1000e 0000:07:00.1: eth3: MAC: 0, PHY: 1, PBA No: D53756-001
```

查看网卡型号

```log
> lspci | grep -e "07\:00\.1"
07:00.1 Ethernet controller: Intel Corporation 82571EB Gigabit Ethernet Controller (rev 06)
```

通过内核信息和网卡型号的数据表得知，此网卡不支持多队列

通过内核信息，得知此网卡的中断号为 125，然后查看此终端号的中断信息：

```log
> cat /proc/interrupts | grep 125
 125:   13032439          0          0          0          0          0          0          0  IR-PCI-MSI-edge      eth5
```

然后查看此中断号的亲缘性配置：

```log
> cat /proc/irq/125/smp_affinity
0f

> cat /proc/irq/125/smp_affinity_list
0-3
```

可以看到此中断的亲缘性的配置是绑定到第一个 CPU 的四个核上，但此网卡不支持多队列，所以，此设置没有什么意义。

### RPS : Receive Packet Steering / RFS : Receive Flow Steering

查看 eth5 的 RPS/RFS 配置

```log
> cat /sys/class/net/eth5/queues/rx-0/rps_cpus
00

>  cat /sys/class/net/eth5/queues/rx-0/rps_flow_cnt
0
```

配置此网卡的 RPS：

```log
> echo 0f > /sys/class/net/eth5/queues/rx-0/rps_cpus
```

这里将此网卡的RPS绑定到第一个 CPU的四个核上，是因为此网卡的PCI是绑定到第一个CPU上的。

配置此网卡的 RFS：

```log
> sysctl net.core.rps_sock_flow_entries=32768
> echo 32768 > /sys/class/net/eth5/queues/rx-0/rps_flow_cnt
```

优化后的结果

```log
top - 11:21:41 up  2:36,  1 user,  load average: 0.00, 0.00, 0.00
Tasks: 194 total,   1 running, 193 sleeping,   0 stopped,   0 zombie
Cpu0  : 14.6%us,  9.2%sy,  0.0%ni, 65.0%id,  0.0%wa,  0.0%hi, 11.2%si,  0.0%st
Cpu1  : 12.1%us,  9.1%sy,  0.0%ni, 75.5%id,  0.0%wa,  0.0%hi,  3.4%si,  0.0%st
Cpu2  : 12.1%us,  8.1%sy,  0.0%ni, 77.5%id,  0.0%wa,  0.0%hi,  2.3%si,  0.0%st
Cpu3  : 10.4%us,  7.0%sy,  0.0%ni, 80.3%id,  0.0%wa,  0.0%hi,  2.3%si,  0.0%st
Cpu4  : 11.0%us,  7.7%sy,  0.0%ni, 77.6%id,  0.0%wa,  0.0%hi,  3.7%si,  0.0%st
Cpu5  : 10.3%us,  7.6%sy,  0.0%ni, 79.1%id,  0.0%wa,  0.0%hi,  3.0%si,  0.0%st
Cpu6  :  9.6%us,  7.3%sy,  0.0%ni, 79.5%id,  0.0%wa,  0.0%hi,  3.6%si,  0.0%st
Cpu7  :  9.4%us,  6.0%sy,  0.0%ni, 81.5%id,  0.0%wa,  0.0%hi,  3.0%si,  0.0%st
Mem:  49393904k total, 15253984k used, 34139920k free,   115340k buffers
Swap: 14843896k total,        0k used, 14843896k free, 10545620k cached

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
 2036 nobody    20   0  133m  88m 1648 S 16.9  0.2  15:23.61 nginx
 2035 nobody    20   0  448m 404m 1648 S 16.6  0.8  14:54.34 nginx
 2040 nobody    20   0  357m 312m 1648 S 16.6  0.6  14:47.45 nginx
 2042 nobody    20   0  447m 402m 1648 S 16.6  0.8  15:59.98 nginx
 2043 nobody    20   0  448m 403m 1648 S 16.6  0.8  14:56.21 nginx
 2044 nobody    20   0  449m 404m 1644 S 16.6  0.8  15:23.09 nginx
 2037 nobody    20   0  448m 403m 1648 S 16.3  0.8  15:11.15 nginx
 2038 nobody    20   0  450m 405m 1648 S 16.3  0.8  15:12.21 nginx
 2039 nobody    20   0  449m 404m 1648 S 16.3  0.8  15:17.14 nginx
 2041 nobody    20   0  445m 400m 1648 S 15.6  0.8  14:40.79 nginx
```

## Reference

* [记录一个软中断问题](https://blog.huoding.com/2013/10/30/296)
