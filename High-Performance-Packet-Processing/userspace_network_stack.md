# Userspace Network Stack

## Reference

* [Why do we use the Linux kernel's TCP stack?](https://jvns.ca/blog/2016/06/30/why-do-we-use-the-linux-kernels-tcp-stack/) 里边提到了一些使用用户态协议栈的一些理由，以及部分实现和参考资源。
* [Why we use the Linux kernel's TCP stack](https://blog.cloudflare.com/why-we-use-the-linux-kernels-tcp-stack/)  这里给出了一些相反的意见，提到了使用系统协议栈的一些优势。
* [User Space Networking Fuels NFV Performance](https://software.intel.com/en-us/blogs/2015/06/12/user-space-networking-fuels-nfv-performance)  列举了一些用户态协议栈，但有点老 2015 年的。

## Open source projects

* [Seastar](http://www.seastar-project.org/)  is an advanced, open-source C++ framework for high-performance server applications on modern hardware. Seastar is used in ScyllaDB, a high-performance NoSQL database compatible with Apache Cassandra. Applications using Seastar can run on Linux or OSv.
    * [fastio/pedis](https://github.com/fastio/pedis)  NoSQL data store using the SEASTAR framework, compatible with Redis，算是Seastar的一个应用。
    * [知乎的一个讨论\(如何评价 ScyllaDB？\)](https://www.zhihu.com/question/35956679)
* [mTCP](http://shader.kaist.edu/mtcp/) [\[github\]](https://github.com/eunyoung14/mtcp) mTCP is a high-performance user-level TCP stack for multicore systems. Scaling the performance of short TCP connections is fundamentally challenging due to inefficiencies in the kernel. mTCP addresses these inefficiencies from the ground up - from packet I/O and TCP connection management all the way to the application interface.

    韩国科学技术院的一个开源项目

    * [Fast User-level TCP Stack on DPDK - DPDK Summit 2016](https://dpdksummit.com/Archive/pdf/2016Asia/DPDK-ChinaAsiaPacificSummit2016-Park-FastUser.pdf)
* [f-stack](http://www.f-stack.org/) [\(github\)](https://github.com/F-Stack/f-stack)  is an open source network framework with high performance based on DPDK， include an user space TCP/IP stack(port FreeBSD 11.0 stable), Posix API(Socket, Epoll, Kqueue), Progamming SDK(Coroutine) and some apps(Nginx, Redis) interface.

    腾讯的项目

    根据腾讯的说法，F-stack采用了单进程单线程模型，这是个问题。

    ```
    Q6:在使用F-Stack库时，其他线程为什么不能调用ff_sendto函数，有什么解决办法吗？

    A6:F-Stack使用了单进程单线程模型，只能在dpdk线程里使用，如果是自己起的线程，调用这些函数会因为curthread为null而挂掉，其他线程可以处理非网络io的事。
    ```

    * [F-Stack的专栏](https://cloud.tencent.com/developer/column/1275)
    * [F-Stack - DPDK Summit 2017 China](https://dpdksummit.com/Archive/pdf/2017Asia/DPDK-China2017-Wang-FStack.pdf)
    * [知乎的一些讨论](https://www.zhihu.com/question/59779624/answer/172519441)
* [lkl/linux](https://github.com/lkl/linux)  LKL (Linux Kernel Library) is aiming to allow reusing the Linux kernel code as extensively as possible with minimal effort and reduced maintenance overhead.

    Examples of how LKL can be used are: creating userspace applications (running on Linux and other operating systems) that can read or write Linux filesystems or can use the **Linux networking stack**, creating kernel drivers for other operating systems that can read Linux filesystems, bootloaders support for reading/writing Linux filesystems, etc.

    With LKL, the kernel code is compiled into an object file that can be directly linked by applications. The API offered by LKL is based on the Linux system call interface.

    LKL is implemented as an architecture port in arch/lkl. It uses host operations defined by the application or a host library (tools/lkl/lib).

    * [User Space TCP - Getting LKL Ready for the Prime Time](https://netdevconf.org/1.2/papers/jerry_chu.pdf)

    貌似是 Google的人在维护

* [OpenOnload®](http://www.openonload.org/) is a high performance network stack from Solarflare that dramatically reduces latency and cpu utilisation, and increases message rate and bandwidth. OpenOnload runs on Linux and supports TCP/UDP/IP network protocols with the standard BSD sockets API, and requires no modifications to applications to use. It achieves performance improvements in part by performing network processing at user-level, bypassing the OS kernel entirely on the data path. Networking performance is improved without sacrificing the security and multiplexing functions that the OS kernel normally provides.



