# [Monitoring and Tuning the Linux Networking Stack: Receiving Data Part1 Overview](https://blog.packagecloud.io/eng/2016/06/22/monitoring-tuning-linux-networking-stack-receiving-data/)

## TL;DR

This blog post explains how computers running the **Linux kernel** receive packets, as well as how to monitor and tune each component of the networking stack as packets flow from the network toward userland programs.

此博客文章解释了运行 **Linux内核** 的计算机如何接收数据包，以及如何在数据包从网络流向用户空程序时监视和调整网络堆栈的每个组件。

UPDATE We’ve released the counterpart to this post: [Monitoring and Tuning the Linux Networking Stack: Sending Data](https://blog.packagecloud.io/eng/2017/02/06/monitoring-tuning-linux-networking-stack-sending-data/).

UPDATE Take a look at [the Illustrated Guide to Monitoring and Tuning the Linux Networking Stack: Receiving Data](https://blog.packagecloud.io/eng/2016/10/11/monitoring-tuning-linux-networking-stack-receiving-data-illustrated/), which adds some diagrams for the information presented below.

It is impossible to tune or monitor the Linux networking stack without reading the source code of the kernel and having a deep understanding of what exactly is happening.

如果不阅读内核的源代码并深入了解究竟发生了什么，就无法调整或监控Linux网络堆栈。

This blog post will hopefully serve as a reference to anyone looking to do this.

这篇博文有望成为任何想要这样做的人的参考。

## Special thanks

Special thanks to the folks at [Private Internet Access](https://privateinternetaccess.com/) who hired us to research this information in conjunction with other network research and who have graciously allowed us to build upon the research and publish this information.

特别感谢 [Private Internet Access](https://privateinternetaccess.com/) 的人们，他们雇用我们与其他网络研究一起研究这些信息，并且慷慨地允许我们在研究的基础上发布并发布这些信息。

The information presented here builds upon the work done for [Private Internet Access](https://privateinternetaccess.com/), which was originally published as a 5 part series starting [here](https://www.privateinternetaccess.com/blog/2016/01/linux-networking-stack-from-the-ground-up-part-1/).

此处提供的信息建立在 [Private Internet Access](https://privateinternetaccess.com/) 的基础上，该互联网最初是作为5部分系列从[这里](https://www.privateinternetaccess.com/blog/2016/01/linux-networking-stack-from-the-ground-up-part-1/)开始发布的。

## General advice on monitoring and tuning the Linux networking stack

UPDATE We’ve released the counterpart to this post: [Monitoring and Tuning the Linux Networking Stack: Sending Data](https://blog.packagecloud.io/eng/2017/02/06/monitoring-tuning-linux-networking-stack-sending-data/).

UPDATE Take a look at [the Illustrated Guide to Monitoring and Tuning the Linux Networking Stack: Receiving Data](https://blog.packagecloud.io/eng/2016/10/11/monitoring-tuning-linux-networking-stack-receiving-data-illustrated/), which adds some diagrams for the information presented below.

The networking stack is complex and there is no one size fits all solution. If the performance and health of your networking is critical to you or your business, you will have no choice but to invest a considerable amount of time, effort, and money into understanding how the various parts of the system interact.

网络协议栈很复杂，不可能适合所有解决方案。如果您的网络的性能和健康状况对您或您的企业至关重要，那么您将别无选择，只能花费大量的时间，精力和金钱来了解系统的各个部分如何相互作用。

Ideally, you should consider measuring packet drops at each layer of the network stack. That way you can determine and narrow down which component needs to be tuned.

理想情况下，您应该考虑在网络协议栈的每一层测量数据包丢弃。这样，您就可以确定并缩小需要调整的组件范围。

This is where, I think, many operators go off track: the assumption is made that a set of `sysctl` settings or `/proc` values can simply be reused wholesale. In some cases, perhaps, but it turns out that the entire system is so nuanced and intertwined that if you desire to have meaningful monitoring or tuning, you must strive to understand how the system functions at a deep level. Otherwise, you can simply use the default settings, which should be good enough until further optimization (and the required investment to deduce those settings) is necessary.

我认为，这就是许多运维人员偏离轨道的原因：假设一组 `sysctl` 设置或 `/proc` 值可以简单地重复使用。在某些情况下，也许可行，但事实证明整个系统是如此微妙和交织在一起，如果您希望进行有意义的监控或调整，您必须努力了解系统如何在深层次运行。否则，您可以简单地使用默认设置，这些设置应该足够好，直到需要进一步优化（以及推断这些设置所需的代价）。

Many of the example settings provided in this blog post are used solely for illustrative purposes and are not a recommendation for or against a certain configuration or default setting. Before adjusting any setting, you should develop a frame of reference around what you need to be monitoring to notice a meaningful change.

本文中提供的许多示例设置仅用于说明目的，并非针对特定配置或默认设置的建议。在调整任何设置之前，您应该围绕着需要监控的内容制定参考框架，以发现有意义的变化。

Adjusting networking settings while connected to the machine over a network is dangerous; you could very easily lock yourself out or completely take out your networking. Do not adjust these settings on production machines; instead make adjustments on new machines and rotate them into production, if possible.

通过网络连接到本机时调整网络设置是危险的；你可以很容易地锁定自己或完全取消你的网络。不要在生产机器上调整这些设置；如果可能的话，改为对新机器进行调整并将其转移到生产环境中。

## Overview

For reference, you may want to have a copy of the device data sheet handy. This post will examine the Intel I350 Ethernet controller, controlled by the `igb` device driver. You can find that data sheet (warning: LARGE PDF) [here for your reference](http://www.intel.com/content/dam/www/public/us/en/documents/datasheets/ethernet-controller-i350-datasheet.pdf).

作为参考，您可能希望获得设备数据表的副本。本文将介绍由`igb`设备驱动程序控制的 Intel I350 以太网控制器。您可以在此处找到该数据表（警告：PDF文件很大）供您[参考](http://www.intel.com/content/dam/www/public/us/en/documents/datasheets/ethernet-controller-i350-datasheet.pdf)。

The high level path a packet takes from arrival to socket receive buffer is as follows:

数据包从到达到套接字接收缓冲区的高级路径如下：

1. Driver is loaded and initialized. <br> 驱动程序已加载并初始化。
2. Packet arrives at the `NIC` from the network. <br> 数据包从网络到达 `NIC`。
3. Packet is copied (via `DMA`) to a ring buffer in kernel memory. <br> 数据包被复制（通过 `DMA`）到内核内存中的环形缓冲区。
4. Hardware interrupt is generated to let the system know a packet is in memory. <br> 生成硬件中断以使系统知道数据包在内存中。
5. Driver calls into NAPI to start a poll loop if one was not running already. <br> 驱动程序调用NAPI以启动轮询循环（如果尚未运行）。
6. `ksoftirqd` processes run on each CPU on the system. They are registered at boot time. The  ksoftirqd processes pull packets off the ring buffer by calling the NAPI `poll` function that the device driver registered during initialization. <br> `ksoftirqd` 进程在系统上的每个CPU上运行。它们在启动时注册。 `ksoftirqd` 进程通过调用设备驱动程序在初始化期间注册的 `NAPI` `poll` 函数将数据包从环形缓冲区中拉出。
7. Memory regions in the ring buffer that have had network data written to them are unmapped. <br> 环形缓冲区中网络数据的内存区域设置成未映射。
8. Data that was DMA’d into memory is passed up the networking layer as an `skb` for more processing. <br> DMA进入内存的数据作为 `skb` 传递到网络层以进行更多处理。
9. Incoming network data frames are distributed among multiple CPUs if packet steering is enabled or if the NIC has multiple receive queues. <br> 如果启用了数据包转向或者NIC具有多个接收队列，则传入的网络数据帧分布在多个CPU之间。
10. Network data frames are handed to the protocol layers from the queues. <br> 网络数据帧从队列传递到协议层。
11. Protocol layers process data. <br> 协议层处理数据。
12. Data is added to receive buffers attached to sockets by protocol layers. <br> 添加数据以接收协议层连接到套接字的缓冲区。

This entire flow will be examined in detail in the following sections.

将在以下部分详细检查整个流程。

The protocol layers examined below are the IP and UDP protocol layers. Much of the information presented will serve as a reference for other protocol layers, as well.

下面检查的协议层是IP和UDP协议层。所提供的大部分信息也将作为其他协议层的参考。

## Detailed Look

> UPDATE We’ve released the counterpart to this post: [Monitoring and Tuning the Linux Networking Stack: Sending Data](https://blog.packagecloud.io/eng/2017/02/06/monitoring-tuning-linux-networking-stack-sending-data/).

> UPDATE Take a look at [the Illustrated Guide to Monitoring and Tuning the Linux Networking Stack: Receiving Data](https://blog.packagecloud.io/eng/2016/10/11/monitoring-tuning-linux-networking-stack-receiving-data-illustrated/), which adds some diagrams for the information presented below.

This blog post will be examining the Linux kernel version 3.13.0 with links to code on GitHub and code snippets throughout this post.

本文将检查Linux内核版本3.13.0，其中包含GitHub上代码的链接以及本文中的代码片段。

Understanding exactly how packets are received in the Linux kernel is very involved. We’ll need to closely examine and understand how a network driver works, so that parts of the network stack later are more clear.

准确了解如何在Linux内核中接收数据包非常复杂。我们需要仔细检查并了解网络驱动程序的工作原理，以便以后网络堆栈的各个部分更加清晰。

This blog post will look at the `igb` network driver. This driver is used for a relatively common server NIC, the Intel Ethernet Controller I350. So, let’s start by understanding how the `igb` network driver works.

这篇博文将介绍 `igb` 网络驱动程序。此驱动程序用于相对常见的服务器 NIC，例如，Intel Ethernet Controller I350。那么，让我们首先了解 `igb` 网络驱动程序的工作原理。