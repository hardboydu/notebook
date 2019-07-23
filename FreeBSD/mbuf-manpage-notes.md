
## NAME

mbuf -- memory management in the kernel IPC subsystem

## SYNOPSIS

```c
     #include <sys/param.h>
     #include <sys/systm.h>
     #include <sys/mbuf.h>
```

### Mbuf allocation macros

```c
     MGET(struct mbuf *mbuf, int how, short type);

     MGETHDR(struct mbuf *mbuf, int how, short type);

     int
     MCLGET(struct mbuf *mbuf, int how);

     MEXTADD(struct mbuf *mbuf, char *buf, u_int size,
        void (*free)(struct mbuf *), void *opt_arg1, void *opt_arg2,
        int flags, int type);
```

### Mbuf utility macros

```c
     mtod(struct mbuf *mbuf, type);

     M_ALIGN(struct mbuf *mbuf, u_int len);

     MH_ALIGN(struct mbuf *mbuf, u_int len);

     int
     M_LEADINGSPACE(struct mbuf *mbuf);

     int
     M_TRAILINGSPACE(struct mbuf *mbuf);

     M_MOVE_PKTHDR(struct mbuf *to, struct mbuf *from);

     M_PREPEND(struct mbuf *mbuf, int len, int how);

     MCHTYPE(struct mbuf *mbuf, short type);

     int
     M_WRITABLE(struct mbuf *mbuf);
```

### Mbuf allocation functions

```c
     struct mbuf *
     m_get(int how, short type);

     struct mbuf *
     m_get2(int size, int how, short type, int flags);

     struct mbuf *
     m_getm(struct mbuf *orig, int len, int how, short type);

     struct mbuf *
     m_getjcl(int how, short type, int flags, int size);

     struct mbuf *
     m_getcl(int how, short type, int flags);

     struct mbuf *
     m_gethdr(int how, short type);

     struct mbuf *
     m_free(struct mbuf *mbuf);

     void
     m_freem(struct mbuf *mbuf);
```

### Mbuf utility functions

```c
     void
     m_adj(struct mbuf *mbuf, int len);

     void
     m_align(struct mbuf *mbuf,	int len);

     int
     m_append(struct mbuf *mbuf, int len, c_caddr_t cp);

     struct mbuf *
     m_prepend(struct mbuf *mbuf, int len, int how);

     struct mbuf *
     m_copyup(struct mbuf *mbuf, int len, int dstoff);

     struct mbuf *
     m_pullup(struct mbuf *mbuf, int len);

     struct mbuf *
     m_pulldown(struct mbuf *mbuf, int offset, int len,	int *offsetp);

     struct mbuf *
     m_copym(struct mbuf *mbuf,	int offset, int	len, int how);

     struct mbuf *
     m_copypacket(struct mbuf *mbuf, int how);

     struct mbuf *
     m_dup(const struct mbuf *mbuf, int how);

     void
     m_copydata(const struct mbuf *mbuf, int offset, int len, caddr_t buf);

     void
     m_copyback(struct mbuf *mbuf, int offset, int len,	caddr_t	buf);

     struct mbuf *
     m_devget(char *buf, int len, int offset, struct ifnet *ifp,
        void (*copy)(char *from, caddr_t to, u_int len));

     void
     m_cat(struct mbuf *m, struct mbuf *n);

     void
     m_catpkt(struct mbuf *m, struct mbuf *n);

     u_int
     m_fixhdr(struct mbuf *mbuf);

     int
     m_dup_pkthdr(struct mbuf *to, const struct	mbuf *from, int	how);

     void
     m_move_pkthdr(struct mbuf *to, struct mbuf	*from);

     u_int
     m_length(struct mbuf *mbuf, struct	mbuf **last);

     struct mbuf *
     m_split(struct mbuf *mbuf,	int len, int how);

     int
     m_apply(struct mbuf *mbuf,	int off, int len,
	 int (*f)(void *arg, void *data, u_int len), void *arg);

     struct mbuf *
     m_getptr(struct mbuf *mbuf, int loc, int *off);

     struct mbuf *
     m_defrag(struct mbuf *m0, int how);

     struct mbuf *
     m_collapse(struct mbuf *m0, int how, int maxfrags);

     struct mbuf *
     m_unshare(struct mbuf *m0,	int how);
```

## DESCRIPTION

An mbuf is a basic unit of memory management in the kernel IPC subsystem. Network packets and socket buffers are stored in mbufs. A network packet may span multiple mbufs arranged into a mbuf chain (linked list), which allows adding or trimming network headers with little overhead.

mbuf是内核IPC子系统中内存管理的基本单元。网络数据包和套接字缓冲区存储在`mbuf`中。 网络数据包可以跨越排列成 `mbuf` 链（链表）的多个 `mbuf`，这允许以较小的开销添加或修剪网络头。

While a developer should not bother with mbuf internals without serious reason in order to avoid incompatibilities with future changes, it is useful to understand the general structure of an mbuf.

虽然开发人员不应该在没有严重理由的情况下使用mbuf内部组件，以避免与未来的更改不兼容，但了解 `mbuf` 的一般结构很有用。

An `mbuf` consists of a variable-sized header and a small internal buffer for data. The total size of an `mbuf`, `MSIZE`, is a constant defined in `<sys/param.h>`. The `mbuf` header includes:

`mbuf` 由一个可变大小的头和一个用于数据的小内部缓冲区组成。 `mbuf`，`MSIZE`的总大小是`<sys/param.h>` 中定义的常量。`mbuf` 头包括：

| | |
|-|-|
|`m_next`|     `(struct mbuf *)` A pointer to the next mbuf in the mbuf chain.|
|`m_nextpkt`|  `(struct mbuf *)` A pointer to the next mbuf chain in the queue.|
|`m_data`|     `(caddr_t)` A pointer to data attached to this mbuf.|
|`m_len`|      `(int)` The length of the data.|
|`m_type`|     `(short)` The type of the data.|
|`m_flags`|    `(int)` The mbuf flags.|

The mbuf flag bits are defined as follows:

```c
     /*	mbuf flags */
     #define M_EXT	     0x00000001	/* has associated external storage */
     #define M_PKTHDR	     0x00000002	/* start of record */
     #define M_EOR	     0x00000004	/* end of record */
     #define M_RDONLY	     0x00000008	/* associated data marked read-only */
     #define M_PROTO1	     0x00001000	/* protocol-specific */
     #define M_PROTO2	     0x00002000	/* protocol-specific */
     #define M_PROTO3	     0x00004000	/* protocol-specific */
     #define M_PROTO4	     0x00008000	/* protocol-specific */
     #define M_PROTO5	     0x00010000	/* protocol-specific */
     #define M_PROTO6	     0x00020000	/* protocol-specific */
     #define M_PROTO7	     0x00040000	/* protocol-specific */
     #define M_PROTO8	     0x00080000	/* protocol-specific */
     #define M_PROTO9	     0x00100000	/* protocol-specific */
     #define M_PROTO10	     0x00200000	/* protocol-specific */
     #define M_PROTO11	     0x00400000	/* protocol-specific */
     #define M_PROTO12	     0x00800000	/* protocol-specific */

     /*	mbuf pkthdr flags (also	stored in m_flags) */
     #define M_BCAST	     0x00000010	/* send/received as link-level broadcast */
     #define M_MCAST	     0x00000020	/* send/received as link-level multicast */

     The available mbuf	types are defined as follows:

     /*	mbuf types */
     #define MT_DATA	     1	     /*	dynamic	(data) allocation */
     #define MT_HEADER	     MT_DATA /*	packet header */
     #define MT_SONAME	     8	     /*	socket name */
     #define MT_CONTROL	     14	     /*	extra-data protocol message */
     #define MT_OOBDATA	     15	     /*	expedited data */

     The available external buffer types are defined as	follows:

     /*	external buffer	types */
     #define EXT_CLUSTER     1	     /*	mbuf cluster */
     #define EXT_SFBUF	     2	     /*	sendfile(2)'s sf_bufs */
     #define EXT_JUMBOP	     3	     /*	jumbo cluster 4096 bytes */
     #define EXT_JUMBO9	     4	     /*	jumbo cluster 9216 bytes */
     #define EXT_JUMBO16     5	     /*	jumbo cluster 16184 bytes */
     #define EXT_PACKET	     6	     /*	mbuf+cluster from packet zone */
     #define EXT_MBUF	     7	     /*	external mbuf reference	*/
     #define EXT_NET_DRV     252     /*	custom ext_buf provided	by net driver(s) */
     #define EXT_MOD_TYPE    253     /*	custom module's	ext_buf	type */
     #define EXT_DISPOSABLE  254     /*	can throw this buffer away w/page flipping */
     #define EXT_EXTREF	     255     /*	has externally maintained ref_cnt ptr */
```

If the `M_PKTHDR` flag is set, a `struct pkthdr m_pkthdr` is added to the mbuf header.  It contains a pointer to the interface the packet has been received from `(struct ifnet *rcvif)`, and the total packet length `(int len)`. Optionally, it may also contain an attached list of packet tags `(struct m_tag)`.  See `mbuf_tags(9)` for details. Fields used in offloading checksum calculation to the hardware are kept in m_pkthdr as well. See `HARDWARE-ASSISTED` `CHECKSUM CALCULATION` for details.

如果设置了`M_PKTHDR`标志，则将`struct pkthdr m_pkthdr`添加到mbuf头中。 它包含一个指向从`（struct ifnet * rcvif）`接收数据包的接口的指针，以及总包长度`（int len）`。可选地，它还可以包含附加的包标签列表`（struct m_tag）`。有关详细信息，请参阅`mbuf_tags（9）`。 将校验和计算卸载到硬件中的字段也保存在m_pkthdr中。 有关详细信息，请参阅“硬件辅助”`CHECKSUM CALCULATION`。

If small enough, data is stored in the internal data buffer of an `mbuf`. If the data is sufficiently large, another `mbuf` may be added to the `mbuf` chain, or external storage may be associated with the mbuf. `MHLEN` bytes of data can fit into an `mbuf` with the `M_PKTHDR` flag set, `MLEN` bytes can otherwise.

如果足够小，数据将存储在 `mbuf` 的内部数据缓冲区中。如果数据足够大，可以将另一个mbuf添加到`mbuf`链，或者外部存储可以与 `mbuf` 相关联。`MHLEN`字节数据可以装入`mbuf`并设置`M_PKTHDR`标志，否则 `MLEN` 字节可以。

If external storage is being associated with an `mbuf`, the `m_ext` header is added at the cost of losing the internal data buffer. It includes a pointer to external storage, the size of the storage, a pointer to a function used for freeing the storage, a pointer to an optional argument that can be passed to the function, and a pointer to a reference counter. An `mbuf` using external storage has the `M_EXT` flag set.

如果外部存储与 `mbuf` 相关联，则以丢失内部数据缓冲区为代价添加`m_ext`头。它包括一个指向外部存储器的指针，存储器的大小，用于释放存储器的函数的指针，指向可以传递给函数的可选参数的指针，以及指向引用计数器的指针。使用外部存储器的 `mbuf` 设置了 `M_EXT` 标志。

The system supplies a macro for allocating the desired external storage buffer, `MEXTADD`.

系统提供一个宏，用于分配所需的外部存储缓冲区，`MEXTADD`。

The allocation and management of the reference counter is handled by the subsystem.

参考计数器的分配和管理由子系统处理。

The system also supplies a default type of external storage buffer called an `mbuf cluster`. `Mbuf clusters` can be allocated and configured with the use of the `MCLGET` macro. Each `mbuf cluster` is `MCLBYTES` in size, where `MCLBYTES` is a machine-dependent constant. The system defines an advisory macro `MINCLSIZE`, which is the smallest amount of data to put into an `mbuf cluster`. It is equal to `MHLEN` plus one. It is typically preferable to store data into the data region of an `mbuf`, if size permits, as opposed to allocating a separate `mbuf cluster` to hold the same data.

系统还提供默认类型的外部存储缓冲区，称为`mbuf cluster`。可以使用`MCLGET`宏来分配和配置`Mbuf cluster`。 每个`mbuf cluster`的大小都是`MCLBYTES`，其中`MCLBYTES`是机器相关的常量。系统定义了一个建议宏`MINCLSIZE`，这是放入`mbuf cluster`的最小数据量。它等于 `MHLEN`加一。如果大小允许，通常最好将数据存储到`mbuf`的数据区域，而不是分配单独的`mbuf cluster`来保存相同的数据。

### Macros and Functions

There are numerous predefined macros and functions that provide the developer with common utilities.

**`mtod(mbuf, type)`**<br>
Convert an `mbuf` pointer to a `data` pointer. The macro expands to the data pointer cast to the specified type. **Note:** It is advisable to ensure that there is enough contiguous data in `mbuf`. See `m_pullup()` for details.

将`mbuf`指针转换为`data`指针。宏扩展为转换为指定类型的数据指针。注意：建议确保`mbuf`中有足够的连续数据。有关详细信息，请参阅 `m_pullup()`。

**`MGET(mbuf, how, type)`**<br>
Allocate an `mbuf` and initialize it to contain internal data. `mbuf` will point to the allocated `mbuf` on success, or be set to `NULL` on failure. The how argument is to be set to `M_WAITOK` or `M_NOWAIT`. It specifies whether the caller is willing to block if necessary. A number of other functions and macros related to mbufs have the same argument because they may at some point need to allocate new `mbufs`.

分配一个`mbuf`并初始化它以包含内部数据。`mbuf`将指向成功时分配的`mbuf`，或者在失败时设置为`NULL`。如何将参数设置为`M_WAITOK`或`M_NOWAIT`。它指定调用者是否愿意在必要时阻止。与`mbuf` 相关的许多其他函数和宏具有相同的参数，因为它们可能在某些时候需要分配新的 `mbufs`。

**`MGETHDR(mbuf, how, type)`**<br>
Allocate an `mbuf` and initialize it to contain a packet header and internal data.  See `MGET()` for details.

分配一个`mbuf`并初始化它以包含一个包头和内部数据。 有关详细信息，请参阅`MGET()`。

**`MEXTADD(mbuf, buf, size, free, opt_arg1, opt_arg2, flags, type)`**<br>
Associate externally managed data with `mbuf`. Any internal data contained in the `mbuf` will be discarded, and the `M_EXT` flag will be set. The `buf` and `size` arguments are the address and length, respectively, of the data. The `free` argument points to a function which will be called to free the data when the `mbuf` is freed; it is only used if type is `EXT_EXTREF`. The `opt_arg1` and `opt_arg2` arguments will be saved in `ext_arg1` and `ext_arg2` fields of the `struct m_ext` of the `mbuf`. The `flags` argument specifies additional `mbuf` flags; it is not necessary to specify `M_EXT`. Finally, the `type` argument specifies the type of external data, which controls how it will be disposed of when the `mbuf` is freed. In most cases, the correct value is `EXT_EXTREF`.

将外部管理的数据与`mbuf`相关联。`mbuf`中包含的任何内部数据都将被丢弃，并且将设置`M_EXT`标志。`buf`和`size`参数分别是数据的地址和长度。`free`参数指向一个函数，当`mbuf`被释放时，该函数将被调用以释放数据; 它仅在类型为`EXT_EXTREF`时使用。`opt_arg1`和`opt_arg2`参数将保存在`mbuf`的`struct m_ext`的`ext_arg1`和`ext_arg2`字段中。`flags`参数指定了额外的`mbuf`标志; 没有必要指定`M_EXT`。最后，`type` 参数指定外部数据的类型，它控制在释放`mbuf`时如何处理它。在大多数情况下，正确的值是 `EXT_EXTREF`。

**`MCLGET(mbuf, how)`**<br>
Allocate and attach an `mbuf cluster` to `mbuf`. On success, a nonzero value returned; otherwise, 0. Historically, consumers would check for success by testing the `M_EXT` flag on the `mbuf`, but	this is now discouraged to avoid unnecessary awareness of the implementation of external storage in protocol stacks and device drivers.

将 `mbuf cluster` 分配并附加到 `mbuf`。成功时，返回非零值; 从历史上看，消费者会通过测试`mbuf`上的`M_EXT`标志来检查是否成功，但现在不鼓励这样做以避免在协议栈和设备驱动程序中不必要地意识到外部存储的实现。

**`M_ALIGN(mbuf, len)`**<br>
Set the pointer `mbuf-_m_data` to place an object of the size `len` at the end of the internal data area of `mbuf`, `long word` aligned. Applicable only if `mbuf` is newly allocated with `MGET()` or `m_get()`.

**`MH_ALIGN(mbuf, len)`**<br>
Serves the same purpose as `M_ALIGN()` does, but only for mbuf newly allocated with `MGETHDR()` or `m_gethdr()`, or initialized by `m_dup_pkthdr()` or `m_move_pkthdr()`.

**`m_align(mbuf, len)`**<br>
Services the same purpose as `M_ALIGN()` but handles any type of `mbuf`.

**`M_LEADINGSPACE(mbuf)`**<br>
Returns the number of bytes available before the beginning of data in `mbuf`.

**`M_TRAILINGSPACE(mbuf)`**<br>
Returns the number of bytes available after the end of data in `mbuf`.

**`M_PREPEND(mbuf, len,	how)`**<br>
This macro operates on an `mbuf chain`. It is an optimized wrapper for `m_prepend()` that can make use of possible empty space before data (e.g. left after trimming of a link-layer header). The new `mbuf chain` pointer or `NULL` is in `mbuf` after the call.

**`M_MOVE_PKTHDR(to, from)`**<br>
Using this macro is equivalent to calling `m_move_pkthdr(to, from)`.

**`M_WRITABLE(mbuf)`**<br>
This macro will evaluate true if `mbuf` is not marked `M_RDONLY` and if either `mbuf` does not contain external storage or, if it does, then if the reference count of the storage is not greater	than 1.	The `M_RDONLY` flag can be set in `mbuf-_m_flags`. This can be achieved during setup of the external storage, by passing the	`M_RDONLY` bit as a flags argument to the `MEXTADD()` macro, or can be directly set in individual `mbufs`.

**`MCHTYPE(mbuf, type)`**<br>
Change the type of `mbuf` to type. This is a relatively expensive operation and should be avoided.
