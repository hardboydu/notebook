# The Memory Hierarchy

To this point in our study of systems, we have relied on a simple model of a computer system as a CPU that executes instructions and a memory system that holds instructions and data for the CPU. In our simple model, the memory system is a linear array of bytes, and the CPU can access each memory location in a constant amount of time. While this is an effective model up to a point, it does not reflect the way that modern systems really work.

到目前为止，在对系统的研究中，我们依赖于一个简单的计算机系统模型，CPU 执行指令，儿存储器系统为CPU存放指令和数据。在简单模型中，存储器系统是一个线性的字节数组，而CPU能够在一个常数时间内访问每个存储器位置，虽然迄今为止这都是一个有效的模型，但是它没有反应现代系统实际工作的方式。

In practice, a memory system is a hierarchy of storage devices with different capacities, costs, and access times. CPU registers hold the most frequently used data. Small, fast cache memories nearby the CPU act as staging areas for a subset of the data and instructions stored in the relatively slow main memory. The main memory stages data stored on large, slow disks, which in turn often serve as staging areas for data stored on the disks or tapes of other machines connected by networks.

实际上，存储器系统（Memory System）是一个具有不同容量、成本和访问时间的存储设备的层次结构。CPU寄存器保存着最常用的数据。靠近CPU的小的、快速的告诉缓存存储器（cache memory）作为一部分存储在相对慢速的主存储器（main memory）中数据和指令的缓冲区域。主存缓存存储在容量较大的、慢速磁盘上的数据，而这些磁盘常常又作为存储在通过网络连接的其他机器的磁盘或磁带上的数据的缓冲区域。

Memory hierarchies work because well-written programs tend to access the storage at any particular level more frequently than they access the storage at the next lower level. So the storage at the next level can be slower, and thus larger and cheaper per bit. The overall effect is a large pool of memory that costs as much as the cheap storage near the bottom of the hierarchy but that serves data to programs at the rate of the fast storage near the top of the hierarchy.

存储器层次结构是可行的，这是因为与下一个更低层次的存储设备相比来说，一个编写良好的程序倾向于更频繁地访问某一个层次上的存储设备。所以，下一层的存储设备可以更慢速一点，也因此可以更大，每个比特位更便宜。整体效果是一个大的存储器池，其成本与层次结构底层最便宜的存储设备想当，但是却以接近于层次结构顶部存储设备的高速率想程序提供数据。

As a programmer, you need to understand the memory hierarchy because it has a big impact on the performance of your applications. If the data your program needs are stored in a CPU register, then they can be accessed in 0 cycles during the execution of the instruction. If stored in a cache, 4 to 75 cycles. If stored in main memory, hundreds of cycles. And if stored in disk, tens of millions of cycles!

最为一个程序员，你需要理解存储器层次结构，因为它对应用程序的性能有着巨大的影响。如果你的程序需要的数据是存储在CPU寄存器中的，那么在指令的执行期间，在 0 个周期内就能访问到它们。如果存储在告诉缓存中，需要 4 ~ 75 个周期。如果存储在主存中，需要上百个周期。而如果存储在磁盘上，需要大约几千万个周期。

Here, then, is a fundamental and enduring idea in computer systems: if you understand how the system moves data up and down the memory hierarchy, then you can write your application programs so that their data items are stored higher in the hierarchy, where the CPU can access them more quickly.

这里就是计算机系统中一个基本而持久的思想：如果你理解了系统是如何将数据在存储器层次结构中上上下下移动的，那么你就可以编写自己的应用程序，使得它们的数据项存储在层次结构较高的地方，在那里 CPU 能更快地访问到它们。

This idea centers around a fundamental property of computer programs known as locality. Programs with good locality tend to access the same set of data items over and over again, or they tend to access sets of nearby data items. Programs with good locality tend to access more data items from the upper levels of the memory hierarchy than programs with poor locality, and thus run faster. For example, on our Core i7 system, the running times of different matrix multiplication kernels that perform the same number of arithmetic operations, but have different degrees of locality, can vary by a factor of almost 40!

这个思想围绕着计算机程序的一个称为局部性（locality）的基本属性。具有良好局部性的程序倾向于一次又一次地访问相同的数据项集合，或是倾向于访问临近的数据项集合。具有良好局部性的程序比局部性差的程序更多地倾向于从存储器层次结构中较高层次处访问数据项，因此运行得更快。例如，在 Core i7 系统，不同的矩阵乘法核心程序执行相同数量的算术操作，但是有不同程度的局部性，他们的运行时间可以相差 40 倍！

In this chapter, we will look at the basic storage technologies—SRAM memory, DRAM memory, ROM memory, and rotating and solid state disks—and describe how they are organized into hierarchies. In particular, we focus on the cache memories that act as staging areas between the CPU and main memory, because they have the most impact on application program performance. We show you how to analyze your C programs for locality, and we introduce techniques for improving the locality in your programs. You will also learn an interesting way to characterize the performance of the memory hierarchy on a particular machine as a "**memory mountain**" that shows read access times as a function of locality.

在本章中，我们会看看基本的存储技术 —— SRAM 存储器、DRAM 存储器、ROM 存储器以及旋转的和固态硬盘 —— 并描述它们是如何被组织成层次结构的。特别地，我们将注意力集中在高速缓存存储器上，它是作为 CPU 和主存之间的缓存区域，因为它们对应用程序性能的影响最大。我们想你展示如何分析C 程序的局部性，并且介绍改进你的程序中局部性的计数。你还会学到一种描绘某台机器上存储器层次结构的性能的有趣方法，称为“存储器山（memory mountain）”，它展示出读访问时间是局部性的一个函数。

## 6.1 Storage Technologies

Much of the success of computer technology stems from the tremendous progress in storage technology. Early computers had a few kilobytes of random access memory. The earliest IBM PCs didn’t even have a hard disk. That changed with the introduction of the IBM PC-XT in 1982, with its 10-megabyte disk.By the year 2015, typical machines had 300,000 times as much disk storage, and the amount of storage was increasing by a factor of 2 every couple of years.

计算机技术的成功很大程度上源自于存储技术的巨大进步。早起的计算机只有几千字节的随机访问存储器。最早的IBM PC 甚至没有硬盘。1982 年引入的 IBM PC-XT 有 10 M字节的磁盘。到 2015年，典型的计算机已有 300,000 倍于 PC-XT 的磁盘存储，而且磁盘的容量以每年两倍的速度增长。

### 6.1.1 Random Access Memory

*Random access memory (RAM)* comes in two varieties — static and dynamic. Static RAM (SRAM) is faster and significantly more expensive than dynamic RAM (DRAM). SRAM is used for cache memories, both on and off the CPU chip. DRAM is used for the main memory plus the frame buffer of a graphics system. Typically, a desktop system will have no more than a few tens of megabytes of SRAM, but hundreds or thousands of megabytes of DRAM.

随机访问存储器分为两类：静态的和动态的。静态RAM（SRAM）比动态RAM（DRAM）更快，但也贵得多。SRAM 用来作为高速缓存存储器，既可以在CPU芯片上，也可以在片下。DRAM 用来作为主存以及图形系统的帧缓冲区。典型地，一个桌面系统的 SRAM 不会超过几兆字节，但是 DRAM 却又几百或几千兆字节。

#### Static RAM

SRAM stores each bit in a bistable memory cell. Each cell is implemented with a six-transistor circuit. This circuit has the property that it can stay indefinitely in either of two different voltage configurations, or states. Any other state will be unstable—starting from there, the circuit will quickly move toward one of the stable states. Such a memory cell is analogous to the inverted pendulum illustrated in Figure 6.1.

SRAM 将每个位存储在一个双稳态的（bistable）存储器单元里。每个单元是用一个六晶体管电路来实现的。这个电路有这样一个属性，它可以无限期地保持在两个不同的电压配置或状态之一。其他任何状态都是不稳定的 —— 从不稳定状态开始，电路会迅速地转移到两个稳定状态中的一个。这样一个存储器单元类似于 图 6-1 中画出的倒转的钟摆。

![1](CHAPTER-06/csapp3e-06-01.png)

*Figure 6.1 Inverted pendulum. Like an SRAM cell, the pendulum has only two stable configurations, or states.*

The pendulum is stable when it is tilted either all the way to the left or all the way to the right. From any other position, the pendulum will fall to one side or the other. In principle, the pendulum could also remain balanced in a vertical position indefinitely, but this state is metastable — the smallest disturbance would make it start to fall, and once it fell it would never return to the vertical position.

当钟摆倾斜到最左边或最右边时，它是稳定的。从其他任何位置，钟摆都会导向一边或者另一边。原则上，钟摆也能在垂直的位置无限期地保持平衡，但是这个状态是 亚稳态（metastable）的 —— 最细微的扰动也能使它倒下，而且一旦倒下就永远不会再恢复到垂直位置。

Due to its bistable nature, an SRAM memory cell will retain its value indefinitely, as long as it is kept powered. Even when a disturbance, such as electrical noise, perturbs the voltages, the circuit will return to the stable value when the disturbance is removed.

由于SDRAM 存储器单元的双稳态特性，只要有电，它就会永远地保持它的值。即使有干扰（例如电噪音）来扰乱电压，当干扰消除时，电路就会恢复到稳定值。

#### Dynamic RAM

DRAM stores each bit as charge on a capacitor. This capacitor is very small—typically around 30 femtofarads — that is, $30 \times 10 ^ {−15}$ farads. Recall, however, that a farad is a very large unit of measure. DRAM storage can be made very denseeach cell consists of a capacitor and a single access transistor. Unlike SRAM, however, a DRAM memory cell is very sensitive to any disturbance. When the capacitor voltage is disturbed, it will never recover. Exposure to light rays will cause the capacitor voltages to change. In fact, the sensors in digital cameras and camcorders are essentially arrays of DRAM cells.

DRAM 将每个位存储为对一个电容的充电，这个电容非常小，通常只有大约 30 毫微微法拉（femtofarad）—— $30 \times 10 ^ {−15}$ 法拉。不过，回想一下法拉是一个非常大的计量单位。DRAM存储器可以制造得非常密集 —— 每个单元由一个电容和一个访问晶体管组成。但是，与SRAM 不同，DRAM存储器单元对干扰非常敏感。当电容的电压被扰乱之后它就永远不会恢复了。暴露在光线下回导致电容电压改变。实际上，数码照相机和摄像机中的传感器本质上就是DRAM单元的阵列。

Various sources of leakage current cause a DRAM cell to lose its charge within a time period of around 10 to 100 milliseconds. Fortunately, for computers operating with clock cycle times measured in nanoseconds, this retention time is quite long. The memory system must periodically refresh every bit of memory by reading it out and then rewriting it. Some systems also use error-correcting codes, where the computer words are encoded using a few more bits (e.g., a 64-bit word might be encoded using 72 bits), such that circuitry can detect and correct any single erroneous bit within a word.

很多原因会导致漏电，是的DRAM 单元在 10 ~ 100 毫秒事件内失去电荷。幸运的是，计算机运行的时钟周期是以纳秒来衡量的，所以相对而言这个保持时间是比较长的。内存系统必须周期性地通过读出，然后重写来刷新内存每一位。有些系统也使用纠错码，其中计算机的字会被多编码几个位（例如 64 位的字可能用 72 位来编码），这样一来，电路可以发现并纠正一个字中任何单位的错误位。

Figure 6.2 summarizes the characteristics of SRAM and DRAM memory. SRAM is persistent as long as power is applied. Unlike DRAM, no refresh is necessary. SRAM can be accessed faster than DRAM. SRAM is not sensitive to disturbances such as light and electrical noise. The trade-off is that SRAM cells use more transistors than DRAM cells and thus have lower densities, are more expensive, and consume more power.

图 6.2 总结了 SRAM 和 DRAM 存储器的特性。只要有供电，SRAM 就会保持不变。与 DRAM 不同，它不需要刷新。SRAM 的存取比 DRAM快。 SRAM 对诸如光和电噪声这样的干扰不敏感。代价是 SRAM 单元比 DRAM 单元使用更多的晶体管，因而密集度低，而且更贵，功耗更大。

|                     | Transistors per bit | Relative access time | Persistent? | Sensitive? | Relative cost | Applications              |
|---------------------|---------------------|----------------------|-------------|------------|---------------|---------------------------|
|SRAM                 | 6                   | 1×                   | Yes         | No         | 1,000×        | Cache memory              |
|DRAM                 | 1                   | 10×                  | No          | Yes        | 1×            | Main memory, frame buffers|

*Figure 6.2 Characteristics of DRAM and SRAM memory.*

#### Conventional DRAMs

The cells (bits) in a DRAM chip are partitioned into d supercells, each consisting of w DRAM cells. A d × w DRAM stores a total of dw bits of information. The supercells are organized as a rectangular array with r rows and c columns, where rc = d. Each supercell has an address of the form (i, j), where i denotes the row and j denotes the column.

DRAM 芯片中的单元 （位）被分成了 $d$ 个超单元（supercell），每个超单元都有 $w$ 个 DRAM 单元组成。一个 $d \times w$ 的 DRAM 总共存储了 $dw$ 位信息。超单元被组织成一个 $r$ 行 $c$ 列的长方形阵列，这里 $rc = d$。每个超单元有形如 $(i, j)$的地址，这里 $i$ 表示行，而 $j$ 表示列

For example, Figure 6.3 shows the organization of a 16 × 8 DRAM chip with d = 16 supercells, w = 8 bits per supercell, r = 4 rows, and c = 4 columns. The shaded box denotes the supercell at address (2, 1). Information flows in and out of the chip via external connectors called pins. Each pin carries a 1-bit signal. Figure 6.3 shows two of these sets of pins: eight data pins that can transfer 1 byte in or out of the chip, and two addr pins that carry two-bit row and column supercell addresses. Other pins that carry control information are not shown.

例如，图 6.3 展示的是一个$16 \times 8$ 的DRAM芯片的组织，有 $d = 16$ 个超单元，每个超单元有 $w = 8$位，$r = 4$行，$c = 4$列。带阴影的方框表示地址$(2, 1)$ 处的超单元，信息通过称为引脚（pin） 的外部连接器流入和流出芯片。每个引脚携带一个 1 为的信号。图 6.3 给出了两组引脚：8个 data 引脚，它们能传送一个字节到芯片或从芯片传出一个字节，以及 2 个 addr 引脚，它们携带 2 位的行和列超单元地址。其他携带控制信息的引脚没有显示出来。

Each DRAM chip is connected to some circuitry, known as the memory controller, that can transfer $w$ bits at a time to and from each DRAM chip. To read the contents of supercell (i, j), the memory controller sends the row address i to the DRAM, followed by the column address j. The DRAM responds by sending the contents of supercell (i, j) back to the controller. The row address i is called a RAS (row access strobe) request. The column address j is called a CAS (column access strobe) request. Notice that the RAS and CAS requests share the same DRAM address pins.

每个 DRAM 芯片被连接到某个称为内存控制器（memory controller）的电路，这个电路可以一次传输 $w$ 位到每个 DRAM 芯片，或者一次从每个 DRAM 芯片传出$w$位。为了读出超单元$(i, j)$的内容，内存控制器将行地址 i 发送到 DRAM，然后是列地址 j。 DRAM吧超单元$(i, j)$的内容发送回给控制器作为相应。行地址 i 称为 RAS （Row Access Strobe 行访问选通脉冲）请求。列地址 j 称为 CAS （Column Access Strobe，列访问选通脉冲）请求。注意 RAS和CAS请求共享相同的 DRAM 引脚。

For example, to read supercell(2, 1) from the 16 × 8 DRAM in Figure 6.3, the memory controller sends row address 2, as shown in Figure 6.4(a). The DRAM responds by copying the entire contents of row 2 into an internal row buffer. Next, the memory controller sends column address 1, as shown in Figure 6.4(b). The DRAM responds by copying the 8 bits in supercell (2, 1) from the row buffer and sending them to the memory controller.

例如，要从图 6.3 中 $16 \times 8$ 的 DRAM 中读出超单元$(2, 1)$，内存控制器发送行地址 2，如图6.4a 所示。DRAM 的相应是将行 2 的整个内存都复制到一个内部行缓冲区。接下来，内存控制器发送地址 1，如图 6.4b 所示。DRAM的响应是从行缓冲区复制出超单元 $(2, 1)$ 中的 8 位，并把他们发送到内存控制器。

One reason circuit designers organize DRAMs as two-dimensional arrays instead of linear arrays is to reduce the number of address pins on the chip. For example, if our example 128-bit DRAM were organized as a linear array of 16 supercells with addresses 0 to 15, then the chip would need four address pins instead of two. The disadvantage of the two-dimensional array organization is that addresses must be sent in two distinct steps, which increases the access time.

电路设计者将DRAM组织成二维阵列而不是线性数组的一个原因是降低芯片上地址引脚的数量。例如，如果实例的 128位 DRAM 被组织成一个 16 个超单元的线性数组，地址为 0 ~ 15，那么芯片会需要 4 个地址引脚 而不是 2个。二维阵列组织的缺点是必须分步发送地址，这增加了访问时间。

>**Aside** A note on terminology
>
>The storage community has never settled on a standard name for a DRAM array  element. Computer architects tend to refer to it as a “cell,” overloading the term with the DRAM storage cell. Circuit designers tend to refer to it as a “word,” overloading the term with a word of main memory. To avoid confusion, we have adopted the unambiguous term “supercell.”
>
>存储领域从来没有为DRAM的阵列元素确定一个标准的名字。计算机架构师称之为“单元”，使这个术语具有 DRAM 存储单元之意。电路设计者倾向于称之为“字”，使之具有主存一个字之意。为了避免混淆，我们采用了无歧义的术语“超单元”。

#### Memory Modules

DRAM chips are packaged in memory modules that plug into expansion slots on the main system board(motherboard). Core i7 systems use the 240-pin dual inline memory module (DIMM), which transfers data to and from the memory controller in 64-bit ch unks.

DRAM 芯片封装在内存模块（memory module）中，它插到主板的扩展槽上。 Core i7 系统使用的 240 个引脚的双列直插内存模块（dual inline memory module，DIMM）它以 64 位为块传送数据到内存控制器和从内存控制器传出数据。

Figure 6.5 shows the basic idea of a memory module. The example module stores a total of 64 MB (megabytes) using eight 64-Mbit 8M × 8 DRAM chips, numbered 0 to 7. Each supercell stores 1 byte of main memory, and each 64-bit word at byte address A in main memory is represented by the eight supercells whose corresponding supercell address is (i, j). In the example in Figure 6.5, DRAM 0 stores the first (lower-order) byte, DRAM 1 stores the next byte, and so on.

图 6.5 展示了一个内存模块的基本思想。实例模块用 8 个 64 Mbit 的 $8M \times 8$ 的 DRAM 芯片，总共存储 64MB（兆字节），这 8 个芯片编号为 0 ~ 7。每个超单元存储主存的一个字节，而用相应超单元地址为 $(i, j)$ 的八个超单元来标识主存中字节地址 A 处的64 位字。在图 6.5 的示例中，DRAM 0 存储第一个（低位）字节，DRAM 1 存储下一个字节，以此类推。

To retrieve the word at memory address A, the memory controller converts A to a supercell address (i, j) and sends it to the memory module, which then broadcasts i and j to each DRAM. In response, each DRAM outputs the 8-bit contents of its (i, j) supercell. Circuitry in the module collects these outputs and forms them into a 64-bit word, which it returns to the memory controller.

要取出内存地址 A 处的一个字，内存控制器将 A 转换成一个超单元地址 (i, j)，并将它发送到内存模块，然后内存模块再讲 i 和 j 广播到每个 DRAM。作为相应，每个 DRAM 输出它的(i, j)超单元的 8 位内容。模块中的电路收集这些输出，并把它们合并成一个 64 位字，在返回给内存控制器。

Main memory can be aggregated by connecting multiple memory modules to the memory controller. In this case, when the controller receives an address A, the controller selects the module k that contains A, converts A to its (i, j) form, and sends (i, j) to module k.

通过将多个内存模块连接到内存控制器，能够聚合成主存。在这种情况中，当控制器收到一个地址A时，控制器选择包含 A 的模块 k，将A 转换成它的 (i, j) 的形式，并将 (i, j) 发送到模块 k。

#### Practice Problem 6.1 (solution page 696)

In the following, let r be the number of rows in a DRAM array, c the number of columns, b r the number of bits needed to address the rows, and b c the number of bits needed to address the columns. For each of the following DRAMs, determine the power-of-2 array dimensions that minimize max(b r , b c ),the maximum number of bits needed to address the rows or columns of the array.

#### Enhanced DRAMs

There are many kinds of DRAM memories, and new kinds appear on the market with regularity as manufacturers attempt to keep up with rapidly increasing processor speeds. Each is based on the conventional DRAM cell, with optimizations that improve the speed with which the basic DRAM cells can be accessed.

有许多种DRAM存储器，而生产厂商视图跟上迅速增长的处理器速度，市场上就会定期退出新的种类，每种都是基于传统的 DRAM 单元，并进行一些优化，提高访问基本 DRAM 单元的速度。

* *Fast page mode DRAM (FPM DRAM)*. A conventional DRAM copies an entire row of supercells into its internal row buffer, uses one, and then discards the rest. FPM DRAM improves on this by allowing consecutive accesses to the same row to be served directly from the row buffer. For example, to read four supercells from row i of aconventional DRAM, the memory controller must send four RAS/CAS requests, even though the row address i is identical in each case. To read supercells from the same row of an FPM DRAM, the memory controller sends an initial RAS/CAS request, followed by three CAS requests. The initial RAS/CAS request copies row i into the row buffer and returns the supercell addressed by the CAS. The next three supercells are served directly from the row buffer, and thus are returned more quickly than the initial supercell.
* *缺页模式*。传统的 DRAM 超单元的一整行复制到它的内部行缓冲区中，使用一个，然后丢弃剩余的。FPM DRAM 允许对同一行连续地访问可以直接从行缓冲区得到服务，从而改进了这一点。例如，要从一个传统的 DRAM 的行 i 中读取 4 个超单元，内存控制器必须发送 4 个 RAS/CAS 请求，即使是行地址 i 在每个情况中都是一样的。要从一个 FPM DRAM 的同一行中读取超单元，内存控制器发送第一个 RAS/CAS 请求，后面跟三个 CAS 请求。初始的 RAS/CAS 请求将行 i 复制到行缓冲区，并返回 CAS 寻址的那个超单元。接下来 三个超单元直接从行缓冲区获得，因此返回得比初始单元更快。
* Extended data out DRAM (EDO DRAM). An enhanced form of FPM DRAM that allows the individual CAS signals to be spaced closer together in time.
* 扩展数据输出 DRAM（Extended data out DRAM (EDO DRAM)）,FPM DRAM 的一个增强的形式，它允许各个 CAS 信号在时间上靠得更紧密一点。
* Synchronous DRAM (SDRAM). Conventional, FPM, and EDO DRAMs are asynchronous in the sense that they communicate with the memory controller using a set of explicit control signals. SDRAM replaces many of these control signals with the rising edges of the same external clock signal that drives the memory controller. Without going into detail, the net effect is that an SDRAM can output the contents of its supercells at a faster rate than its asynchronous counterparts.
* 同步 DRAM （Synchronous DRAM (SDRAM)）。就它们与内存控制器通信使用一组显示控制新来来说，常规的，FPM 和 EDO RAM 都是一部的。SDRAM 用于驱动内存控制器相同的外部时钟信号的上升沿来代替许多这样的控制信号。我们不会深入讨论细节，最终效果就是 SDRAM 能够比那些异步的存储器更快地输出它的超单元的内容。
* Double Data-Rate Synchronous DRAM(DDR SDRAM). DDR SDRAM is an enhancement of SDRAM that doubles the speed of the DRAM by using both clock edges as control signals. Different types of DDR SDRAMs are characterized by the size of a small prefetch buffer that increases the effective bandwidth: DDR (2 bits), DDR2 (4 bits), and DDR3 (8 bits).
* 双倍数据速率同步 DRAM （Double Data-Rate Synchronous DRAM(DDR SDRAM)），DDR SDRAM是对SDRAM的一种增强，它通过使用两个时钟沿作为控制信号，从而使DRAM得速度翻倍，不同的DDR SDRAM 是用提高有效带宽的很小的预取缓冲区的大小来划分的：DDR（2位），DDR2（四位）和DDR3（8位）。
* Video RAM (VRAM). Used in the frame buffers of graphics systems. VRAM is similar in spirit to FPM DRAM. Two major differences are that (1) VRAM output is produced by shifting the entire contents of the internal buffer in sequence and (2) VRAM allows concurrent reads and writes to the memory. Thus, the system can be painting the screen with the pixels in the frame buffer (reads) while concurrently writing new values for the next update (writes).
* 视频RAM （Video RAM (VRAM)）。它用在图形系统的帧缓冲区中。VRAM 的思想与FPM DRAM 类似。两个主要区别是：1）VRAM的输出是通过依次对内部缓冲区的整个内容进行位移得到的；2）VRAM 允许对内存并行地读和写。因此，系统可以在写下一次更新的新值（写）的同时，用镇缓冲区中的像素刷屏幕（读）。

>**Aside** Historical popularity of DRAM technologies
>
>Until 1995, most PCs were built with FPM DRAMs. From 1996 to 1999, EDO DRAMs dominated the market, while FPM DRAMs all but disappeared. SDRAMs first appeared in 1995 in high-end systems, and by 2002 most PCs were built with SDRAMs and DDR SDRAMs. By 2010, most server and desktop systems were built with DDR3 SDRAMs. In fact, the Intel Core i7 supports only DDR3 SDRAM.
>
>直到 1995年，大多数PC都是用FPM DRAM构造的。1996 ~ 1999 年， EDO DRAM 在市场上占据了主导，而 FPM DRAM 几乎销声匿迹了。SDRAM 最早出现在 1995 年的高端系统中，到 2002 年，大多数PC都是用 SDRAM 和 DDR SDRAM 制造的。到 2010 年之前，大多数服务器和桌面系统都是用 DDR3 SDRAM 构造的，实际上 Intel Core i7 只支持 DDR3 SDRAM。

#### Nonvolatile Memory

DRAMs and SRAMs are volatile in the sense that they lose their information if the supply voltage is turned off. Nonvolatile memories, on the other hand, retain their information even when they are powered off. There are a variety of nonvolatile memories. For historical reasons, they are referred to collectively as read-only memories (ROMs), even though some types of ROMs can be written to as well as read. ROMs are distinguished by the number of times they can be reprogrammed (written to) and by the mechanism for reprogramming them.

如果断电， DRAM 和 SRAM 会丢失它们的信息，从这个意义上说，它们是易失的（volatile）。另一方面，非易失性存储器（nonvolatile memory）即使实在关电后，仍然保存着它们的信息。现在有很多种非易失性存储器。由于历史原因，虽然ROM中的类型既可以读也可以写，但是它们整体上都被称为只读存储器（Read-Only Memory, ROM）,ROM是以它们能够被重编程（写）的次数和对它们进行重编程所用的机制来区分的。

A programmable ROM (PROM) can be programmed exactly once. PROMs include a sort of fuse with each memory cell that can be blown once by zapping it with a high current.

PROM (programmable ROM，可编程 ROM)只能被编程一次。PROM 的每个存储单元有一种熔丝（fuse），只能用高电流熔断一次。

An erasable programmable ROM (EPROM) has a transparent quartz window that permits light to reach the storage cells. The EPROM cells are cleared to zeros by shining ultraviolet light through the window. Programming an EPROM is done by using a special device to write ones into the EPROM. An EPROM can be erased and reprogrammed on the order of 1,000 times. An electrically erasable PROM (EEPROM) is akin to an EPROM, but it does not require a physically separate programming device, and thus can be reprogrammed in-place on printed circuitcards. An EEPROM can be reprogrammed on the order of $10^5$ times before it wears out.

可擦写可编程ROM（erasable programmable ROM，EPROM）有一个透明的石英窗口，允许光到达存储单元。紫外线光照射过窗口，EPROM单元就被清除为 0。对 EPROM 可编程是通过一种把 1 写入 EPROM 的特殊设备来完成的。EPROM 能够被擦除和重编程的次数数量级可以达到 1000次。电子可擦出 PROM （electrically erasable PROM，EEPROM）类似于EPROM，但是它不需要一个物理上独立的编程设备，因此可以直接在印刷电路卡上编程。EEPROM能够被编程的次数的数量级可以达到 $10 ^ 5$ 次。

Flash memory is a type of nonvolatile memory, based on EEPROMs, that has become an important storage technology. Flash memories are everywhere, providing fast and durable nonvolatile storage for a slew of electronic devices, including digital cameras, cell phones, and music players, as well as laptop, desktop, and server computer systems. In Section 6.1.3, we will look in detail at a new form of flash-based disk drive, known as a solid state disk (SSD), that provides a faster, sturdier, and less power-hungry alternative to conventional rotating disks.

闪存（flash memory）是一类非易失性存储器，基于EEPROM，它已经成为了一种重要的存储技术。闪存无处不在，为大量的电子设备提供快速而持久的非易失性存储，包括数码相机、手机、音乐播放器、PDA和笔记本、台式机和服务器计算机系统。在 6.1.3 节中，我们会自己研究一种新型的基于闪存的磁盘驱动器，称为固态硬盘（solid state disk，SSD），它能提供相对于传统旋转磁盘的一种更快速，更强健和更低能耗的选择。

Programs stored in ROM devices are often referred to as firmware. When a computer system is powered up, it runs firmware stored in a ROM. Some systems provide a small set of primitive input and output functions in firmware—for example, a PC's BIOS (basic input/output system) routines. Complicated devices such as graphics cards and disk drive controllers also rely on firmware to translate I/O (input/output) requests from the CPU.

存储在ROM设备中的程序通常被称为固件（fireware）。当一个计算机系统通电以后，它会运行存储在 ROM 中的固件。一些系统在固件中提供了少量基本的输入和输出函数 —— 例如 PC 的 BIOS （基本输入/输出系统）例程。复杂的设备，像图形卡和磁盘驱动控制器，也依赖固件翻译来自CPU 的 I/O（输入/输出）请求。

#### Accessing Main Memory

Data flows back and forth between the processor and the DRAM main memory over shared electrical conduits called buses. Each transfer of data between the CPU and memory is accomplished with a series of steps called a bus transaction. A read transaction transfers data from the main memory to the CPU. A write transaction transfers data from the CPU to the main memory.

数据流通过称为总线（bus）的共享电子电路在处理器和 DRAM 主存之间来来回回。每次 CPU 和 主存之间的数据传送都是通过一系列步骤来完成的，这些步骤称为总线事务（bus transaction）。读事务（read transaction）从主存传送数据到CPU。写事务（write transaction）从CPU传送数据到主存。

A bus is a collection of parallel wires that carry address, data, and control signals. Depending on the particular bus design, data and address signals can share the same set of wires or can use different sets. Also, more than two devices can share the same bus. The control wires carry signals that synchronize the transaction and identify what kind of transaction is currently being performed. For example, is this transaction of interest to the main memory, or to some other I/O device such as a disk controller? Is the transaction a read or a write? Is the information on the bus an address or a data item?

总线是一组并行的导线，能携带地址、数据和控制信号。取决于总线的设计，数据和地址信号可以共享一组导线，也可以使用不同的。同时，两个以上的设备也能共享同一总线。控制线携带的信号会同步事务，并标识出当前正在被执行的事务的类型。例如，当前关注的这个事务是到主存的吗？还是到诸如磁盘控制器这样的其他I/O设备？这个事务是读还是写？总线上的信息是地址还是数据项？

Figure 6.6 shows the configuration of an example computer system. The main components are the CPU chip, a chipset that we will call an I/O bridge (which includes the memory controller), and the DRAM memory modules that make up main memory. These components are connected by a pair of buses: a system bus that connects the CPU to the I/O bridge, and a memory bus that connects the I/O bridge to the main memory. The I/O bridge translates the electrical signals of the system bus into the electrical signals of the memory bus. As we will see, the I/O bridge also connects the system bus and memory bus to an I/O bus that is shared by I/O devices such as disks and graphics cards. For now, though, we will focus on the memory bus.

图 6.6 展示了一个示例计算机系统的配置。主要部件是CPU芯片、我们将成为 I/O 桥接器 （I/O bridge）的芯片组（其中包括内存控制器），以及组成主存的 DRAM 内存模块。这些部件由一堆总线连接起来，其中一条总线是系统总线（system bus），它连接 CPU 和 I/O 桥接器，另一条是内存总线（memory bus），它连接 I/O 桥接器和主存。I/O 桥接器将系统总线的电子信号翻译成内存总线的电子信号。正如我们看到的那样，I/O 桥接器也将系统总线和内存总线连接到I/O 总线，像磁盘和图形卡这样的 I/O 设备共享 I/O 总线。不过现在我们将注意力集中在内存总线上。

Consider what happens when the CPU performs a load operation such as:

```asm
movq A,%rax
```

where the contents of address A are loaded into register %rax. Circuitry on the CPU chip called the bus interface initiates a read transaction on the bus. The read transaction consists of three steps. First, the CPU places the address A on the system bus. The I/O bridge passes the signal along to the memory bus (Figure 6.7(a)). Next, the main memory senses the address signal on the memory bus, reads the address from the memory bus, fetches the data from the DRAM, and writes the data to the memory bus. The I/O bridge translates the memory bus signal into a system bus signal and passes it along to the system bus (Figure 6.7(b)). Finally, the CPU senses the data on the system bus, reads the data from the bus, and copies the data to register %rax (Figure 6.7(c)).

Conversely, when the CPU performs a store operation such as

```asm
movq %rax,A
```

where the contents of register %rax are written to address A, the CPU initiates a write transaction. Again, there are three basic steps. First, the CPU places the address on the system bus. The memory reads the address from the memory bus and waits for the data to arrive (Figure 6.8(a)). Next, the CPU copies the data in %rax to the system bus (Figure 6.8(b)). Finally, the main memory reads the data from the memory bus and stores the bits in the DRAM (Figure 6.8(c)).

>Aside A note on bus designs
>
>Bus design is a complex and rapidly changing aspect of computer systems. Different vendors develop different bus architectures as a way to differentiate their products. For example, some Intel systems use chipsets known as the northbridge and the southbridge to connect the CPU to memory and I/O devices, respectively. In older Pentium and Core 2 systems, a front side bus (FSB) connects the CPU to the northbridge. Systems from AMD replace the FSB with the HyperTransport interconnect, while newer Intel Core i7 systems use the QuickPath interconnect. The details of these different bus architectures are beyond the scope of this text. Instead, we will use the high-level bus architecture from Figure 6.6 as a running example throughout. It is a simple but useful abstraction that allows us to be concrete. It captures the main ideas without being tied too closely to the detail of any proprietary designs.
