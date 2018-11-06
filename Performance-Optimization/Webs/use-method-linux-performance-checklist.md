# [USE Method: Linux Performance Checklist](http://www.brendangregg.com/USEmethod/use-linux.html)

The USE Method provides a strategy for performing a complete check of system health, identifying common bottlenecks and errors. For each system resource, metrics for utilization, saturation and errors are identified and checked. Any issues discovered are then investigated using further strategies.

This is an example USE-based metric list for Linux operating systems (eg, Ubuntu, CentOS, Fedora). This is primarily intended for system administrators of the physical systems, who are using command line tools. Some of these metrics can be found in remote monitoring tools.

## CPU utilization

### system-wide `vmstat`

```sh
vmstat 1, "us" + "sy" + "st"
```

The virtual memory statistics command, `vmstat(8)`, prints system-wide CPU averages in the last few columns, and a count of runnable threads in the first column. Here is example output from the Linux version:

虚拟内存统计命令，`vmstat(8)`，在最后几列打印了系统全局范围的 `CPU` 平均负载，另外在第一列还有可运行线程数。下面是 Linux 版本的一个示例输出：

```sh
[]$ vmstat -w 1
procs -----------------------memory---------------------- ---swap-- -----io---- -system-- --------cpu--------
 r  b         swpd         free         buff        cache   si   so    bi    bo   in   cs  us  sy  id  wa  st
 1  0            0     31819056         2244       325256    0    0     6     1   20   17   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  335  275   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  282  261   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  307  271   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  331  309   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  299  257   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  299  271   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  310  269   0   0 100   0   0
 0  0            0     31819056         2244       325256    0    0     0     0  332  312   0   0 100   0   0
```

vmstat 命令行参数

```sh
[]$ vmstat --help

Usage:
 vmstat [options] [delay [count]]

Options:
 -a, --active           active/inactive memory
 -f, --forks            number of forks since boot
 -m, --slabs            slabinfo
 -n, --one-header       do not redisplay header
 -s, --stats            event counter statistics
 -d, --disk             disk statistics
 -D, --disk-sum         summarize disk statistics
 -p, --partition <dev>  partition specific statistics
 -S, --unit <char>      define display unit
 -w, --wide             wide output
 -t, --timestamp        show timestamp

 -h, --help     display this help and exit
 -V, --version  output version information and exit

For more details see vmstat(8).
```

vmstat 关于进程和CPU的帮助信息

```man
     Procs
         r: The number of runnable processes (running or waiting for run time).
         b: The number of processes in uninterruptible sleep.

    CPU
         These are percentages of total CPU time.
         us: Time spent running non-kernel code.| (user time, including nice time)
         sy: Time spent running kernel code.  (system time)
         id: Time spent idle.  Prior to Linux 2.5.41, this includes IO-wait time.
         wa: Time spent waiting for IO.  Prior to Linux 2.5.41, included in idle.
         st: Time stolen from a virtual machine.| Prior to Linux 2.6.11, unknown.
```

The first line of output is the summary-since-boot, with the exception of r on Linux—which begins by showing current values. The columns are：

第一行为摘要信息，在Linux上，这些值显示的是当前值（除了 r）：

| col    |note
|--------|----------------------
|  **r**:| run-queue length—the total number of runnable threads (see below) <br> 运行队列长度 —— 可运行线程的总数（见下）
| **us**:| user-time <br> 用户态时间，非内核时间，包括 nice time（nice 时间）
| **sy**:| system-time (kernel) <br> 内核时间
| **id**:| idle <br> 空闲
| **wa**:| wait I/O, which measures CPU idle when threads are blocked on disk I/O <br> 等待 I/O ，即线程被阻塞等待磁盘 I/O 的 CPU 空闲时间。
| **st**:| stolen (not shown in the output), which for virtualized environments shows CPU time spent servicing other tenants <br>  偷取，CPU 在虚拟化的环境下在其他租户上的开销

All of these values are system-wide averages across all CPUs, with the exception of r, which is the total.

这些值都是所有CPU的系统平均数，除了r，r表示的是总数。

On **Linux**, the `r` column is the total number of tasks waiting plus those running. The man page currently describes it as something else—"the number of processes waiting for run time"—which suggests it counts only those waiting and not running. As insight into what this is supposed to be, the original `vmstat(1)` by *Bill Joy* and *Ozalp Babaoglu* for 3BSD in 1979 begins with an `RQ` column for the number of runnable and running processes, as the Linux `vmstat(8)` currently does. The man page needs updating.

在 Linux 系统上，r 列代表所有等待的加上正在运行的线程数。这与当前手册里的描述不太一样，手册里是这样描述的 —— “等待运行的进程数” —— 这意味着只计算了鞥带中而排出了运行中的任务。为了了解本来应该是什么内存，可以参考最初由 Bill Joy 和 Ozalp Babaoglu 1979年为 3BSD 编写的 `vmstat(1)`，这个版本以 RQ 列开始，代表可运行和正在运行的进程数，和 Linux vmstat(8) 相匹配。这个手册需要更新。

### system-wide `sar`

```sh
sar -u, sum fields except "%idle" and "%iowait"; 
```

The system activity reporter, sar(1), can be used to observe current activity and can be configured to archive and report historical statistics. It was introduced in Chapter 4, Observability Tools, and is mentioned in other chapters as appropriate.

系统活动报告器，`sar(1)` ，可以用来观察当前的活动，以及配置用以归档和报告历史统计信息，第四章曾经介绍过，也在其他章节出现过。

The Linux version provides the following options:

Linux版本有一下选项：

|           |                                                                                                   |
|-----------|---------------------------------------------------------------------------------------------------|
| `-P ALL`: |same as `mpstat -P ALL`
|     `-u`: |same as `mpstat(1)`’s default output: system-wide average only
|     `-q`: |includes run-queue size as runq-sz (waiting plus running, the same as `vmstat`’s `r`) and load averages

### system-wide `dstat`

```sh
dstat -c, sum fields except "idl" and "wai"; 
```

`CentOS 7` 下需要安装 `yum -y install dstat`。

-c 代表只统计 `CPU`

### per-cpu `mpstat`

```sh
mpstat -P ALL 1, sum fields except "%idle" and "%iowait"; 
```

The multiprocessor statistics tool, mpstat, can report statistics per CPU. Here is some example output from the Linux version:

```sh
Linux 3.10.0-862.11.6.el7.x86_64 (localhost.localdomain)        11/06/2018      _x86_64_        (24 CPU)

02:25:13 PM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest  %gnice   %idle
02:25:13 PM  all    0.03    0.00    0.02    0.02    0.00    0.00    0.00    0.00    0.00   99.94
02:25:13 PM    0    0.11    0.00    0.05    0.10    0.00    0.00    0.00    0.00    0.00   99.73
02:25:13 PM    1    0.01    0.00    0.01    0.01    0.00    0.00    0.00    0.00    0.00   99.97
02:25:13 PM    2    0.08    0.00    0.03    0.01    0.00    0.00    0.00    0.00    0.00   99.88
02:25:13 PM    3    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.98
02:25:13 PM    4    0.05    0.00    0.04    0.01    0.00    0.00    0.00    0.00    0.00   99.90
02:25:13 PM    5    0.01    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.97
02:25:13 PM    6    0.10    0.00    0.04    0.02    0.00    0.00    0.00    0.00    0.00   99.84
02:25:13 PM    7    0.01    0.00    0.01    0.02    0.00    0.00    0.00    0.00    0.00   99.97
02:25:13 PM    8    0.09    0.00    0.04    0.01    0.00    0.00    0.00    0.00    0.00   99.86
02:25:13 PM    9    0.00    0.00    0.01    0.01    0.00    0.00    0.00    0.00    0.00   99.98
02:25:13 PM   10    0.10    0.00    0.03    0.01    0.00    0.00    0.00    0.00    0.00   99.85
02:25:13 PM   11    0.01    0.00    0.00    0.02    0.00    0.00    0.00    0.00    0.00   99.97
02:25:13 PM   12    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.99
02:25:13 PM   13    0.00    0.00    0.00    0.02    0.00    0.00    0.00    0.00    0.00   99.97
02:25:13 PM   14    0.02    0.00    0.04    0.02    0.00    0.00    0.00    0.00    0.00   99.92
02:25:13 PM   15    0.02    0.00    0.04    0.01    0.00    0.00    0.00    0.00    0.00   99.93
02:25:13 PM   16    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.98
02:25:13 PM   17    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.98
02:25:13 PM   18    0.03    0.00    0.02    0.01    0.00    0.00    0.00    0.00    0.00   99.95
02:25:13 PM   19    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.98
02:25:13 PM   20    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.99
02:25:13 PM   21    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.99
02:25:13 PM   22    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.99
02:25:13 PM   23    0.00    0.00    0.00    0.01    0.00    0.00    0.00    0.00    0.00   99.98
```

### per-cpu `sar`

sar -P ALL, same as mpstat; 

per-process: 

top, "%CPU"; 

htop, "CPU%"; 

ps -o pcpu; 

pidstat 1, "%CPU"; 

per-kernel-thread: 

top/htop ("K" to toggle), where VIRT == 0 (heuristic). There can be some oddities with the %CPU from top/htop in virtualized environments; I'll update with details later when I can.

