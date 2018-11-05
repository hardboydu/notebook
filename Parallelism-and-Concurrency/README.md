# Parallelism(并行) and Concurrency(并发)

## 并行与并发的区别

* 《深入理解计算机系统（第三版）》 第12章

    如果逻辑控制流在时间上重叠，那么他们就是**并发的（concurrent）**。这种常见的现象称为**并发（concurrency）**。

    **并行**（Parallelism）程序是一个运行在多个处理器上的并发程序，因此，并行程序的集合是并发程序集合的真子集。

* 《并发的艺术》 第1章

    如果某个系统支持两个或者多个动作（Action）同时存在，那么这个系统就是一个并发系统。如果某个系统支持两个或者多个动作同时执行，那么这个系统就是一个并行系统。并发系统与并行系统这两个定义之间的关键差异在于“存在”这个词。

    在并发程序中可以同时拥有两个或者多个线程。这意味着，如果程序在单核处理器上运行，那么这两个线程将交替地换入或者换出内存。这些线程是同时“存在”的——每个线程都处于执行过程中的某个状态。如果程序能够并行执行，那么就一定是运行在多核处理器上。此时，程序中的每个线程都将分配到一个独立的处理器核上，因此可以同时运行。

    我相信你已经能够得出结论——“并行”概念是“并发”概念的一个子集。也就是说，你可以编写一个拥有多个线程或者进程的并发程序，但如果没有多核处理器来执行这个程序，那么就不能以并行方式来运行代码。因此，凡是在求解单个问题时涉及多个执行流程的编程模式或者执行行为，都属于并发编程的范畴。

* Erlang 之父 Joe Armstrong

    ![Parallel and Concurrency](Parallel_and_Concurrency.jpg)

## Reference

* [Parallel Computer Architecture and Programming (CMU 5-418/618)](http://15418.courses.cs.cmu.edu/fall2017/)
* [Scal High-Performance Multicore-Scalable Computing](http://scal.cs.uni-salzburg.at/) We study the design, implementation, performance, and scalability of concurrent objects on multicore systems by analyzing the apparent trade-off between adherence to concurrent data structure semantics and scalability.

### Blogs

* [Preshing on Programming](http://preshing.com/)
    * [An Introduction to Lock-Free Programming](http://preshing.com/20120612/an-introduction-to-lock-free-programming/)
    * [A Lock-Free... Linear Search?](http://preshing.com/20130529/a-lock-free-linear-search/)
    * [The World's Simplest Lock-Free Hash Table](http://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/)
    * [Atomic vs. Non-Atomic Operations](http://preshing.com/20130618/atomic-vs-non-atomic-operations/)
    * [New Concurrent Hash Maps for C++](http://preshing.com/20160201/new-concurrent-hash-maps-for-cpp/)
    * [A Resizable Concurrent Map](http://preshing.com/20160222/a-resizable-concurrent-map/)
* [1024cores](http://www.1024cores.net/)   about lockfree, waitfree, obstructionfree synchronization algorithms and data structures, scalability-oriented architecture, multicore/multiprocessor design patterns, high-performance computing, threading technologies and libraries (OpenMP, TBB, PPL), message-passing systems and related topics.
* [Kukuruku Hub](https://kukuruku.co/)
    * [Lock-free Data Structures. 1 — Introduction](https://kukuruku.co/post/lock-free-data-structures-introduction/)
    * [Lock-free Data Structures. Basics: Atomicity and Atomic Primitives](https://kukuruku.co/post/lock-free-data-structures-basics-atomicity-and-atomic-primitives/)
    * [Lock-free Data Structures. Memory Model. Part 3](https://kukuruku.co/post/lock-free-data-structures-memory-model-part-3/)
    * [Lock-free Stack for Windows](https://kukuruku.co/post/lock-free-stack-for-windows/)
    * [Lock-free Data Structures. The Inside. Memory Management Schemes](https://kukuruku.co/post/lock-free-data-structures-the-inside-memory-management-schemes/)
    * [Lock-free Data Structures. The Inside. RCU](https://kukuruku.co/post/lock-free-data-structures-the-inside-rcu/)
    * [Lock-Free Data Structures. The Evolution of a Stack](https://kukuruku.co/post/lock-free-data-structures-the-evolution-of-a-stack/#reference)
    * [Lock-Free Data Structures. Yet Another Treatise](https://kukuruku.co/post/lock-free-data-structures-yet-another-treatise/)
    * [Lock-Free Data Structures. Exploring Queues](https://kukuruku.co/post/lock-free-data-structures-exploring-queues/)
* [Cameron](http://moodycamel.com/)
    * [A Fast Lock-Free Queue for C++](http://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++)
    * [Solving the ABA Problem for Lock-Free Free Lists](http://moodycamel.com/blog/2014/solving-the-aba-problem-for-lock-free-free-lists)
    * [A Fast General Purpose Lock-Free Queue for C++](http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++)




### Open source

* [libcds](https://github.com/khizmax/libcds)  The Concurrent Data Structures (CDS) library is a collection of concurrent containers that don't require external (manual) synchronization for shared access, and safe memory reclamation (SMR) algorithms like Hazard Pointer and user-space RCU that is used as an epoch-based SMR.

### Lock-Free Reference

* [Parallel Computer Architecture and Programming (CMU 5-418/618)](http://15418.courses.cs.cmu.edu/fall2017/)
    * [Lecture 18: Fine-grained synchronization & lock-free programming](http://15418.courses.cs.cmu.edu/fall2017/lecture/lockfree)
* [SEI CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard)
    * [CON09-C. Avoid the ABA problem when using lock-free algorithms](https://wiki.sei.cmu.edu/confluence/display/c/CON09-C.+Avoid+the+ABA+problem+when+using+lock-free+algorithms)
* [atomic_data: A Multibyte General Purpose Lock-Free Data Structure](https://alexpolt.github.io/atomic-data.html)
* [Lock-Free Concurrent Data Structures (PPT)](http://users.minet.uni-jena.de/~nwk/LockFree.pdf)
* A Pragmatic Implementation of Non-Blocking Linked Lists
* A practical multi-word compare-and-swap operation
* On the Design and Implementation of an Efficient Lock-Free Scheduler (JSSPP 2015, JSSPP 2016: Job Scheduling Strategies for Parallel Processing pp 22-45)

### Test Reference

* [Testing Concurrent Software](http://users.minet.uni-jena.de/~nwk/ConcurrTest.pdf)
