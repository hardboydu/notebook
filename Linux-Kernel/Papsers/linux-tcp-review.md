# [Implementation of Transmission Control Protocol in Linux](https://wiki.aalto.fi/download/attachments/70789052/linux-tcp-review.pdf)

* Proceedings of Seminar on Network Protocols in Operating Systems

## ABSTRACT

Transmission Control Protocol is the most used transmission layer protocol in the Internet. In addition to reliable and good performance in transmission between two nodes, it provides congestion control mechanism that is a major reason why Internet has not collapsed. Because of its complicated nature, implementations of it can be challenging to understand. This paper describes fundamental details of Transmission Control Protocol implementation in Linux kernel. Focus is on clarifying data structures and segments route through TCP stack.

传输控制协议是Internet中最常用的传输层协议。 除了在两个节点之间传输的可靠和良好性能之外，它还提供拥塞控制机制，这是因特网未崩溃的主要原因。 由于其复杂的性质，理解它的实现可能具有挑战性。本文描述了Linux内核中传输控制协议实现的基本细节。重点是澄清数据结构和通过 TCP 堆栈的路径。

## 1. INTRODUCTION

In May 1974 *Vint Cerf* and *Bob Kahn* published paper where they described an inter-networked protocol, which central control component was Transmission Control Program [3, 2].Later it was divided into modular architecture and in 1981 Transmission Control Protocol (TCP), as it is know today, was specified in RFC 793 [7].

1974年5月，*Vint Cerf* 和 *Bob Kahn* 发表论文，描述了一种网络间协议，其中央控制组件是传输控制程序[3, 2]。后来分为模块化架构和1981年传输控制协议（TCP）， 正如今天所知，在RFC 793 [7]中有详细说明。

Today, TCP is the most used transmission layer protocol in the Internet [4] providing reliable transmission between two hosts through networks [7]. In order to gain good performance for communication, implementations of TCP must be highly optimized. Therefore, TCP is one of the most complicated components in Linux networking stack. In kernel 3.5.4, it consists of over 21000 lines of code under `net/ipv4/` directory (all `tcp*.c` files together), while IPv4 consist of less than 13000 lines of code (all `ip*.c` files in the same directory). This paper explains the most fundamental data structures and operations used in Linux to implement TCP.

今天，TCP 是互联网中最常用的传输层协议[4]，通过网络在两台主机之间提供可靠的传输 [7]。为了获得良好的通信性能，必须高度优化 TCP 的实现。因此，TCP 是 Linux 网络堆栈中最复杂的组件之一。在 `kernel 3.5.4`中，它由 `net/ipv4/` 目录（所有 `tcp*.c` 文件一起）下的超过21000行代码组成，而IPv4由少于13000行代码组成（所有 `ip*.c` 文件在同一目录中）。本文解释了 Linux 中用于实现 TCP 的最基本的数据结构和操作。

TCP provides reliable communication over unreliable network by using acknowledgment messages. In addition to provide resending of the data, TCP also controls its sending rate by using so-called 'windows' to inform the other end how much of data receiver is ready to accept.

TCP 通过使用确认消息在不可靠的网络上提供可靠的通信。除了提供数据重发之外，TCP还通过使用所谓的 "窗口(windows)" 来控制其发送速率，以通知另一端有多少数据接收器已准备好接受。

As parts of the TCP code are dependent on network layer implementation, the scope of this paper is limited to IPv4 implementation as it is currently supported and used more widely than IPv6. However, most of the code is shared between IPv4 and IPv6, and `tcp_ipv6.c` is the only file related to TCP under net/ipv6/. In addition, TCP congestion control will be handled in a separate paper, so it will be handled very briefly. If other assumptions is made it is mentioned in the beginning of the related section.

由于 TCP 代码的一部分依赖于网络层实现，因此本文的范围仅限于 IPv4 实现，因为它目前支持和使用比 IPv6 更广泛。但是，大多数代码在 IPv4 和 IPv6 之间共享，而 `tcp_ipv6.c` 是 `net/ipv6/` 下唯一与TCP相关的文件。此外，TCP 拥塞控制将在另一篇文章中处理，因此将非常简单地处理。如果做出其他假设，则在相关章节的开头提及。

Paper structure will be following: First section **"Overview of implementation"** will cover most important files and basic data structures used by TCP (`sk_buff`, `tcp_sock`), how data is stored inside these structures and how different queues are implemented, what timers TCP is using and how TCP sockets are kept in memory. Then socket initialization and data flows through TCP is discussed. Section **"Algorithms, optimizations and options"** will handle logic of TCP state machine, explain what is TCP fast path and discuss about socket options that can be used to modify behavour of TCP.

论文的结构如下：第一部分 **“实现概述”** 将涵盖TCP（`sk_buff`，`tcp_sock`）使用的最重要的文件和基本数据结构，数据如何存储在这些结构中以及如何实现不同的队列，什么是TCP定时器，正在使用以及TCP套接字如何保存在内存中。 然后讨论套接字初始化和通过TCP的数据流。**“算法，优化和选项”** 部分将处理TCP状态机的逻辑，解释什么是 TCP 快速路径，并讨论可用于修改 TCP 行为的套接字选项。

## 2. OVERVIEW OF IMPLEMENTATION

In this section basic operation of TCP in Linux will be explained. It covers the most fundamental files and data structures used by TCP, as well as functions used when we are sending to or receiving from network.

在本节中，将解释Linux中TCP的基本操作。它涵盖了 TCP 使用的最基本的文件和数据结构，以及我们发送到网络或从网络接收时使用的功能。

The most important files of implementation are listed in table 1. In addion to `net/ipv4/` where most TCP files are located, there are also few headers located in `include/net/` and `include/linux/` directories.

最重要的实现文件列在 表1 中。除了大多数TCP文件所在的 `net/ipv4/` 之外，`include/net/` 和 `include/linux/` 目录中也有很少的头文件。

Table 1: Most important files of TCP

|File                    | Description
|------------------------|--------------------------------------------------------------------------
|`tcp.c`                 | Layer between user and kernel space <br> 用户和内核空间之间的层
|`tcp_output.c`          | TCP output engine. Handles outgoing data and passes it to network layer  <br> TCP输出引擎。处理传出数据并将其传递到网络层
|`tcp_input.c`           | TCP input engine. Handles incoming segments.  <br> TCP输入引擎。处理传入的细分。
|`tcp_timer.c`           | TCP timer handling  <br> TCP计时器处理
|`tcp_ipv4.c`            | IPv4 related functions, receives segments from network layer  <br> IPv4相关功能，从网络层接收段
|`tcp_cong.c`            | Congestion control handler, includes also **TCP Reno** implementation <br> 拥塞控制处理程序，还包括 **TCP Reno** 实现
|`tcp [veno|vegas|..].c` | Congestion control algorithms, named as `tcp_NAME.c`  <br> 拥塞控制算法，命名为 `tcp_NAME.c`
|`tcp.h`                 | Main header files of TCP. struct tcp sock is defined here. Note that there is `tcp.h` in both `include/net/` and `include/linux/`  <br> TCP的主头文件。 struct tcp sock在这里定义。 请注意，`include/net/` 和 `include/linux/` 中都有`tcp.h`

### 2.1 Data structures

Data structures are crucial sections of any software in order of performance and re-usability. As TCP is a highly optimized and remarkably complex entirety, robust understanding of data structures used is mandatory for mastering the implementation.

数据结构是任何软件的关键部分，按性能和可重用性顺序排列。 由于TCP是一个高度优化且非常复杂的整体，因此必须对所使用的数据结构有深刻理解，以掌握实现。

#### 2.1.1 `struct sk_buff`

`struct sk_buff` (located in `include/linux/skbuff.h`) is used widely in the network implementation in Linux kernel. It is a socket buffer containing one slice of the data we are sending or receiving. In Figure 1 we see how data is stored inside structure. Data is hold in the continuous memory area surrounded by empty spaces, head and tail rooms. By having these empty spaces more data can be added to before or after current data without needing to copy or move it, and minimize risk of need to allocate more memory. However, if the data does not fit to space allocated, it will be fragmented to smaller segments and saved inside `struct skb_shared_info` that lives at the end of data (at the end pointer).

`struct sk_buff`（位于`include/linux/skbuff.h`）在 Linux 内核的网络实现中被广泛使用。它是一个套接字缓冲区，包含我们发送或接收的一个数据片。在图1中，我们看到数据如何存储在结构中。数据保存在由空的空间，头部和尾部空间包围的连续存储区域中。通过使这些空白空间可以在当前数据之前或之后添加更多数据，而无需复制或移动它，并最大限度地降低分配更多内存的风险。 但是，如果数据不适合分配的空间，它将被分段为较小的段并保存在生成数据末尾的 `struct skb_shared_info` 中（在结束指针处）。

![1](linux-tcp-review/01.PNG)

All the data cannot be held in one large segment in the memory, and therefore we must have several socket buffers to be able to handle major amounts of data and to resend data segment that was lost during transmission to receiver. Because of that need of network data queues is obvious. In Linux these queues are implemented as ring-lists of `sk_buff` structures (Figure 2). Each socket buffer has a pointer to the previous and next buffers. There is also special data structure to represent the whole list, known as struct `sk_buff` head. More detailed  information about the data queues is in section 2.1.3.

所有数据都不能保存在内存中的一个大段中，因此我们必须有几个套接字缓冲区才能处理大量数据并重新发送在传输到接收器期间丢失的数据段。由于网络数据队列的需要是显而易见的。在Linux中，这些队列被实现为 `sk_buff` 结构的环形链表（图2）。每个套接字缓冲区都有一个指向前一个和下一个缓冲区的指针，还有特殊的数据结构来表示整个列表，称为struct`sk_buff`头。 有关数据队列的更多详细信息，请参见第2.1.3节。

![2](linux-tcp-review/02.PNG)

In addition data pointers, `sk_buff` also has pointer to owning socket, device from where data is arriving from or leaving by and several other members. All the members are documented in `skbuff.h`.

此外，数据指针 `sk_buff` 还具有指向拥有套接字的指针，数据来自或离开的设备以及其他几个成员。所有成员都记录在`skbuff.h`中。

#### 2.1.2 `struct tcp_sock`

`struct tcp_sock` (`include/linux/tcp.h`) is the core structure for TCP. It contains all the information and packet buffers for certain TCP connection. Figure 3 visualizes how this structure is implemented in Linux. Inside `tcp_sock` there is a few other, more general type of sockets. As a next, more general type of socket is always first member of socket type, can a pointer to socket be type-casted to other type of socket. This allows us to make general functions that handles with, for example, `struct sock`, even in reality pointer would be also a valid pointer to `struct tcp_sock`. Also depending on the type of the socket can different structure be as a first member of the certain socket. For example, as UDP is connection-less protocol, first member of `struct udp_sock` is `struct inet_sock`, but for `struct tcp_sock` first member must be `struct inet_connection_sock`, as it provides us features needed with connection-oriented protocols.

`struct tcp_sock`（`include/linux/tcp.h`）是TCP的核心结构。它包含特定 TCP 连接的所有信息和数据包缓冲区。图3 显示了如何在 Linux 中实现此结构。在`tcp_sock` 里面还有一些其他更通用的套接字。作为下一个，更一般类型的 socket 始终是 socket 类型的第一个成员，可以将指向套接字的指针类型转换为其他类型的套接字。 这允许我们使用例如 `struct sock` 来处理一般函数，即使实际上指针也是 `struct tcp_sock` 的有效指针。 另外，根据 socket 的类型，可以将不同的结构作为特定 socket 的第一个成员。 例如，由于 UDP 是无连接协议，`struct udp_sock` 的第一个成员是`struct inet_sock`，但是对于`struct tcp_sock`，第一个成员必须是`struct inet_connection_sock`，因为它为我们提供了面向连接的协议所需的功能。

From Figure 3 it can be seen that TCP has many packet queues. There is receive queue, backlog queue and write queue (not in figure) under `struct sock`, and prequeue and out-of-order queue under `tcp_sock`. These different queues and their functions are explained in detail in section 2.1.3.

从图3可以看出TCP具有许多分组队列。 在`struct sock`下有接收队列，backlog 队列和写队列（不在图中），在`tcp_sock`下有prequeue和out-order-queue队列。这些不同的队列及其功能将在2.1.3节中详细说明。

`struct inet_connection_sock` (`include/net/inet_connection_sock`) is a socket type one level down from the tcp_sock. It contains information about protocol congestion state, protocol timers and the accept queue.

`struct inet_connection_sock`（`include/net/inet_connection_sock`）是一个从 `tcp_sock` 向下一级的套接字类型。它包含有关协议拥塞状态，协议计时器和接受队列的信息。

Next type of socket is `struct inet_sock` (`include/net/inet_sock.h`). It has information about connection ports and IP addresses.

下一种类型的 socket 是`struct inet_sock`（`include/net/inet_sock.h`）。它包含有关连接端口和 IP 地址的信息。

Finally there is general socket type `struct sock`. It contains two of TCP’s three receive queues, `sk_receive_queue` and `sk_backlog`, and also queue for sent data, used with retransmission.

最后有一般套接字类型`struct sock`。 它包含两个TCP的三个接收队列，`sk_receive_queue`和`sk_backlog`，以及用于重发的已发送数据的队列。

#### 2.1.3 Data queues

There is four queues implemented for incoming data: receive queue, prequeue, backlog queue and out-of-order queue. In normal case when segment arrives and user is not waiting for the data, segment is processed immediately and the data is copied to the receive buffer. If socket is blocked as user is waiting for data, segment is copied to prequeue and user task is interrupted to handle the segment. If user is handling segments at the same time when we receive a new one, it will be put to the backlog queue, and user context will handle the segment after it has handled all earlier segments. If the segment handler detects out-of-order segment, it will be put to the out-of-order queue and copied to the receive buffer after the missing segments have been arrived.

为传入数据实现了四个队列：接收队列，预队列，积压队列和无序队列。 在正常情况下，当段到达并且用户没有等待数据时，立即处理段并将数据复制到接收缓冲器。 如果套接字在用户等待数据时被阻止，则段被复制到预队列，并且用户任务被中断以处理该段。 如果用户在收到新段时同时处理段，则会将其放入积压队列，并且用户上下文将在处理完所有早期段后处理该段。 如果段处理程序检测到无序段，则在丢失的段到达后，它将被置于无序队列并复制到接收缓冲区。

Figure 4 visualizes use of receive, pre- and backlog-queues.

#### 2.1.4 Hash tables

Sockets are located in kernel’s hash table from where them are fetched when a new segment arrives or socket is otherwise needed. Main hash structure is `struct inet_hashinfo` (`include/net/inet_hashtables.h`), and TCP uses it as a type of global variable `tcp_hashinfo` located in `net/ipv4/tcp_ipv4.c`.

套接字位于内核的哈希表中，当新段到达或者需要套接字时，它们将从中获取。 主哈希结构是 `struct inet_hashinfo`（`include/net/inet_hashtables.h`），TCP使用它作为一种位于 `net/ipv4/tcp_ipv4.c` 中的全局变量 `tcp_hashinfo` 。

`struct inet_hashinfo` 有三个主要的哈希表：一个用于具有完整标识的套接字，一个用于绑定，一个用于监听套接字。 除此之外，完整标识哈希表分为两部分：`TIME_WAIT` 状态的套接字和其他部分 。

As hash tables are more general and not only TCP specific part of kernel, this paper will not go into logic behind these more deeply.

#### 2.1.5 Other data structures

There is also other data structures that must be known in order to understand how TCP stack works. `struct proto` (`include/net/sock.h`) is a general structure presenting transmission layer to socket layer. It contains function pointers that are set to TCP specific functions in `net/ipv4/tcp_ipv4.c`, and applications function calls are eventually, through other layers, mapped to these.

还必须知道其他数据结构才能理解 TCP 堆栈的工作原理。`struct proto`（`include/net/sock.h`）是一个呈现传输层到套接字层的通用结构。它包含在 `net/ipv4/tcp_ipv4.c` 中设置为 TCP 特定函数的函数指针，并且应用程序函数调用最终通过其他层映射到这些函数 。

`struct tcp_info` is used to pass information about socket state to user. Structure will be filled in function `tcp_get_info()`. It contains values for connection state (Listen, Established, etc), congestion control state (Open, Disorder, CWR, Recovery, Lost), receiver and sender MSS, rtt and various counters.

`struct tcp_info` 用于将有关套接字状态的信息传递给用户。结构将填入函数 `tcp_get_info()`。 它包含连接状态（侦听，已建立等），拥塞控制状态（打开，无序，CWR，恢复，丢失），接收器和发送器MSS，rtt和各种计数器的值。

### 2.2 TCP timers

To provide reliable communication with good performance, TCP uses four timers: Retransmit timer, delayed ack timer, keep-alive timer and zero window prope timer. Retransmit, delayed ack and zero window probe timers are located in `struct inet_connection_sock`, and keep-alive timer can be found from `struct sock` (Figure 3).

Although there is dedicated timer handling file `net/ipv4/tcp_timer.c`, timers are set and reset in several locations in the code as a result of events that occur.

### 2.3 Socket initialization

TCP functions available to socket layer are set to previously explained (section 2.1.5) `struct proto` in `tcp_ipv4.c`. This structure will be held in `struct inet_protosw` in `af_inet.c`, from where it will be fetched and set to `sk->sk_prot` when user does `socket()` call. During socket creation in the function `inet_create()` function `sk->sk_prot->init()` will be called, which points to `tcp_v4_init_sock()`. From there the real initialization function `tcp_init_sock()` will be called.

Address-family independent initialization of TCP socket occurs in `tcp_init_sock()` (`net/ipv4/tcp.c`). The function will be called when socket is created with `socket()` system call. In that function fields of structure `tcp_sock` are initialized to default values. Also out of order queue will be initialized with `skb_queue_head_init()`, prequeue with `tcp_prequeue_init()`, and TCP timers with `tcp_init_xmit_timers()`. At this point, state of the socket is set to `TCP_CLOSE`.

#### 2.3.1 Connection socket

Next step to do when user wants to create a new TCP connection to other host is to call `connect()`. In the case of TCP, it maps to function `inet_stream_connect()`, from where `sk->sk_prot->connect()` is called. It maps to TCP function `tcp_v4_connect()`.

`tcp_v4_connect()` validates end host address by using `ip_route_connect()` function. After that `inet_hash_connect()` will be called. `inet_hash_connect()` selects source port for our socket, if not set, and adds the socket to hash tables. If everything is fine, initial sequence number will be fetched from `secure_tcp_sequence_number()` and the socket is passed to `tcp_connect()`.

`tcp_connect()` calls first `tcp_connect_init()`, that will initialize parameters used with TCP connection, such as maximum segment size (MSS) and TCP window size. After that `tcp_connect()` will reserve memory for socket buffer, add buffer to sockets write queue and passes buffer to function `tcp_transmit_skb()`, that builds TCP headers and passes data to network layer. Before returning `tcp_connect()` will start retransmission timer for the `SYN` packet. When `SYN-ACK` packet is received, state of socket is modified to `ESTABLISHED`, `ACK` is sent and communication between nodes may begin.

#### 2.3.2 Listening socket

Creation of listening socket should be done in two phases. Firstly, `bind()` must be called to pick up port what will be listened to, and secondly, `listen()` must be called.

`bind()` maps to `inet_bind()`. Function validates port number and socket, and then tries to bind the wanted port. If everything goes fine function returns 0, otherwise error code indicating problem will be returned.

Function call `listen()` will become to function `inet_listen()`. `inet_listen()` performs a few sanity checks, and then calls function `inet_csk_listen_start()`, which allocates memory for socket accept queue, sets socket state to `TCP_LISTEN` and adds socket to TCP hash table to wait incoming connections

### 2.4 Data flow through TCP in kernel

Knowing the rough route of incoming and outgoing segments through the layer is on of the most important part of TCP implementation to understand. In this section a roughly picture of it in most common cases will be given. Handling of all the cases is not appropriate and possible under the limits of this paper.

In this section it is assumed that DMA (`CONFIG_NET_DMA`) is not in use. It would be used to offload copying of data to dedicated hardware, thus saving CPU time. [1]

#### 2.4.1 From the network

Figure 5 shows us a simplified summary about incoming data flow through TCP in Linux kernel.

In the case of IPv4, TCP receives incoming data from network layer in `tcp_v4_rcv()` (`net/ipv4/tcp_ipv4.c`). The function checks if packet is meant for us and finds the matching TCP socket from the hash table using IPs and ports as the keys. If the socket is not owned by user (user context is not handling the data), we first try to put the packet to prequeue. Prequeuing is possible only when user context is waiting for the data. If prequeuing was not possible, we pass the data to `tcp_v4_do_rcv()`. There socket state is checked. If state is `TCP_ESTABLISHED`, data is passed to `tcp_rcv_established()`, and copied to receive queue. Otherwise buffer is passed to `tcp_rcv_state_process()`, where all the other states will be handled.

If the socket was not owned by user in function tcp_v4_rcv(), data will be copied to the backlog queue of the socket.

When user tries to read data from the socket (`tcp_recvmsg()`), queues must be processed in order. First receive queue, then data from prequeue will be waited, and when the process ready to release socket, packets from backlog will be copied to the receive queue. Handling of the queues must be preserved in order to ensure that data will be copied to user buffer in the same order as it was sent.

Figure 4 visualizes the overall queuing process.

#### 2.4.2 From the user

Figure 6 shows us a simplified summary about outgoing data flow through TCP in Linux kernel.

When user-level application writes data to TCP socket, first function that will be called is `tcp_sendmsg()`. It calculates size goal for segments and then creates `sk_buff` buffers of calculated size from the data, pushes buffers to write queue and notifies TCP output engine of new segments. Segments will go through TCP output engine and end up to `tcp_transmit_skb()`.

`tcp_write_xmit` takes care that segment is sent only when it is allowed to. If congestion control, sender window or Nagle’s algorithm prevent sending, the data will not go forward. Also retransmission timers will be set from `tcp_write_xmit`, and after data send, congestion window will be validated referring to RFC 2861 [5].

`tcp_transmit_skb()` builds up TCP headers and passes data to network layer by calling function `queue_xmit()` found from `struct inet_connection_sock` from member `icsk_af_ops`.

## 3. ALGORITHMS, OPTIMIZATIONS AND OPTIONS

This section will go through a few crucial parts of implementation and clarify why these are important features to have and to work properly in a modern TCP implementation.

### 3.1 TCP state machine

There is several state machines implemented in Linux TCP. Probably most known TCP state machine is connection state machine, introduced in RFC 793 [7]. Figure 3.1 presents states and transitions implemented in kernel. In addition to connection state machine TCP has own state machine for congestion control.

The most important function in TCP state handling is `tcp_rcv_state_process()`, as it handles all the states except `ESTABLISHED` and `TIME_WAIT`. `TIME_WAIT` is handled in `tcp_v4_rcv()`, and state `ESTABLISHED` in `tcp_rcv_established()`.

As stated, `TIME_WAIT` is handled in `tcp_v4_rcv()`. Depending on return value of `tcp_timewait_state_process`, packet will be discarded, acked or processed again with a new socket (if the packet was SYN initializing a new connection). Implementation of function is very clean and easy to follow.

### 3.2 Congestion control

At first TCP did not have specific congestion control algorithms, and due to misbehaving TCP implementations Internet had first ’congestion collapse’ in October 1988. Investigation on that leaded to first TCP congestion control algorithms described by *Jacobson* in 1988 [6]. However, it took almost 10 years before official RFC based on *Jacobson’s* research on congestion control algorithms came out [8].

Main file for TCP congestion control in Linux is tcp_cong.c. It contains congestion control algorithm database, functions to register and to active algorithm and implementation of TCP_Reno. Congestion algorithm is linked to rest of the TCP stack by using `struct tcp_congestion_ops`, that has function pointers to currently used congestion control algorithm implementation. Pointer to the structure is found in `struct inet_connection_sock` (member `icsk_ca_ops`), see it at Figure 3.

Important fields for congestion control are located in `struct tcp_sock` (see section 2.1.2). Being the most important variable, member snd cwnd presents sending congestion window and rcv wnd current receiver window. Congestion window is the estimated amount of data that can be in the network without data being lost. If too many bytes is sent to the network, TCP is not allowed to send more data before an acknowledgment from the other end is received.

As congestion control is out of scoop of this paper, it will not be investigated more deeply.

### 3.3 TCP fast path

Normal, so-called slow path is a comprehensive processing route for segments. It handles special header flags and out-of-order segments, but because of that, it is also requiring heavy processing that is not needed in normal cases during data transmission.

Fast path is an TCP optimization used in `tcp_rcv_established()` to skip unnecessary packet handling in common cases when deep packet inspection is not needed. By default fast path is disabled, and before fast path can be enabled, four things must be verifed: The out-of-order queue must be empty, receive window can not be zero, memory must be available and urgent pointer has not been received. This four cases are checked in function `tcp_fast_path_check()`, and if all cases pass, will fast path be enabled in certain cases. Even after
fast path is enabled, segment must be verified to be accepted to fast path.

TCP uses technique known as header prediction to verify segment to fast path. Header prediction allows TCP input machine to compare certain bits in the incoming segment’s header to check if the segment is valid for fast path. Header prediction ensures that there are no special conditions requiring additional processing. Because of this fast path is easily turned off by setting header prediction bits to zero, causing header prediction to fail always. In addition to pass header prediction, segment received must be next in order to be accepted to fast path.

### 3.4 Socket options

Behaving of TCP can be affected by modifying its parameters through socket options. System-wide settings can be accessed by files in the directory `/proc/sys/net/ipv4`. Options affecting to only certain TCP connection (socket) can be set by using `getsockopt()`/`setsockopt()` system calls.

System-wide configurations related to TCP are mapped to kernel in `net/ipv4/sysctl_net_ipv4.c`. All implemented options are listed in `include/net/tcp.h`. In Linux 3.5.3, there are 44 of them.

Setting and getting socket options is handled in kernel in `do_tcp_setsockopt()` and `do_tcp_getsockopt()` (`net/ipv4/tcp.c`). In Linux 3.5.3, there are 22 options, defined in `include/linux/tcp.h`.

## 4. CONCLUSION

Implementation of TCP in Linux is a complex and highly optimized to gain as high performance as possible. Because of that it is also time-consuming process to get into code level in kernel and understand TCP details. This paper described the most fundamental components of the TCP implementation in Linux 3.5.3 kernel.

## 5. REFERENCES

1. [1] Linux kernel options documentation. http://lxr.linux.no/#linux+v3.5.3/drivers/dma/Kconfig.
2. [2] V. Cerf, Y. Dalal, and C. Sunshine. Specification of Internet Transmission Control Program. RFC 675, Dec. 1974.
3. [3] V. G. Cerf and R. E. Khan. A protocol for packet network intercommunication. IEEE TRANSACTIONS ON COMMUNICATIONS, 22:637–648, 1974.
4. [4] K. Cho, K. Fukuda, H. Esaki, and A. Kato. Observing slow crustal movement in residential user traffic. In Proceedings of the 2008 ACM CoNEXT Conference, CoNEXT ’08, pages 12:1–12:12, New York, NY, USA, 2008. ACM.
5. [5] M. Handley, J. Padhye, and S. Floyd. TCP Congestion Window Validation. RFC 2861 (Experimental), June 2000.
6. [6] V. Jacobson. Congestion avoidance and control. SIGCOMM Comput. Commun. Rev., 18(4):314–329, Aug. 1988.
7. [7] J. Postel. RFC 793: Transmission control protocol, Sept. 1981.
8. [8] W. Stevens. TCP Slow Start, Congestion Avoidance, Fast Retransmit, and Fast Recovery Algorithms. RFC 2001 (Proposed Standard), Jan. 1997. Obsoleted by RFC 2581.
