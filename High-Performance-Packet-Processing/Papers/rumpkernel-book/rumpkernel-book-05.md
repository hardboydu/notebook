# 5 Short History

This chapter gives a short overview of the history of rump kernels and the events that lead to the current situation. The intention is to provide reasonable amounts of insight into why things evolved like they did. This chapter is written in chronological order on a high level, but we attempt to discuss one subject matter in one go, so there is no timeline in the strict sense.

We will use the late 2006 to early 2007 period as the starting point for the story. Back then, work was done on the puffs userspace file systems framework in NetBSD. This work led to key observation for rump kernels: it is very easy to implement a superficially working file server in userspace because a crash does not mean the end of everything. However, if the file server is supposed to be both performant and robust, things get very complicated even in userspace. In effect, the difference that userspace offers is the immeasurably nicer development environment.

The next logical question to raise was if there is any fundamental reason that kernel file systems cannot be developed in userspace without having to perform back-andforth porting of the drivers between userspace interfaces and kernel interfaces. Given that you are reading this document, the answer is: no.

## 5.1 First Steps

In summer 2007, running NetBSD’s kernel FFS driver as a puffs userspace server was made possible. The guideline for the work was that the FFS driver must remain unmodified, and various shims be built for linking and running the driver. Making the unmodified FFS driver work in userspace took approximately two weeks. A good part of that time was spent fixing bugs in the shim that translated file offsets to backing device block numbers (getpages, putpages & bmap). Translating offsets sounds simple enough, but when you have to take into account the sizes of device blocks, file system blocks and memory pages, along with potentially extending the file, things start getting complicated.

Originally, the working name for rump kernels was sakern, which was officially supposed to mean “standalone kernel”. Unofficially, it was supposed to be a reference to the Swedish language and mean “things”, denoting the slightly messy nature of the shims. The name was changed to RUMP before import to the NetBSD source tree. Later, that “backronym” would be dropped in favor of “rump kernels”. For consistency, we will simply use “rump kernels” throughout this section, instead of altering the nomenclature based on which timeframe we are talking about.

In August 2007, rump kernels were introduced into the NetBSD source tree. At that time, there was support for a number of file systems which could run as puffs servers. Support for running the file drivers as standalone programs was yet not available, since there was no support for the top layers of the kernel. For example, the namei part of the namei+lookup ping pong game was handled by puffs in the host kernel, and rump kernels only supported lookup performed by individual file system drivers.

Also around the same time, the first steps were taken to run NetBSD kernel code on non-NetBSD hosts. Some assumptions about the host being NetBSD were addressed and fixed, and a mental note was made about what was required for supporting non-NetBSD platforms. At that time, building for non-NetBSD required huge amount of manual work, and it would not be until late 2012 when buildrump.sh was introduced that building any given vintage of rump kernels for non-NetBSD platforms would be made easy

## 5.2 Towards Robust and Maintainable

Hacking up something that runs is not difficult. The real challenge is in making things maintainable in a system under constant development. While the shims made it possible to run kernel drivers from one vintage of NetBSD, the shims would eventually go out-of-date and cause compile-time and runtime failures. Since NetBSD kernel code will not go out-of-date with itself, shims were slowly replaced by using NetBSD kernel code directly. Figuring out which interfaces need to implemented as shims, and which interfaces can be used directly from NetBSD sources ended up being a multi-year effort. The general principles on what to reuse and what to rewrite have been more or less clear since 2011 and the core technology has not changed since then.

Another area that needed improvements was multithreading support. Initially, rump kernels did not support locking, synchronization, or anything that interacted with the scheduler. This was fine for file system drivers, because most of them use only a single thread, and simply ignoring most of the aspects of locking was enough. The problem with that approach was of course that it was not good enough to support any given kernel driver, and neither could it be used to exercise the multithreading robustness of file system drivers.

Most of the support for multithreading was added later in 2007. Support for kernel threads and locking and was relatively straightforward to add. The hard part of multithreading support was figuring out how the inverse scheduling model should be handled — normally operating systems pick a thread to schedule on a core, but rump kernels schedule a virtual core for a thread. The original design of the scheduler was done on an airplane napkin in 2009, but coming up with a deadlock free implementation was anything but trivial. It took over a year to successfully implement the design.

## 5.3 Syscall Support

As mentioned, in the beginning rump kernels supported only file servers, and those accessed the kernel drivers directly at the in-kernel virtual file system layer. Hence, there was no need to support the POSIX-style system call interfaces.

However, as application possibilities such as fs-utils were being imagined, the necessity for a syscall-like interface grew. One early attempt at a syscall-like facility was ukfs (or “userspace kernel file system”). It implemented enough of the top half of the kernel for syscall-like pathname-based access to file systems to be possible.

While ukfs worked for file systems, it had issues:

1. The implementation of the top half, especially namei, is complex, so getting it right was non-trivial.
2. The interface was not quite syscall-like, which necessitated writing applications specifically for ukfs instead of being able to use existing applications against rump kernels in a library-like fashion.
3. ukfs was specific to file systems and did not address other subsystems such as the networking stack.

To remedy this, the top levels of the kernel, including a subset of the system call handlers and namei, were added to rump kernels in the beginning of 2008. One mistake was originally made with the introduction of the rump kernel syscall interface. To avoid the implicitly thread-local quality of the errno variable, each system call interface in the rump kernel syscall API took an extra “int *error” parameter for returning the error. This approach made it difficult to use existing applications against the rump kernel syscall API, since the application side required modification. It took almost a year for work to progress far enough for the extra parameter to become a problem. The extra error parameter was removed in January 2009 now making the rump kernel syscall API and ABI equal to the one provided by the NetBSD libc.

As for ukfs, the homegrown pieces were removed, and the interface was implemented on top of rump kernel syscalls. For some years, ukfs was intended to become a portable interface for accessing file systems, i.e. one which does not suffer from type incongruence between the rump kernel and the host (think of the canonical “struct stat” example). However, ukfs is now considered all but an obsolete early experiment on the road of figuring out how to best do things.

## 5.4 Beyond File Systems

From the start, the idea was to support multiple kernel subsystems with rump kernels. This is evident also from the directory structure of the initial 2007 import: file systems were in sys/rump/fs.

It took over a year from the initial import in 2007 for support for another subsystem to appear. The natural choice was networking, since it was required by NFS, and support was added in late 2008. As part of the initial networking support, a virtual network interface to access the network via the host’s Ethernet tap device was added. The idea was to provide network access, not to necessarily be fast. Support for the fast userspace packet I/O frameworks DPDK and netmap emerged in 2013 — not that those frameworks even existed back in 2008.

Beyond file systems and networking, the remaining major source of drivers is for devices. These were the last major set of drivers for which rump kernel support was added. The first steps were taken in mid-2009 with the support of the kernel autoconfiguration framework in rump kernels. Later in 2009, the first experiments with driving actual devices was taken in the form USB devices. Even so, the USB implementation did not touch raw devices, and depended on the USB host controller driver being supported by the host. Support for real device drivers appeared first in 2013 with PCI device support under Xen.

## 5.5 Symbol Isolation

If code is compiled and linked into a non-freestanding environment, there is a risk of symbol collisions. For example, consider hosting rump kernels in userspace: both the rump kernel and host libc provide a function called malloc(). However, the kernel malloc of NetBSD has a different signature, and takes e.g. a flag parameter which specifies whether or not the returned memory should be zeroed. In early versions of rump kernels, kernel and userspace symbols were haphazardly linked together. For example, if the kernel malloc is serviced by the libc implementation, the flags will be ignored and potentially non-zeroed memory will be returned. Amusingly enough, it took a few months before a bug was triggered by malloc not zeroing memory even if it was expected to.

Early attempts at avoiding symbol collisions were done in an ad-hoc fashion. For example, malloc was dealt with by using the --wrap flag in the linker, and for example the kernel valloc() routine was renamed the vnalloc() to avoid a collision with userspace. The exact set of conflicting symbols, however, depend on the platform, so the early attempts were never considered anything more than bandaid.

The real solution came in early 2009 with mass symbol renaming: at build-time, all symbols in a rump kernel are mass renamed to start with the prefix “rump”. While doing so solved the collision problem, the renaming also had a much more profound implication. After symbol renaming was instated, the rump kernel was now a closed namespace; a rump kernel could only access routines within itself or in the hypercall

layer. We now knew for certain that a rump kernel would not block without making a hypercall. This knowledge led to understanding the required scheduling model. Furthermore, hypercalls now defined the portability interface for rump kernels.

## 5.6 Local Clients

One of the things to figure out about the scheduling of rump kernels was where and how to decide which process and thread context the currently executing host thread possesses in the rump kernel. Originally, and heavily influenced by the original file server mode of operation, it was completely up to the client to specify the PID and thread ID. Having the client select the PID allowed for the file server to operate with the same process context as the original operation in the host kernel. Having the same PID meant that errors produced by the fs driver in the rump kernel, e.g. “file system full”, reported the correct host PID that had caused the malfunction.

The downside of the “client decides” approach was that the client had to decide even if it did not want to, i.e. there was no simple way for the client to say “I want a new process for a new set of file descriptors, don’t care about the PID”. The end result of all of this was that emulating things like fork and exit for remote clients would have been close to impossible.

Figuring out a sensible interface for deciding the current context took many years of development and 3 or 4 rewrites of the interface. In 2010, the lwproc set of interfaces was introduced, along with the concepts of implicit and bound threads, marking a big milestone in defining the unique characteristics of rump kernels.

## 5.7 Remote Clients

The first experiments with remote clients were done in early 2009. However, it took over 1.5 years for a revised implementation to appear. Unlike in the case of the scheduler, the delay was not due to the code being complicated to figure out. On the contrary, it took so long because writing RPC code was boring.

While remote clients showed that it is possible to use Unix as a distributed OS without requiring a complete Plan 9-like implementation from scratch, that was not the main goal of remote clients. Support for remote clients made it possible to configure a rump kernel in a more natural way, where a configuration program is run with some parameters, the program performs some syscalls on the kernel, after which the program exits. Before remote clients, any configuration code was run in a local client, usually written manually. This was not a huge problem for file systems, since the configuration is usually just a matter of calling mount(), but for configuring the network stack, not being able to use ifconfig, route and friends was becoming an issue.

## 5.8 Shifting Focus Towards Driver Reuse

In early 2011, rump kernels were declared complete; the basic infrastructure required by portable kernel drivers had been figured after about four years more of more or less full-time work. The declaration also marked the end of most of the work taking place in the NetBSD source tree, since the main architecture of rump kernels had been defined. Given that the goal is to stick to unmodified NetBSD source code, a decent amount of work still goes on directly in the NetBSD tree, but other development loci are emerging.

The first major landmark was the introduction of the buildrump.sh script in late 2012. The importance of buildrump.sh was in that for the first time it was simple to build a given NetBSD source vintage for a non-NetBSD platform. Buildrump.sh was also the first piece of code to be committed onto GitHub to what would eventually become the rumpkernel organization.

The shift toward driver reuse also brought a completely new focus for development. Earlier, when debugging was the main motivation, the main focus was making rump kernels convenient. A good example of this attitude are the implicit threads provided by the rump kernel scheduler: if you do not know about bound threads or do not care to manage them, the default implicit threads will always work correctly, though they do not perform well. With driver reuse, and especially with networking, it started being important to be as performant as possible. Improving performance is still on-going work, but steps such as improving the performance of curlwp have been done in 2014. In fact, the old and new goals conflict slightly, because with implicit threads things will work just fine, but unless you know about bound threads, you will be losing a good deal of potential performance.

## 5.9 We’re not in Kansas Anymore

Up until late 2012, rump kernels were only able to run in userspace processes. Support was mostly limited to NetBSD and Linux at the time, but it’s a small hop from one userspace to another (though the amount of details involved in those small hops is rather dumbfounding). The challenge was to evaluate if rump kernels could run on top of literally anything.

The first non-userspace platform ended up being a web browser, with kernel drivers being compiled to JavaScript instead of the usual assembly. That experiment was both a success and a failure. It was a success because rump kernels ran smoothly in a web browser when compiled with a C→JavaScript compiler. It was a failure because the compiler provided emulation for the POSIX interfaces, and the existing rump kernel hypercalls could be used. In essence, a rump kernel running as JavaScript in a web browser still worked more or less like when running in userspace.

The second non-userspace experiment was undertaken in early 2013. It was a bit more successful in choosing a decidedly non-POSIX platform: the Linux kernel. There were no big surprises in getting rump kernels running on the Linux kernel. One interesting detail was the i386 Linux kernel being compiled with -mregparm=3. That compiler parameter controls how many function arguments are passed in registers, with the rest being passed on the stack. If the rump kernel components are not compiled with the same parameter, the ABIs do not match, and chaos will result. The same concern applies to routines written in assembly. The discrepancy is not insurmountable, merely something that needs to be noted and addressed where necessary. Another useful thing that came out of the Linux kernel experiment was minor improvements to the rumpuser hypercall interface. The hypercall interface has remained the same since then.

Even though the Linux kernel is not a POSIX environment, it still resembles userspace a fair deal: there are scheduled threads, synchronization primitives, etc. To further test the portability of rump kernels, a hypercall implementation for the Xen hypervisor was written in summer 2013. Since the Xen hypervisor is a bare-bones platform without the above mentioned userspace-like synchronization and multiprocessing features, it would test rump kernel portability even more than the Linux kernel did. Granted, the hypercalls were written on top of the Xen MiniOS instead of directly on top of the hypervisor, but MiniOS is there only to provide bootstrapping support, access to paravirtualized devices and MD code for stack switching. All in all, that platform was more or less like running rump kernels directly on top of bare metal.

The Xen platform was also the first to support running a full application stack on top of a rump kernel, including libc and other userspace libraries. Earlier, clients used rump kernels only as libraries, but especially standard applications depended also on the functionality provided by the host system. With additional work, it became possible to run off-the-shelf programs fully on top of rump kernels. The Xen platform was also the first time that a rump kernel was fully in control of the symbol namespace, instead of integrating into an existing one. Notably, not having to integrate into an existing namespace is technically simpler than having to, but it was useful to verify that rump kernels could cope nonetheless.

The first third party rump kernel hypercall implementation was done by Genode Labs for their Genode OS Framework. Support was first released as part of Genode 14.02, which in a highly mnemonic fashion was released in February 2014. Other third parties building OS and OS-like products have since started looking at rump kernels for driver support.

The final frontier, even if traditional operating systems it was the first frontier, was booting and functioning on bare metal. Support for x86 bare metal was written in August 2014. The implication of bare metal was also being able to support a number of virtualization platforms, e.g. KVM, which unlike paravirtualized Xen are bare metal except with alternative I/O devices. That said, the bare metal platform first supported only real I/O devices (e.g. the E1000 NIC instead of a virtio NIC), and virtual ones were added only later when the applicability for the cloud started becoming apparent.

The bare metal and Xen platforms were hosted in separate repositories for a long time, but eventually it became apparent that they consisted mostly of the same bits, and cross-porting changes from one to the other was becoming taxing. The repositories were combined in early 2015 and the resulting repository was called “Rumprun software stack”. The arduous process of merging the diverged codebases was completed in April 2015. The same month the name was changed from “Rumprun software stack” to “Rumprun unikernel”. For one, the change brought synergy with the growing unikernel trend, but the more important implication of using an established term was making it easier to explain Rumprun to newcomers.

## 5.10 Summary

The initial implementation of rump kernels built around file system drivers was minimalistic. Throughout the years the interfaces and implementation were modified to achieve both better client-side usability and maintainability. Introducing features and dependencies was avoided where feasible by critically examining each need. Eventually, stability was achieved.

The second phase was to show the value of rump kernels for providing production quality drivers. This phase was conducted by experimenting with using rump kernels as building blocks on various platforms.

