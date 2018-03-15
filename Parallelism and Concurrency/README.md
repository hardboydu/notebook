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

### Lock-Free Reference

* [Parallel Computer Architecture and Programming (CMU 5-418/618)](http://15418.courses.cs.cmu.edu/fall2017/)
    * [Lecture 18: Fine-grained synchronization & lock-free programming](http://15418.courses.cs.cmu.edu/fall2017/lecture/lockfree)
* [SEI CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard)
    * [CON09-C. Avoid the ABA problem when using lock-free algorithms](https://wiki.sei.cmu.edu/confluence/display/c/CON09-C.+Avoid+the+ABA+problem+when+using+lock-free+algorithms)
* [atomic_data: A Multibyte General Purpose Lock-Free Data Structure](https://alexpolt.github.io/atomic-data.html)
* [Lock-Free Concurrent Data Structures (PPT)](http://users.minet.uni-jena.de/~nwk/LockFree.pdf)
* A Pragmatic Implementation of Non-Blocking Linked Lists
* A practical multi-word compare-and-swap operation

### Test Reference

* [Testing Concurrent Software](http://users.minet.uni-jena.de/~nwk/ConcurrTest.pdf)
