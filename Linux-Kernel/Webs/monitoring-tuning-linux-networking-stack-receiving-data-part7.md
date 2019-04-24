# [Monitoring and Tuning the Linux Networking Stack: Receiving Data Part5 SoftIRQs](https://blog.packagecloud.io/eng/2016/06/22/monitoring-tuning-linux-networking-stack-receiving-data/)

## SoftIRQs

Before examining the network stack, we’ll need to take a short detour to examine something in the Linux kernel called SoftIRQs.

### What is a softirq ?

The softirq system in the Linux kernel is a mechanism for executing code outside of the context of an interrupt handler implemented in a driver. This system is important because hardware interrupts may be disabled during all or part of the execution of an interrupt handler. The longer interrupts are disabled, the greater chance that events may be missed. So, it is important to defer any long running actions outside of the interrupt handler so that it can complete as quickly as possible and re-enable interrupts from the device.

There are other mechanisms that can be used for deferring work in the kernel, but for the purposes of the networking stack, we’ll be looking at softirqs.

The softirq system can be imagined as a series of kernel threads (one per CPU) that run handler functions which have been registered for different softirq events. If you’ve ever looked at top and seen ksoftirqd/0 in the list of kernel threads, you were looking at the softirq kernel thread running on CPU 0.

Kernel subsystems (like networking) can register a softirq handler by executing the open_softirq function. We’ll see later how the networking system registers its softirq handlers. For now, let’s learn a bit more about how softirqs work.

### `ksoftirqd`

Since softirqs are so important for deferring the work of device drivers, you might imagine that the ksoftirqd process is spawned pretty early in the life cycle of the kernel and you’d be correct.

Looking at the code found in kernel/softirq.c reveals how the ksoftirqd system is initialized:

```c
static struct smp_hotplug_thread softirq_threads = {
  .store              = &ksoftirqd,
  .thread_should_run  = ksoftirqd_should_run,
  .thread_fn          = run_ksoftirqd,
  .thread_comm        = "ksoftirqd/%u",
};

static __init int spawn_ksoftirqd(void)
{
  register_cpu_notifier(&cpu_nfb);

  BUG_ON(smpboot_register_percpu_thread(&softirq_threads));

  return 0;
}
early_initcall(spawn_ksoftirqd);
```

As you can see from the struct smp_hotplug_thread definition above, there are two function pointers being registered: ksoftirqd_should_run and run_ksoftirqd.

Both of these functions are called from kernel/smpboot.c as part of something which resembles an event loop.

The code in kernel/smpboot.c first calls ksoftirqd_should_run which determines if there are any pending softirqs and, if there are pending softirqs, run_ksoftirqd is executed. The run_ksoftirqd does some minor bookkeeping before it calls __do_softirq.

### `__do_softirq`

The __do_softirq function does a few interesting things:

* determines which softirq is pending
* softirq time is accounted for statistics purposes
* softirq execution statistics are incremented
* the softirq handler for the pending softirq (which was registered with a call to open_softirq) is executed.

So, when you look at graphs of CPU usage and see softirq or si you now know that this is measuring the amount of CPU usage happening in a deferred work context.

### Monitoring

#### `/proc/softirqs`

The softirq system increments statistic counters which can be read from /proc/softirqs Monitoring these statistics can give you a sense for the rate at which softirqs for various events are being generated.

Check softIRQ stats by reading /proc/softirqs.

```sh
$ cat /proc/softirqs
                    CPU0       CPU1       CPU2       CPU3
          HI:          0          0          0          0
       TIMER: 2831512516 1337085411 1103326083 1423923272
      NET_TX:   15774435     779806     733217     749512
      NET_RX: 1671622615 1257853535 2088429526 2674732223
       BLOCK: 1800253852    1466177    1791366     634534
BLOCK_IOPOLL:          0          0          0          0
     TASKLET:         25          0          0          0
       SCHED: 2642378225 1711756029  629040543  682215771
     HRTIMER:    2547911    2046898    1558136    1521176
         RCU: 2056528783 4231862865 3545088730  844379888
```

This file can give you an idea of how your network receive (NET_RX) processing is currently distributed across your CPUs. If it is distributed unevenly, you will see a larger count value for some CPUs than others. This is one indicator that you might be able to benefit from Receive Packet Steering / Receive Flow Steering described below. Be careful using just this file when monitoring your performance: during periods of high network activity you would expect to see the rate NET_RX increments increase, but this isn’t necessarily the case. It turns out that this is a bit nuanced, because there are additional tuning knobs in the network stack that can affect the rate at which NET_RX softirqs will fire, which we’ll see soon.

You should be aware of this, however, so that if you adjust the other tuning knobs you will know to examine /proc/softirqs and expect to see a change.

Now, let’s move on to the networking stack and trace how network data is received from top to bottom.