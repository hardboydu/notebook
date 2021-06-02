# 6 Conclusions

A codebase’s real value lies not in the fact that it exists, but in that it has been proven and hardened “out there”. The purpose of this work is harnessing the power and stability of existing in-kernel drivers.

We defined an anykernel to be an organization of kernel code which allows the kernel’s unmodified drivers to be run in various configurations such as libraries, servers, small standalone operating systems, and also in the original monolithic kernel. We showed by means of a production quality implementation that the NetBSD monolithic kernel could be turned into an anykernel with relatively simple modifications. The key point is retaining the battle-hardened nature of the drivers.

An anykernel can be instantiated into units which include the minimum support functionality for running kernel driver components. These units are called rump kernels since they provide only a part of the original kernel’s features. Features not provided by rump kernels include for example a thread scheduler, virtual memory and the capability to execute binaries. These omissions make rump kernels straightforward to integrate into any platform that has approximately one megabyte or more of RAM and ROM. Alternatively, entirely new software stacks built around rump kernels are possible to execute with relative ease, as we explored with the Rumprun unikernel.

As the parting thoughts, we remind ourselves of why operating systems have the role they currently have, and what we should do to move forward.

The birth of timesharing operating systems took place over 50 years ago, an era from which we draw even the current concept of the operating system. Back then, hardware was simple, scarce and sacred, and those attributes drove the development of the concepts of the system and the users. In the modern world, computing is done in a multitude of ways, and the case for the all-encompassing operating system has watered down.

The most revered feature of the modern operating system is support for running existing applications. We can harness that power through rump kernels. Therefore, there is no reason to cram a traditional operating system into every problem space. Instead, we should choose the most suitable software stack based on the problem at hand.

# References

1. Slirp, the PPP/SLIP-on-terminal emulator. URL http://slirp. sourceforge.net/.
2. Thomas E. Anderson, Brian N. Bershad, Edward D. Lazowska, and Henry M. Levy. 1992. Scheduler Activations: Effective Kernel Support for the User-level Management of Parallelism. ACM Transactions on Computer Systems 10, no. 1, pages 53–79.
3. Paul Barham, Boris Dragovic, Keir Fraser, Steven Hand, Tim Harris, Alex Ho, Rolf Neugebauer, Ian Pratt, and Andrew Warfield. 2003. Xen and the Art of Virtualization. In: Proceedings of the ACM Symposium on Operating Systems Principles, pages 164–177.
4. Andrew Baumann, Paul Barham, Pierre-Evariste Dagand, Tim Harris, Rebecca Isaacs, Simon Peter, Timothy Roscoe, Adrian Schupbach, and Akhilesh ¨Singhania. 2009. The Multikernel: A new OS architecture for scalable multicore systems. In: Proceedings of the ACM Symposium on Operating Systems Principles, pages 29–44.
5. Fabrice Bellard. 2005. QEMU, a Fast and Portable Dynamic Translator. In: Proceedings of the USENIX Annual Technical Conference, FREENIX Track, pages 41–46.
6. Brian N. Bershad, Thomas E. Anderson, Edward D. Lazowska, and Henry M. Levy. 1990. Lightweight Remote Procedure Call. ACM Transactions on Computer Systems 8, pages 37–55.
7. Jeff Bonwick. 1994. The Slab Allocator: An Object-Caching Kernel Memory Allocator. In: Proceedings of the USENIX Summer Technical Conference, pages 87–98.
8. Martin Campbell-Kelly. 1998. Programming the EDSAC: Early Programming Activity at the University of Cambridge. IEEE Annals of the History of Computing 20, no. 4, pages 46–67.
9. Adam M. Costello and George Varghese. 1995. Redesigning the BSD Callout and Timer Facilities. Technical Report WUCS-95-23, Washington University.
10. Charles D. Cranor. 1998. Design and Implementation of the UVM Virtual Memory System. Ph.D. thesis, Washington University.
11. Steven E. Czerwinski, Ben Y. Zhao, Todd D. Hodes, Anthony D. Joseph, and Randy H. Katz. 1999. An Architecture for a Secure Service Discovery Service. In: Proceedings of the 5th MobiCom, pages 24–35.
12. Luke Deller and Gernot Heiser. 1999. Linking Programs in a Single Address Space. In: Proceedings of the USENIX Annual Technical Conference, pages 283–294.
13. Mathieu Desnoyers. 2009. Low-Impact Operating System Tracing. Ph.D. thesis, Ecole Polytechnique de Montr´eal.
14. Mathieu Desnoyers, Paul E. McKenney, Alan S. Stern, Michel R. Dagenais, and Jonathan Walpole. 2012. User-Level Implementations of Read-Copy Update. IEEE Transactions on Parallel and Distributed Systems 23, no. 2, pages 375–382.
15. Edsger W. Dijkstra. 2001. My recollections of operating system design. URL http://www.cs.utexas.edu/users/EWD/ewd13xx/EWD1303.PDF.
16. Jeff Dike. 2001. A user-mode port of the Linux kernel. In: Proceedings of the Atlanta Linux Showcase. URL http://www.linuxshowcase.org/2001/full_papers/dike/dike.pdf.
17. Aggelos Economopoulos. 2007. A Peek at the DragonFly Virtual Kernel. LWN.net. URL http://lwn.net/Articles/228404/
18. Bryan Ford, Godmar Back, Greg Benson, Jay Lepreau, Albert Lin, and Olin Shivers. 1997. The Flux OSKit: A Substrate for OS and Language Research. In: Proceedings of the ACM Symposium on Operating Systems Principles, pages 38–51.
19. Bryan Ford and Russ Cox. 2008. Vx32: Lightweight User-level Sandboxing on the x86. In: Proceedings of the USENIX Annual Technical Conference, pages 293–306.
20. Ludovico Gardenghi, Michael Goldweber, and Renzo Davoli. 2008. ViewOS: A New Unifying Approach Against the Global View Assumption. In: Proceedings of the 8th International Conference on Computational Science, Part I, pages 287–296.
21. Robert A. Gingell, Meng Lee, Xuong T. Dang, and Mary S. Weeks. 1987. Shared Libraries in SunOS. In: Proceedings of the USENIX Summer Technical Conference, pages 375–390.
22. GNU GRUB. URL http://www.gnu.org/software/grub/.
23. James P. Hennessy, Damian L. Osisek, and Joseph W. Seigh II. 1989. Passive Serialization in a Multitasking Environment. US Patent 4,809,168.
24. Mike Hibler, Robert Ricci, Leigh Stoller, Jonathon Duerig, Shashi Guruprasad, Tim Stack, Kirk Webb, and Jay Lepreau. 2008. Large-scale Virtualization in the Emulab Network Testbed. In: Proceedings of the USENIX Annual Technical Conference, pages 113–128.
25. Jon Howell, Bryan Parno, and John R. Douceur. 2013. Embassies: Radically Refactoring the Web. In: Proceedings of the USENIX Conference on Networked Systems Design and Implementation, pages 529–546.
26. Xuxian Jiang and Dongyan Xu. 2003. SODA: A Service-On-Demand Architecture for Application Service Hosting Utility Platforms. In: Proceedings of the 12th IEEE International Symposium on High Performance Distributed Computing, pages 174–183.
27. Poul-Henning Kamp and Robert N. M. Watson. 2000. Jails: Confining the omnipotent root. In: Proceedings of SANE Conference. URL http://www.sane.nl/events/sane2000/papers/kamp.pdf.
28. Antti Kantee. 2007. puffs - Pass-to-Userspace Framework File System. In: Proceedings of AsiaBSDCon, pages 29–42.
29. Avi Kivity, Yaniv Kamay, Dor Laor, Uri Lublin, and Anthony Liguori. 2007. KVM: the Linux Virtual Machine Monitor. In: Proceedings of the 2007 Ottawa Linux Symposium, pages 225–230.
30. Steve R. Kleiman. 1986. Vnodes: An Architecture for Multiple File System Types in Sun UNIX. In: Proceedings of the USENIX Annual Technical Conference, pages 238–247.
31. Jochen Liedtke. 1993. Improving IPC by Kernel Design. In: Proceedings of the ACM Symposium on Operating Systems Principles, pages 175–188.
32. Michael Matz, Jan Hubiˇcka, Andreas Jaeger, and Mark Mitchell. 2010. System V Application Binary Interface, AMD64 Architecture Processor Supplement, Draft Version 0.99.5. URL http://www.x86-64.org/documentation/abi-0.99.5.pdf.
33. Jim Mauro and Richard McDougall. 2001. Solaris Internals: Core Kernel Architecture. Sun Microsystems, Inc. ISBN 0-13-022496-0.
34. Steven McCanne and Van Jacobson. 1993. The BSD packet filter: a new architecture for user-level packet capture. In: Proceedings of the USENIX Winter Technical Conference, pages 259–269.
35. Paul E. McKenney. 2004. Exploiting Deferred Destruction: An Analysis of Read-Copy-Update Techniques in Operating System Kernels. Ph.D. thesis, OGI School of Science and Engineering at Oregon Health and Sciences University.
36. Marshall Kirk McKusick, Keith Bostic, Michael J. Karels, and John S. Quarterman. 1996. The Design and Implementation of the 4.4BSD Operating System. Addison Wesley. ISBN 0-201-54979-4.
37. Luke Mewburn and Matthew Green. 2003. build.sh: Cross-building NetBSD. In: Proceedings of the USENIX BSD Conference, pages 47–56.
38. Robert B. Miller. 1968. Response time in man-computer conversational transactions. In: Proceedings of the Fall Joint Computer Conference, AFIPS (Fall, part I), pages 267–277.
39. S. P. Miller, B. C. Neuman, J. I. Schiller, and J. H. Saltzer. 1988. Kerberos Authentication and Authorization System. In: Project Athena Technical Plan.
40. Ronald G. Minnich and David J. Farber. February 1993. The Mether System: Distributed Shared Memory for SunOS 4.0. Technical Report MS-CIS-93-24, University of Pennsylvania Department of Computer and Information Science.
41. Sape J. Mullender, Guido van Rossum, Andrew S. Tanenbaum, Robbert van Renesse, and Hans van Staveren. 1990. Amoeba: A Distributed Operating System for the 1990s. Computer 23, no. 5, pages 44–53.
42. Mutt E-Mail Client. URL http://www.mutt.org/.
43. NetBSD Kernel Interfaces Manual. November 2007. pud – Pass-to-Userspace Device.
44. OpenSSH. URL http://www.openssh.com/.
45. Simon Peter, Jialin Li, Irene Zhang, Dan R. K. Ports, Doug Woos, Arvind Krishnamurthy, Thomas Anderson, and Timothy Roscoe. 2014. Arrakis: The Operating System is the Control Plane. In: Proceedings of the USENIX Symposium on Operating Systems Design and Implementation, pages 1–16.
46. Rob Pike, Dave Presotto, Ken Thompson, and Howard Trickey. 1990. Plan 9 from Bell Labs. In: Proceedings of the Summer UKUUG Conference, pages 1–9.
47. The Transport Layer Security (TLS) Protocol. 2008. RFC 5246.
48. QEMU – open source processor emulator. URL http://qemu.org/.
49. Richard Rashid, Avadis Tevanian, Michael Young, David Golub, Robert Baron, David Black, William Bolosky, and Jonathan Chew. 1987. Machine-Independent Virtual Memory Management for Paged Uniprocessor and Multiprocessor Architectures. SIGARCH Computer Architecture News 15, pages 31–39.
50. Luigi Rizzo. 2012. netmap: A Novel Framework for Fast Packet I/O. pages 101–112.
51. Chuck Silvers. 2000. UBC: An Efficient Unified I/O and Memory Caching Subsystem for NetBSD. In: Proceedings of the USENIX Annual Technical Conference, FREENIX Track, pages 285–290.
52. A NONSTANDARD FOR TRANSMISSION OF IP DATAGRAMS OVER SERIAL LINES: SLIP. 1988. RFC 1055.
53. Stephen Soltesz, Herbert P¨otzl, Marc E. Fiuczynski, Andy Bavier, and Larry Peterson. 2007. Container-based Operating System Virtualization: A Scalable, High-performance Alternative to Hypervisors. In: Proceedings of the 2nd ACM SIGOPS/EuroSys European Conference on Computer Systems, pages 275–287.
54. The Go Programming Language. URL http://golang.org/.
55. Unikernel. URL http://en.wikipedia.org/wiki/Unikernel.
56. Carl A. Waldspurger. 2002. Memory Resource Management in VMware ESX Server. ACM SIGOPS Operating Systems Review 36, pages 181–194.
57. Zhikui Wang, Xiaoyun Zhu, Pradeep Padala, and Sharad Singhal. May 2007. Capacity and Performance Overhead in Dynamic Resource Allocation to Virtual Containers. In: Proceedings of the IFIP/IEEE Symposium on Integrated Management, pages 149–158.
58. Gary R. Wright and W. Richard Stevens. 1995. TCP/IP Illustrated, Volume 2.Addison Wesley. ISBN 0-201-63354-X.
59. Junfeng Yang, Can Sar, Paul Twohey, Cristian Cadar, and Dawson Engler. 2006. Automatically Generating Malicious Disks using Symbolic Execution. In: Proceedings of the IEEE Symposium on Security and Privacy, pages 243–257.
60. Arnaud Ysmal and Antti Kantee. 2009. Fs-utils: File Systems Access Tools for Userland. In: Proceedings of EuroBSDCon. URL http://www.netbsd.org/~stacktic/ebc09_fs-utils_paper.pdf.