# The Design and Implementation of the Anykernel and Rump Kernels - 01. Introduction

In its classic role, an operating system is a computer program which abstracts the platform it runs on and provides services to application software. Applications in turn provide functionality that the user of the computer system is interested in. For the user to be satisfied, the operating system must therefore support both the platform and application software.

An operating system is understood to consist of the kernel, userspace libraries and utilities, although the exact division between these parts is not definitive in every operating system. The kernel, as the name says, contains the most fundamental routines of the operating system. In addition to low-level platform support and critical functionality such as thread scheduling and IPC, the kernel offers drivers, which abstract an underlying entity. Throughout this dissertation we will use the term driver in an extended sense which encompasses not only hardware device drivers, but additionally for example file system drivers and the TCP/IP network driver.

Major contemporary operating systems follow the monolithic kernel model. Of popular general purpose operating systems, for example Linux, Windows and Mac OS X are regarded as monolithic kernel operating systems. A monolithic kernel means that the entire kernel is executed in a single privileged domain, as opposed to being spread out to multiple independent domains which communicate via message passing. The single privileged domain in turn means that all code in the kernel has full capability to directly control anything running on that particular system. Furthermore, the monolithic kernel does not inherently impose any technical restrictions for the structure of the kernel: a routine may call any other routine in the kernel and access all memory directly. However, like in most disciplines, a well-designed architecture is desirable. Therefore, even a monolithic kernel tends towards structure.

## 1.1 Challenges with the Monolithic Kernel

Despite its widespread popularity, we identified a number of suboptimal characteristics in monolithic kernels which we consider as the motivating problems for this dissertation:

1. **Weak security and robustness**. Since all kernel code runs in the same privileged domain, a single mistake can bring the whole system down. The fragile nature of the monolithic kernel is a long-standing problem to which all monolithic kernel operating systems are vulnerable.
Bugs are an obvious manifestation of the problem, but there are more subtle issues to consider. For instance, widely used file system drivers are vulnerable against untrusted disk images [115]. This vulnerability is acknowledged in the manual page of the mount command for example on Linux: “It is possible for a corrupted file system to cause a crash”. The commonplace act of accessing untrusted removable media such as a USB stick or DVD disk with an in-kernel file system driver opens the entire system to a security vulnerability.
2. **Limited possibilities for code reuse**. The code in a monolithic kernel is viewed to be an all-or-nothing deal due to a belief that everything is intertwined with everything else. This belief implies that features cannot be cherry-picked and put into use in other contexts and that the kernel drivers have value only when they are a part of the monolithic kernel. Examples include file systems [115] and networking [82].
One manifestation of this belief is the reimplementation of kernel drivers for userspace. These reimplementations include TCP/IP stacks [27, 96] and file system drivers [2, 89, 105] <sup>1</sup> for the purposes of research, testing, teaching and application-level drivers. The common approaches for reimplementation are starting from scratch or taking a kernel driver and adjusting the code until the specific driver can run in userspace.
If cherry-picking unmodified drivers were possible, kernel drivers could be directly used at application level. Code reuse would not only save the initial implementation effort, but more importantly it would save from having to maintain the second implementation.
3. **Development and testing is convoluted**. This is a corollary of the previous point: in the general case testing involves booting up the whole operating system for each iteration. Not only is the bootup slow in itself, but when it is combined with the fact that an error may bring the entire system down, development cycles become long and batch mode regression testing is difficult. The difficulty of testing affects how much testing and quality assurance the final software product receives. Users are indirectly impacted: better testing produces a better system.
Due to the complexity and slowness of in-kernel development, a common approach is to implement a prototype in userspace before porting the code to the kernel. For example FFS in BSD [70] and ZFS in Solaris [16] were implemented this way. This approach may bring additional work when the
code is being moved into the kernel, as the support shim in userspace may not have fully emulated all kernel interfaces [70].

> <sup>1</sup> We emphasize that with file systems we do not mean FUSE (Filesystem in Userspace) [106]. FUSE provides a mechanism for attaching a file system driver as a microkernel style server, but does not provide the driver itself. The driver attached by FUSE may be an existing kernel driver which was reimplemented in userspace [2, 3].

## 1.2 Researching Solutions

One option for addressing problems in monolithic kernels is designing a better model and starting from scratch. Some examples of alternative kernel models include the microkernel [9, 43, 45, 64] Exokernel [33] and a partitioned kernel [12, 112]. The problem with starting from scratch is getting to the point of having enough support for external protocols to be a viable alternative for evaluation with real world applications. These external protocols include anything serviced by a driver and range from a networking stack to a POSIX interface. As the complexity of the operating environment and external restrictions grow, it is more and more difficult to start working on an operating system from scratch [92]. For a figure on the amount of code in a modern OS, we look at two subsystems in the Linux 3.3 kernel from March 2012. There are 1,001,218 physical lines of code for file system drivers in the fs subdirectory and 711,150 physical lines of code for networking drivers in the net subdirectory (the latter figure does not include NIC drivers, which are kept elsewhere in the source tree). For the sake of discussion, let us assume that a person who can write 100 lines of bugfree code each day writes all of those drivers. In that case, it will take over 46 years to produce the drivers in those subdirectories.

Even if there are resources to write a set of drivers from scratch, the drivers have not been tested in production in the real world when they are first put out. Studies show that new code contains the most faults [18, 91]. The faults do not exist because the code would have been poorly tested before release, but rather because it is not possible to anticipate every real world condition in a laboratory environment. We argue that real world use is the property that makes an existing driver base valuable, not just the fact that it exists.

## 1.3 Thesis

We claim that it is possible to construct a flexible kernel architecture which solves the challenges listed in Section 1.1, and yet retain the monolithic kernel. Furthermore, it is possible to implement the flexible kernel architecture solely by good programming principles and without introducing levels of indirection which hinder the monolithic kernel’s performance characteristics. We show our claim to be true by an implementation for a BSD-derived open source monolithic kernel OS, NetBSD [87].

We define an anykernel to be an organization of kernel code which allows the kernel’s unmodified drivers to be run in various configurations such as application libraries and microkernel style servers, and also as part of a monolithic kernel. This approach leaves the configuration the driver is used in to be decided at runtime. For example, if maximal performance is required, the driver can be included in a monolithic kernel, but where there is reason to suspect stability or security, the driver can still be used as an isolated, non-privileged server where problems cannot compromised the entire system.

An anykernel can be instantiated into units which virtualize the bare minimum support functionality for kernel drivers. We call these virtualized kernel instances rump kernels since they retain only a part of the original kernel. This minimalistic approach makes rump kernels fast to bootstrap (˜10ms) and introduces only a small memory overhead (˜1MB per instance). The implementation we present hosts rump kernels in unprivileged user processes on a POSIX host. The platform that the rump kernel is hosted on is called the host platform or host.

At runtime, a rump kernel can assume the role of an application library or that of a server. Programs requesting services from rump kernels are called rump kernel clients. Throughout this dissertation we use the shorthand client to denote rump kernel clients. We define three client types.

1. Local: the rump kernel is used in a library capacity. Like with any library, using the rump kernel as a library requires that the application is written to use APIs provided by a rump kernel. The main API for a local client is a system call API with the same call signatures as on a regular NetBSD system.
For example, it is possible to use a kernel file system driver as a library in an application which interprets a file system image.
2. Microkernel: the host routes client requests from regular processes to drivers running in isolated servers. Unmodified application binaries can be used.
For example, it is possible to run a block device driver as a microkernel style server, with the kernel driver outside the privileged domain.
3. Remote: the client and rump kernel are running in different containers (processes) with the client deciding which services to request from the rump kernel and which to request from the host kernel. For example, the client can use the TCP/IP networking services provided by a rump kernel. The kernel and client can exist either on the same host or on different hosts. In this model, both specifically written applications and unmodified applications can use services provided by a rump kernel. The API for specifically written applications is the same as for local clients.
For example, it is possible to use an unmodified Firefox web browser with the TCP/IP code running in a rump kernel server.

Each configuration contributes to solving our motivating problems:

1. **Security and robustness**. When necessary, the use of a rump kernel will allow unmodified kernel drivers to be run as isolated microkernel servers while preserving the user experience. At other times the same driver code can be run in the original fashion as part of the monolithic kernel.
2. **Code reuse**. A rump kernel may be used by an application in the same fashion as any other userlevel library. A local client can call any routine inside the rump kernel.
3. **Development and testing**. The lightweight nature and safety properties of a rump kernel allow for safe testing of kernel code with iteration times in the millisecond range. The remote client model enables the creation of tests using familiar tools.

Our implementation supports rump kernels for file systems [55], networking [54] and device drivers [56]. Both synthetic benchmarks and real world data gathered from a period between 2007 and 2011 are used for the evaluation. We focus our efforts at drivers which do not depend on a physical backend being present. Out of hardware device drivers, support for USB drivers has been implemented and verified. We expect it is possible to support generic unmodified hardware device drivers in rump kernels by using previously published methods [62].

## 1.4 Contributions

The original contributions of this dissertation are as follows:

1. The definition of an anykernel and a rump kernel.
2. Showing that it is possible to implement the above in production quality code and maintain them in a real world monolithic kernel OS.
3. Analysis indicating that the theory is generic and can be extended to other operating systems.

## 1.5 Dissertation Outline

Chapter 2 defines the concept of an anykernel and explains rump kernels. Chapter 3 discusses the implementation and provides microbenchmarks as supporting evidence for implementation decisions. Chapter 4 evaluates the solution. Chapter 5 looks at related work. Chapter 6 provides concluding remarks.

## 1.6 Further Material

### 1.6.1 Source Code

The implementation discussed in this dissertation can be found in source code form from the NetBSD CVS tree as of March 31st 2011 23:59UTC.

NetBSD is an evolving open source project with hundreds of volunteers and continuous change. Any statement we make about NetBSD reflects solely the above timestamp and no other. It is most likely that statements will apply over a wide period of time, but it is up to the interested reader to verify if they apply to earlier or later dates.

It is possible to retrieve the source tree with the following command:

```sh
cvs -d anoncvs@anoncvs.netbsd.org:/cvsroot co -D’20110331 2359UTC’ src
```

Whenever we refer to source file, we implicitly assume the src directory to be a part of the path, i.e. `sys/kern/init_main.c` means `src/sys/kern/init_main.c`.

For simplicity, the above command checks out the entire NetBSD operating system source tree instead of attempting to cherry-pick only the relevant code. The checkout will require approximately 1.1GB of disk space. Most of the code relevant to this document resides under `sys/rump`, but relevant code can be found under other paths as well, such as `tests` and `lib`.

Diffs in Appendix C detail where the above source tree differs from the discussion in this dissertation.

The project was done in small increments in the NetBSD source with almost daily changes. The commits are available for study from repository provided by the NetBSD project, e.g. via the web interface at cvsweb.NetBSD.org.

#### NetBSD Release Model

We use NetBSD release cycle terminology throughout this dissertation. The following contains an explanation of the NetBSD release model. It is a synopsis of the information located at http://www.NetBSD.org/releases/release-map.html.

The main development branch or *HEAD* of NetBSD is known as *NetBSD-current* or, if NetBSD is implied, simply *-current*. The source code used in this dissertation is therefore *-current* from the aforementioned date.

Release branches are created from -current and are known by their major number, for example NetBSD 5. Release branches get bug fixes and minor features, and releases are cut from them at suitable dates. Releases always contain one or more minor numbers, e.g. NetBSD 5.0.1. The first major branch after March 31st is NetBSD 6 and therefore the first release to potentially contain this work is NetBSD 6.0.

A *-current* snapshot contains a kernel API/ABI version. The version is incremented only when an interface changes. The kernel version corresponding to March 31st is 5.99.48. While this version number stands for any -current snapshot between March 9th and April 11th 2011, whenever 5.99.48 is used in this dissertation, it stands for *-current* at 20110331.

#### Code examples

This dissertation includes code examples from the NetBSD source tree. All such examples are copyright of their respective owners and are not public domain. If pertinent, please check the full source for further information about the licensing and copyright of each such example.

### 1.6.2 Manual Pages

Unix-style manual pages for interfaces described in this dissertation are available in Appendix A. The manual pages are taken verbatim from the NetBSD 5.99.48 distribution.

### 1.6.3 Tutorial

Appendix B contains a hands-on tutorial. It walks through various use cases where drivers are virtualized, such as encrypting a file system image using the kernel crypto driver and running applications against virtual userspace TCP/IP stacks. The tutorial uses standard applications and does not require writing code or compiling special binaries.
