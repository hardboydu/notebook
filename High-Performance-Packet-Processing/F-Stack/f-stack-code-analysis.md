# F-Stack Code Analysis

FreeBSD 协议栈的用户态一直的演进是这样的：

```conf
+--------------+      +--------------+      +--------------+
|  libplebnet  |----->|  libuinet    |----->|  f-stack     |
+--------------+      +--------------+      +--------------+
```

* [libplebnet](https://gitorious.org/freebsd/kmm-sandbox/commit/fa8a11970bc0ed092692736f175925766bebf6af?p=freebsd:kmm-sandbox.git;a=tree;f=lib/libplebnet;h=ae446dba0b4f8593b69b339ea667e12d5b709cfb;hb=refs/heads/work/svn_trunk_libplebnet) . 基于 FreeBSD 8.x 内核
* [libuinet](https://github.com/pkelsey/libuinet) . 基于 FreeBSD 9.1-RELEASE 内核
* [F-Stack](https://github.com/F-Stack/f-stack) . 基于 FreeBSD 11.0 内核

代码的分析以 `F-Stack` 为主，`libuinet` 次之，然后再参考 `libplebnet`

## 代码结构

通过分析 F-Stack/lib/Makefile 代码分成两个部分，**Kernel** 和 **Host**：

|name    | notes                                                                  |
|-------:|------------------------------------------------------------------------|
|Kernel  | 指的是 FreeBSD Kernel，它的代码包含了部分 FreeBSD 内核代码和 必要的替代代码|
|Host    | 主要是运行期 FreeBSD Kernel 的一些支持文件，包括调用接口，回调接口，配置等等 |


```conf
              ff_freebsd_init.c
              ff_veth.c
              ff_syscall_wrapper.c
+---------+   <---------------  +---------+
|         |                     |         |
|         |                     |         |
|         |                     |         |
|         |                     |         |
| Kernel  |                     | Host    |
|         |                     |         |
|         |                     |         |
|         |                     |         |
|         |                     |         |
|         |                     |         |
+---------+   --------------->  +---------+
      ff_glue.c --> ff_host_interface.c
```

由上图可知 Host 通过 `ff_freebsd_init.c`、`ff_veth.c`、`ff_syscall_wrapper.c` 提供的接口调用 Kernel 中的功能，而 Kernel 通过 `ff_glue.c` 调用 `ff_host_interface.c` 提供的 Host 功能。

## 代码编译

代码编译分为两个部分，Kernel 部分和 Host 部分：

kernel 部分的编译

```make
${ASM_OBJS}: %.o: %.S ${IMACROS_FILE}
    ${NORMAL_S}

${OBJS}: %.o: %.c ${IMACROS_FILE}
    ${NORMAL_C}
```

Host 部分的编译

```make
${HOST_OBJS}: %.o: %.c
    ${HOST_C}
```

链接也分为两个部分

Kernel 部分

```make
#
# The library is built by first incrementally linking all the object
# to resolve internal references.  Then, all symbols are made local.
# Then, only the symbols that are part of the  API are made
# externally available.
#
libfstack.a: machine_includes ff_api.symlist MHEADERS{MSRCS} HOSTOBJS{ASM_OBJS} ${OBJS}
    ${LD} -d -r -o $*.ro ${ASM_OBJS} ${OBJS}
    nm $*.ro  | grep -v ' U ' | cut -d ' ' -f 3 > $*_localize_list.tmp
    objcopy --localize-symbols=$*_localize_list.tmp $*.ro 
    rm $*_localize_list.tmp
    objcopy --globalize-symbols=ff_api.symlist $*.ro
    rm -f $@
```

Host 部分

```make
    ar -cqs $@ $*.ro ${HOST_OBJS}
    rm -f $*.ro
```
