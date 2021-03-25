# CHAPTER 1 Introduction to Consistency and Coherence

Many modern computer systems and most multicore chips (chip multiprocessors) support shared memory in hardware. In a shared memory system, each of the processor cores may read and write to a single shared address space. These designs seek various goodness properties, such as high performance, low power, and low cost. Of course, it is not valuable to provide these goodness properties without first providing correctness. Correct shared memory seems intuitive at a hand-wave level, but, as this lecture will help show, there are subtle issues in even defining what it means for a shared memory system to be correct, as well as many subtle corner cases in designing a correct shared memory implementation. Moreover, these subtleties must be mastered in hardware implementations where bugfixes are expensive. Even academics should master these subtleties to make it more likely that their proposed designs will work.

许多现代计算机系统和大多数多核芯片（芯片多处理器）都支持硬件中的共享内存。 在共享存储系统中，每个处理器内核都可以读取和写入单个共享地址空间。 这些设计寻求各种优良特性，例如高性能，低功耗和低成本。 当然，在没有首先提供正确性的情况下提供这些优良特性是没有价值的。 正确的共享内存似乎是一臂之力，但正如本讲座将帮助表明的那样，即使定义正确的共享内存系统意味着什么，也存在一些细微的问题，并且在设计内存时还存在许多细微的案例。 正确的共享内存实现。 而且，这些细微之处必须在错误修正成本很高的硬件实现中掌握。 甚至学者也应该掌握这些细微之处，以使他们提出的设计更有可能起作用。


Designing and evaluating a correct shared memory system requires an architect to understand memory consistency and cache coherence, the two topics of this primer. Memory consistency (consistency, memory consistency model, or memory model) is a precise, architecturally visible definition of shared memory correctness. Consistency definitions provide rules about loads and stores (or memory reads and writes) and how they act upon memory. Ideally, consistency definitions would be simple and easy to understand. However, defining what it means for shared memory to behave correctly is more subtle than defining the correct behavior of, for example, a single-threaded processor core. The correctness criterion for a single processor core partitions behavior between one correct result and many incorrect alternatives. This is because the processor’s architecture mandates that the execution of a thread transforms a given input state into a single well-defined output state, even on an out-of-order core. Shared memory consistency models, however, concern the loads and stores of multiple threads and usually allow many correct executions while disallowing many (more) incorrect ones. The possibility of multiple correct executions is due to the ISA allowing multiple threads to execute concurrently, often with many possible legal interleavings of instructions from different threads. The multitude of correct executions complicates the erstwhile simple challenge of determining whether an execution is correct. Nevertheless, consistency must be mastered to implement shared memory and, in some cases, to write correct programs that use it.

设计和评估正确的共享内存系统要求架构师了解内存一致性和缓存一致性，这是本入门课程的两个主题。内存一致性（一致性，内存一致性模型或内存模型）是共享内存正确性的精确的，结构上可见的定义。一致性定义提供有关加载和存储（或内存读写）以及它们如何作用于内存的规则。理想情况下，一致性定义应该简单易懂。但是，定义共享内存正确行为的含义比定义例如单线程处理器内核的正确行为要微妙得多。单处理器内核的正确性标准将行为划分为一个正确的结果和许多错误的替代方案。这是因为处理器的体系结构要求，即使在无序的内核上，线程的执行也可以将给定的输入状态转换为明确定义的单个输出状态。但是，共享内存一致性模型涉及多个线程的负载和存储，通常允许许多正确的执行，而不允许许多（更多）不正确的执行。多次正确执行的可能性归因于ISA允许多个线程同时执行，并且通常可能对来自不同线程的指令进行许多合法的交织。大量正确的执行使以前确定执行是否正确的简单挑战变得复杂。但是，必须掌握一致性才能实现共享内存，并且在某些情况下，必须编写使用该内存的正确程序。

The microarchitecture — the hardware design of the processor cores and the shared memory system—must enforce the desired consistency model. As part of this consistency model support, the hardware provides cache coherence (or coherence). In a shared-memory system with caches, the cached values can potentially become out-of-date (or incoherent) when one of the processors updates its cached value. Coherence seeks to make the caches of a shared-memory system as functionally invisible as the caches in a single-core system; it does so by propagating a processor’s write to other processors’ caches. It is worth stressing that unlike consistency which is an architectural specification that defines shared memory correctness, coherence is a means to supporting a consistency model.

微体系结构（处理器核心和共享内存系统的硬件设计）必须强制执行所需的一致性模型。 作为此一致性模型支持的一部分，硬件提供了缓存一致性（或一致性）。 在具有缓存的共享内存系统中，当处理器之一更新其缓存值时，缓存值可能会过时（或不一致）。 Coherence 力图使共享内存系统的缓存在功能上与单核系统中的缓存不可见。 它是通过将处理器的写操作传播到其他处理器的缓存来实现的。 值得强调的是，与一致性（定义共享内存正确性的体系结构规范）不同，一致性是支持一致性模型的一种手段。

Even though consistency is the first major topic of this primer, we begin in Chapter 2 with a brief introduction to coherence because coherence protocols play an important role in providing consistency. The goal of Chapter 2 is to explain enough about coherence to understand how consistency models interact with coherent caches, but not to explore specific coherence protocols or implementations, which are topics we defer until the second portion of this primer in Chapters 6–9.

尽管一致性是本入门书的第一个主要主题，但我们还是从第2章开始对一致性进行了简要介绍，因为一致性协议在提供一致性方面起着重要的作用。 第2章的目的是对一致性进行足够的解释，以了解一致性模型如何与一致性缓存交互，而不是探究特定的一致性协议或实现，这是我们推迟到第6-9章入门的第二部分为止的主题。

## 1.1 CONSISTENCY (A.K.A., MEMORY CONSISTENCY, MEMORY CONSISTENCY MODEL, OR MEMORY MODEL)

Consistency models define correct shared memory behavior in terms of loads and stores (memmory reads and writes), without reference to caches or coherence. To gain some real-world intuition on why we need consistency models, consider a university that posts its course schedule online. Assume that the Computer Architecture course is originally scheduled to be in Room 152. The day before classes begin, the university registrar decides to move the class to Room 252. The registrar sends an e-mail message asking the website administrator to update the online schedule, and a few minutes later, the registrar sends a text message to all registered students to check the newly updated schedule. It is not hard to imagine a scenario—if, say, the website administrator is too busy to post the update immediately—in which a diligent student receives the text message, immediately checks the online schedule, and still observes the (old) class location Room 152. Even though the online schedule is eventually updated to Room 252 and the registrar performed the “writes” in the correct order, this diligent student observed them in a different order and thus went to the wrong room. A consistency model defines whether this behavior is correct (and thus whether a user must take other action to achieve the desired outcome) or incorrect (in which case the system must preclude these reorderings).

一致性模型根据加载和存储（内存读取和写入）定义了正确的共享内存行为，而没有引用缓存或一致性。为了获得关于为什么我们需要一致性模型的一些直觉，可以考虑一所大学在线发布课程表。假定计算机体系结构课程原定安排在152室中。在上课的前一天，大学注册服务商决定将课程移至252房间。注册服务商发送电子邮件，要求网站管理员更新在线时间表。几分钟后，注册服务商会向所有注册的学生发送一条短信，以检查新近更新的时间表。不难想像一种情况，例如，网站管理员太忙而无法立即发布更新，在这种情况下，勤奋的学生会收到短信，立即检查在线时间表，并仍然观察（以前的）课堂位置152室。尽管最终将在线时间表更新为252室，并且注册服务商以正确的顺序执行“写操作”，但这位勤奋的学生却以不同的顺序观察它们，因此进入了错误的房间。一致性模型定义此行为是正确的（因此，用户是否必须采取其他操作才能实现所需的结果）还是不正确的（在这种情况下，系统必须排除这些重新排序）。

Although this contrived example used multiple media, similar behavior can happen in shared memory hardware with out-of-order processor cores, write buffers, prefetching, and multiple cache banks. Thus, we need to define shared memory correctness—that is, which shared memory behaviors are allowed—so that programmers know what to expect and implementors know the limits to what they can provide.

尽管这个人为的示例使用了多种媒体，但是在具有乱序的处理器内核，写缓冲区，预取和多个缓存库的共享内存硬件中，也会发生类似的行为。 因此，我们需要定义共享内存的正确性（即允许哪些共享内存行为），以便程序员知道期望什么，实现者知道他们可以提供的限制。

Shared memory correctness is specified by a memory consistency model or, more simply, a memory model. The memory model specifies the allowed behavior of multithreaded programs executing with shared memory. For a multithreaded program executing with specific input data, the memory model specifies what values dynamic loads may return and, optionally, what possible final states of the memory are. Unlike single-threaded execution, multiple correct behaviors are usually allowed, making understanding memory consistency models subtle.

共享内存正确性是由内存一致性模型或更简单地由内存模型指定的。 内存模型指定使用共享内存执行的多线程程序的允许行为。 对于使用特定输入数据执行的多线程程序，内存模型指定动态负载可能返回的值，以及可选的内存最终状态是什么。 与单线程执行不同，通常允许多种正确的行为，这使对内存一致性模型的理解变得微妙。

Chapter 3 introduces the concept of memory consistency models and presents sequential consistency (SC), the strongest and most intuitive consistency model. The chapter begins by motivating the need to specify shared memory behavior and precisely defines what a memory consistency model is. It next delves into the intuitive SC model, which states that a multithreaded execution should look like an interleaving of the sequential executions of each constituent thread, as if the threads were time-multiplexed on a single-core processor. Beyond this intuition, the chapter formalizes SC and explores implementing SC with coherence in both simple and aggressive ways, culminating with a MIPS R10000 case study.

第3章介绍了内存一致性模型的概念，并介绍了顺序一致性（SC），这是最强大，最直观的一致性模型。 本章首先激发了指定共享内存行为的需求，并精确定义了什么是内存一致性模型。 接下来，它研究了直观的SC模型，该模型指出多线程执行应该看起来像每个组成线程的顺序执行的交错，就像线程在单核处理器上是时分复用的一样。 除了这种直觉之外，本章还对SC进行了形式化，并探索了以简单和积极的方式以连贯的方式实现SC的过程，最后以MIPS R10000案例研究为结尾。

In Chapter 4, we move beyond SC and focus on the memory consistency model implemented by x86 and historical SPARC systems. This consistency model, called total store order (TSO), is motivated by the desire to use first-in–first-out write buffers to hold the results of committed stores before writing the results to the caches. This optimization violates SC, yet promises enough performance benefit to inspire architectures to define TSO, which permits this optimization. In this chapter, we show how to formalize TSO from our SC formalization, how TSO affects implementations, and how SC and TSO compare.

在第4章中，我们将超越SC，而将重点放在由x86和历史SPARC系统实现的内存一致性模型上。 这种一致性模型被称为总存储顺序（TSO），其原因是希望在将结果写入高速缓存之前使用先进先出写入缓冲区来保存已提交存储的结果。 此优化违反了SC，但承诺将提供足够的性能优势，以启发体系结构来定义TSO，从而允许进行此优化。 在本章中，我们将说明如何从我们的SC形式化中将TSO形式化，TSO如何影响实现以及SC与TSO的比较。

Finally, Chapter 5 introduces “relaxed” or “weak” memory consistency models. It motivates these models by showing that most memory orderings in strong models are unnecessary. If a thread updates ten data items and then a synchronization flag, programmers usually do not care if the data items are updated in order with respect to each other but only that all data items are updated before the flag is updated. Relaxed models seek to capture this increased ordering flexibility to get higher performance or a simpler implementation. After providing this motivation, the chapter develops an example relaxed consistency model, called XC, wherein programmers get order only when they ask for it with a FENCE instruction (e.g., a FENCE after the last data update but before the flag write). The chapter then extends the formalism of the previous two chapters to handle XC and discusses how to implement XC (with considerable reordering between the cores and the coherence protocol). The chapter then discusses a way in which many programmers can avoid thinking about relaxed models directly: if they add enough FENCEs to ensure their program is data-race free (DRF), then most relaxed models will appear SC. With “SC for DRF,” programmers can get both the (relatively) simple correctness model of SC with the (relatively) higher performance of XC. For those who want to reason more deeply, the chapter concludes by distinguishing acquires from releases, discussing write atomicity and causality, pointing to commercial examples (including an IBM Power case study), and touching upon high-level language models ( Java and CCC).

最后，第5章介绍了“松弛”或“弱”的内存一致性模型。通过显示强模型中的大多数内存排序是不必要的，从而激发了这些模型。如果一个线程先更新十个数据项，然后再更新一个同步标志，则程序员通常不在乎数据项是否相对于彼此依次更新，而只是在更新标志之前仅更新所有数据项。宽松的模型试图捕获这种增加的订购灵活性，以获得更高的性能或更简单的实现。在提供了这种动机之后，本章将开发一个示例性的宽松一致性模型，称为XC，其中程序员仅在使用FENCE指令（例如，在最后一次数据更新之后但在写入标志之前）请求FENCE时才获得命令。然后，本章将前两章的形式主义扩展到处理XC，并讨论如何实现XC（在内核和一致性协议之间进行大量重新排序）。然后，本章讨论了许多程序员可以避免直接考虑松弛模型的方法：如果他们添加足够的FENCE以确保其程序不涉及数据争用（DRF），则大多数松弛模型将显示为SC。使用“用于DRF的SC”，程序员可以同时获得（相对）简单的SC正确性模型和（相对）更高的XC性能。对于那些想更深入地推理的人，本章的结尾部分是区分发布中的获取和获取，讨论写原子性和因果关系，指向商业示例（包括IBM Power案例研究）以及高级语言模型（Java和CCC）。 

Returning to the real-world consistency example of the class schedule, we can observe that the combination of an email system, a human web administrator, and a text-messaging system represents an extremely weak consistency model. To prevent the problem of a diligent student going to the wrong room, the university registrar needed to perform a FENCE operation after her email to ensure that the online schedule was updated before sending the text message.

回到课堂时间表的实际一致性示例，我们可以看到电子邮件系统，人工Web管理员和文本消息系统的组合代表了一个非常弱的一致性模型。 为了防止勤奋的学生进入错误的房间，大学注册服务商需要在发送电子邮件后执行FENCE操作，以确保在发送短信之前更新了在线日程表。

## 1.2 COHERENCE (A.K.A., CACHE COHERENCE)

Unless care is taken, a coherence problem can arise if multiple actors (e.g., multiple cores) have access to multiple copies of a datum (e.g., in multiple caches) and at least one access is a write. Consider an example that is similar to the memory consistency example. A student checks the online schedule of courses, observes that the Computer Architecture course is being held in Room 152 (reads the datum), and copies this information into her calendar app in her mobile phone (caches the datum). Subsequently, the university registrar decides to move the class to Room 252, updates the online schedule (writes to the datum) and informs the students via a text message. The student’s copy of the datum is now stale, and we have an incoherent situation. If she goes to Room 152, she will fail to find her class. Examples of incoherence from the world of computing, but not including computer architecture, include stale web caches and programmers using un-updated code repositories

除非小心，否则如果多个参与者（例如，多个核）可以访问数据的多个副本（例如，在多个高速缓存中）并且至少一次访问是写操作，则可能会出现一致性问题。 考虑一个与内存一致性示例相似的示例。 一个学生检查了在线课程表，观察到计算机体系结构课程正在152室举行（读取数据），然后将此信息复制到她的手机日历应用程序中（存储数据）。 随后，大学注册服务商决定将班级移至252室，更新在线时间表（写入数据），并通过短信通知学生。 该学生的基准副本现在已过时，而且我们的情况不连贯。 如果她去152室，她将找不到自己的班级。 计算世界（但不包括计算机体系结构）的不一致性示例包括陈旧的Web缓存和使用未更新代码存储库的程序员

Access to stale data (incoherence) is prevented using a coherence protocol, which is a set of rules implemented by the distributed set of actors within a system. Coherence protocols come in many variants but follow a few themes, as developed in Chapters 6–9. Essentially, all of the variants make one processor’s write visible to the other processors by propagating the write to all caches, i.e., keeping the calendar in sync with the online schedule. But protocols differ in when and how the syncing happens. There are two major classes of coherence protocols. In the first approach, the coherence protocol ensures that writes are propagated to the caches synchronously. When the online schedule is updated, the coherence protocol ensures that the student’s calendar is updated as well. In the second approach, the coherence protocol propagates writes to the caches asynchronously, while still honoring the consistency model. The coherence protocol does not guarantee that when the online schedule is updated, the new value will have propagated to the student’s calendar as well; however, the protocol does ensure that the new value is propagated before the text message reaches her mobile phone. This primer focuses on the first class of coherence protocols (Chapters 6–9) while Chapter 10 discusses the emerging second class.

使用一致性协议可以防止对陈旧数据的访问（不一致性），该协议是由系统内分散的参与者集合实现的一组规则。一致性协议有许多变体，但遵循第6–9章开发的几个主题。本质上，所有变体都通过将写入传播到所有缓存来使一个处理器的写入对其他处理器可见，即使日历与在线时间表保持同步。但是协议在同步的时间和方式方面有所不同。一致性协议主要有两类。在第一种方法中，一致性协议确保将写入同步传播到高速缓存。更新在线时间表后，一致性协议可确保学生的日历也被更新。在第二种方法中，一致性协议将写操作异步传播到高速缓存，同时仍然遵守一致性模型。一致性协议不能保证在更新在线时间表后，新值也将传播到学生的日历中；但是，该协议确实确保了在短信到达其手机之前传播新值。本入门手册重点关注第一类一致性协议（第6-9章），而第10章讨论了新兴的第二类协议。

Chapter 6 presents the big picture of cache coherence protocols and sets the stage for the subsequent chapters on specific coherence protocols. This chapter covers issues shared by most coherence protocols, including the distributed operations of cache controllers and memory  controllers and the common MOESI coherence states: modified (M), owned (O), exclusive (E), shared (S), and invalid (I). Importantly, this chapter also presents our table-driven methodology for presenting protocols with both stable (e.g., MOESI) and transient coherence states. Transient states are required in real implementations because modern systems rarely permit atomic transitions from one stable state to another (e.g., a read miss in state Invalid will spend some time waiting for a data response before it can enter state Shared). Much of the real complexity in coherence protocols hides in the transient states, similar to how much of processor core complexity hides in micro-architectural states.

第6章介绍了高速缓存一致性协议的概况，并为后续有关特定一致性协议的章节奠定了基础。 本章涵盖了大多数一致性协议共享的问题，包括高速缓存控制器和内存控制器的分布式操作以及常见的MOESI一致性状态：已修改（M），已拥有（O），独占（E），共享（S）和无效（I）。 重要的是，本章还介绍了我们的表驱动方法，用于介绍具有稳定状态（例如MOESI）和瞬态相干状态的协议。 由于现代系统很少允许原子从一种稳定状态转换为另一种稳定状态（例如，处于无效状态的读未命中将花费一些时间等待数据响应才能进入共享状态），因此在实际实现中需要使用瞬态。 相干协议中的许多实际复杂度隐藏在瞬态中，类似于微体系结构状态隐藏了多少处理器核心复杂度。

Chapter 7 covers snooping cache coherence protocols, which initially dominated the commercial market. At the hand-wave level, snooping protocols are simple. When a cache miss occurs, a core’s cache controller arbitrates for a shared bus and broadcasts its request. The shared bus ensures that all controllers observe all requests in the same order and thus all controllers can coordinate their individual, distributed actions to ensure that they maintain a globally consistent state. Snooping gets complicated, however, because systems may use multiple buses and modern buses do not atomically handle requests. Modern buses have queues for arbitration and can send responses that are unicast, delayed by pipelining, or out-of-order. All of these features  lead to more transient coherence states. Chapter 7 concludes with case studies of the Sun UltraEnterprise E10000 and the IBM Power5.

第7章介绍了窥探式缓存一致性协议，该协议最初在商业市场上占主导地位。 在手波级别，监听协议很简单。 当发生缓存未命中时，核心的缓存控制器会为共享总线进行仲裁并广播其请求。 共享总线确保所有控制器以相同的顺序观察所有请求，因此所有控制器可以协调其各自的分布式操作，以确保它们保持全局一致的状态。 但是，侦听变得很复杂，因为系统可能使用多个总线，而现代总线无法自动处理请求。 现代总线上有仲裁队列，可以发送单播，流水线延迟或乱序的响应。 所有这些特征导致更多的瞬态相干状态。 第7章以Sun UltraEnterprise E10000和IBM Power5的案例研究作为结束。

Chapter 8 delves into directory cache coherence protocols that offer the promise of scaling to more processor cores and other actors than snooping protocols that rely on broadcast. There is a joke that all problems in computer science can be solved with a level of indirection. Directory protocols support this joke: A cache miss requests a memory location from the next level cache (or memory) controller, which maintains a directory that tracks which caches hold which locations. Based on the directory entry for the requested memory location, the controller sends a response message to the requestor or forwards the request message to one or more actors currently caching the memory location. Each message typically has one destination (i.e., no broadcast or multicast), but transient coherence states abound as transitions from one stable coherence state to another stable one can generate a number of messages proportional to the number of actors in the system. This chapter starts with a basic MSI directory protocol and then refines it to handle the MOESI states E and O, distributed directories, less stalling of requests, approximate directory entry representations, and more. The chapter also explores the design of the directory itself, including directory caching techniques. The chapter concludes with case studies of the old SGI Origin 2000 and the newer AMD HyperTransport, HyperTransport Assist, and Intel QuickPath Interconnect (QPI).

第8章研究目录高速缓存一致性协议，该协议有望将其扩展到比依赖广播的侦听协议更多的处理器内核和其他参与者。开个玩笑说，计算机科学中的所有问题都可以通过一定程度的间接解决。目录协议支持这种笑话：高速缓存未命中从下一级高速缓存（或内存）控制器请求存储位置，该控制器维护一个目录，该目录跟踪哪些高速缓存保存哪些位置。基于所请求的存储位置的目录条目，控制器将响应消息发送给请求者，或将请求消息转发给当前缓存该存储位置的一个或多个参与者。每个消息通常具有一个目的地（即，没有广播或多播），但是瞬态一致性状态比比皆是，因为从一种稳定的一致性状态到另一种稳定的一致性的转换可以生成与系统中的参与者数量成比例的消息。本章从基本的MSI目录协议开始，然后对其进行完善，以处理MOESI状态E和O，分布式目录，较少的请求停顿，近似的目录条目表示等等。本章还探讨了目录本身的设计，包括目录缓存技术。本章以旧SGI Origin 2000和较新的AMD HyperTransport，HyperTransport Assist和Intel QuickPath Interconnect（QPI）的案例研究作为结束。

Chapter 9 deals with some, but not all, of the advanced topics in coherence. For ease of explanation, the prior chapters on coherence intentionally restrict themselves to the simplest system models needed to explain the fundamental issues. Chapter 9 delves into more complicated system models and optimizations, with a focus on issues that are common to both snooping and directory protocols. Initial topics include dealing with instruction caches, multilevel caches, write-through caches, translation lookaside buffers (TLBs), coherent direct memory access (DMA), virtual caches, and hierarchical coherence protocols. Finally, the chapter delves into performance optimizations (e.g., targeting migratory sharing and false sharing) and a new protocol family called Token Coherence that subsumes directory and snooping coherence.

第9章讨论了一些（但不是全部）连贯的高级主题。 为了便于说明，有关一致性的前几章有意地将自己限制在解释基本问题所需的最简单的系统模型上。 第9章深入探讨了更复杂的系统模型和优化，重点介绍了侦听和目录协议共同的问题。 最初的主题包括处理指令高速缓存，多级高速缓存，直写式高速缓存，转换后备缓冲器（TLB），相干直接内存访问（DMA），虚拟高速缓存和分层一致性协议。 最后，本章深入研究了性能优化（例如，针对迁移共享和虚假共享）和称为令牌一致性的新协议系列，该协议系列包含目录和监听一致性。

## 1.3 CONSISTENCY AND COHERENCE FOR HETEROGENEOUS SYSTEMS

Modern computer systems are predominantly heterogeneous. A mobile phone processor today not only contains a multicore CPU, it also has a GPU and other accelerators (e.g., neural network hardware). In the quest for programmability, such heterogeneous systems are starting to support shared memory. Chapter 10 deals with consistency and coherence for such heterogeneous processors.

现代计算机系统主要是异构的。 当今的移动电话处理器不仅包含多核CPU，而且还具有GPU和其他加速器（例如神经网络硬件）。 在对可编程性的追求中，这种异构系统开始支持共享内存。 第10章讨论了此类异构处理器的一致性和一致性。

The chapter starts by focusing on GPUs, arguably the most popular accelerators today. The chapter observes that GPUs originally chose not to support hardware cache coherence, since GPUs are designed for embarrassingly parallel graphics workloads that do not synchronize or share data all that much. However, the absence of hardware cache coherence leads to programmability and/or performance challenges when GPUs are used for general-purpose workloads with fine-grained synchronization and data sharing. The chapter discusses in detail some of the promising coherence alternatives that overcome these limitations—in particular, explaining why the candidate protocols enforce the consistency model directly rather than implementing coherence in a consistency-agnostic manner. The chapter concludes with a brief discussion on consistency and coherence across CPUs and the accelerators.

本章首先关注GPU，可以说是当今最流行的加速器。 本章指出，GPU最初选择不支持硬件缓存一致性，因为GPU是为无法同步或共享数据的尴尬并行图形工作负载而设计的。 但是，当GPU用于具有细粒度同步和数据共享的通用工作负载时，缺乏硬件缓存一致性会导致可编程性和/或性能挑战。 本章详细讨论了克服这些限制的一些有前途的一致性替代方法，尤其是解释了为什么候选协议直接执行一致性模型而不是以与一致性无关的方式实现一致性的原因。 本章最后简要讨论了CPU和加速器之间的一致性和一致性。

## 1.4 SPECIFYING AND VALIDATING MEMORY CONSISTENCY MODELS AND CACHE COHERENCE

Consistency models and coherence protocols are complex and subtle. Yet, this complexity must be managed to ensure that multicores are programmable and that their designs can be validated. To achieve these goals, it is critical that consistency models are specified formally. A formal specification would enable programmers to clearly and exhaustively (with tool support) understand what behaviors are permitted by the memory model and what behaviors are not. Second, a precise formal specification is mandatory for validating implementations.

一致性模型和一致性协议是复杂而微妙的。 但是，必须管理这种复杂性，以确保多核是可编程的，并且可以验证其设计。 为了实现这些目标，至关重要的是要正式指定一致性模型。 正式的规范将使程序员能够清楚且详尽地（在工具支持下）了解内存模型允许哪些行为，哪些行为不允许。 其次，对于验证实现，必须有一个精确的正式规范。

Chapter 11 starts by discussing two methods for specifying systems — axiomatic and operational — focusing on how these methods can be applied for consistency models and coherence protocols. Then the chapter goes over techniques for validating implementations — including processor pipeline and coherence protocol implementations — against their specification. The chapter discusses both formal methods and informal testing

第11章首先讨论了用于指定系统的两种方法-公理的和可操作的-着眼于如何将这些方法应用于一致性模型和一致性协议。 然后，本章介绍了根据规范验证实现的技术，包括处理器管线和一致性协议实现。 本章讨论了正式方法和非正式测试

## 1.5 A CONSISTENCY AND COHERENCE QUIZ

It can be easy to convince oneself that one’s knowledge of consistency and coherence is sufficient and that reading this primer is not necessary. To test whether this is the case, we offer this pop quiz.

可以很容易地说服自己一个人对一致性和连贯性的了解就足够了，而不必阅读本入门书。 为了测试是否存在这种情况，我们提供了此弹出式测验。

Question 1: In a system that maintains sequential consistency, a core must issue coherence requests in program order. True or false? (Answer is in Section 3.8)

问题1：在保持顺序一致性的系统中，内核必须按程序顺序发出一致性请求。 对或错？ （答案在第3.8节中）

Question 2: The memory consistency model specifies the legal orderings of coherence transactions. True or false? (Section 3.8)

问题2：内存一致性模型指定了一致性事务的合法顺序。 对或错？ （第3.8节）

Question 3: To perform an atomic read–modify–write instruction (e.g., test-and-set), a core must always communicate with the other cores. True or false? (Section 3.9)

问题3：要执行基本的读取-修改-写指令（例如，测试并设置），一个内核必须始终与其他内核通信。 对或错？ （第3.9节）

Question 4: In a TSO system with multithreaded cores, threads may bypass values out of the write buffer, regardless of which thread wrote the value. True or false? (Section 4.4)

问题4：在具有多线程内核的TSO系统中，线程可能会绕过写缓冲区之外的值，而不管哪个线程写了该值。 对或错？ （第4.4节）

Question 5: A programmer who writes properly synchronized code relative to the high-level language’s consistency model (e.g., Java) does not need to consider the architecture’s memory consistency model. True or false? (Section 5.9)

问题5：相对于高级语言的一致性模型（例如Java）编写适当同步的代码的程序员无需考虑体系结构的内存一致性模型。 对或错？ （第5.9节）

Question 6: In an MSI snooping protocol, a cache block may only be in one of three coherence states. True or false? (Section 7.2)

问题6：在MSI侦听协议中，缓存块可能仅处于三个一致性状态之一。 对或错？ （第7.2节）

Question 7: A snooping cache coherence protocol requires the cores to communicate on a bus. True or false? (Section 7.6)

问题7：侦听缓存一致性协议要求内核在总线上进行通信。 对或错？ （第7.6节）

Question 8: GPUs do not support hardware cache coherence. Therefore, they are unable to enforce a memory consistency model. True or False? (Section 10.1).

问题8：GPU不支持硬件高速缓存一致性。 因此，他们无法强制执行内存一致性模型。 对或错？ （第10.1节）。

Even though the answers are provided later in this primer, we encourage readers to try to answer the questions before looking ahead at the answers.

即使在本入门课程的后面提供了答案，我们还是鼓励读者尝试着回答问题，然后再展望答案。

## 1.6 WHAT THIS PRIMER DOES NOT DO

This lecture is intended to be a primer on coherence and consistency. We expect this material could be covered in a graduate class in about ten 75-minute classes (e.g., one lecture per Chapter 2 to Chapter 11).

本讲座旨在作为连贯性和一致性的入门。 我们希望这些材料可以在大约10个75分钟的课堂上被研究生班所使用（例如，每第2章到第11章讲一次）。

For this purpose, there are many things the primer does not cover. Some of these include the following.

为此，底漆有许多内容无法涵盖。 其中一些包括以下内容。

* Synchronization. Coherence makes caches invisible. Consistency can make shared memory look like a single memory module. Nevertheless, programmers will probably need locks, barriers, and other synchronization techniques to make their programs useful. Readers are referred to the Synthesis Lecture on Shared-Memory synchronization [2]. <br> 同步。 一致性使缓存不可见。 一致性可使共享内存看起来像单个内存模块。 但是，程序员可能需要锁，屏障和其他同步技术才能使程序有用。 读者可以参考有关共享内存同步的综合讲座[2]。
* Commercial Relaxed Consistency Models. This primer does not cover the subtleties of the ARM, PowerPC, and RISC-V memory models, but does describe which mechanisms they provide to enforce order. <br> 商业宽松一致性模型。 本入门文章未涵盖ARM，PowerPC和RISC-V内存模型的精妙之处，但确实描述了它们提供的用于执行顺序的机制。
* Parallel programming. This primer does not discuss parallel programming models, methodologies, or tools. <br> 并行编程。 本入门手册不讨论并行编程模型，方法或工具。
* Consistency in distributed systems. This primer restricts itself to consistency within a shared memory multicore, and does not cover consistency models and their enforcement for a general distributed system. Readers are referred to the Synthesis Lectures on Database Replication [1] and Quorum Systems [3]. <br> 分布式系统中的一致性。 本入门手册将其自身限制在共享内存多核内的一致性，并且不涉及一致性模型及其对通用分布式系统的实施。 读者可以参考有关数据库复制的综合讲座[1]和仲裁系统[3]。

## 1.7 REFERENCES

1. B. Kemme, R. Jiménez-Peris, and M. Patiño-Martínez. Database Replication. Synthesis Lectures on Data Management. Morgan & Claypool Publishers, 2010. DOI: 10.1007/978-1-4614-8265-9_110. 8
2. M. L. Scott. Shared-Memory Synchronization. Synthesis Lectures on Computer Architecture. Morgan & Claypool Publishers, 2013. DOI: 10.2200/s00499ed1v01y201304cac023.7
3. M. Vukolic. Quorum Systems: With Applications to Storage and Consensus. Synthesis Lectures on Distributed Computing Theory. Morgan & Claypool Publishers, 2012. DOI: 10.2200/s00402ed1v01y201202dct009. 8
