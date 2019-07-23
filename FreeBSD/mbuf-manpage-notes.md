
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

* **`mtod(mbuf, type)`**<br>
Convert an `mbuf` pointer to a `data` pointer. The macro expands to the data pointer cast to the specified type. **Note:** It is advisable to ensure that there is enough contiguous data in `mbuf`. See `m_pullup()` for details. <br><br>
将`mbuf`指针转换为`data`指针。宏扩展为转换为指定类型的数据指针。注意：建议确保`mbuf`中有足够的连续数据。有关详细信息，请参阅 `m_pullup()`。

* **`MGET(mbuf, how, type)`**<br>
Allocate an `mbuf` and initialize it to contain internal data. `mbuf` will point to the allocated `mbuf` on success, or be set to `NULL` on failure. The how argument is to be set to `M_WAITOK` or `M_NOWAIT`. It specifies whether the caller is willing to block if necessary. A number of other functions and macros related to mbufs have the same argument because they may at some point need to allocate new `mbufs`. <br><br>
分配一个`mbuf`并初始化它以包含内部数据。`mbuf`将指向成功时分配的`mbuf`，或者在失败时设置为`NULL`。如何将参数设置为`M_WAITOK`或`M_NOWAIT`。它指定调用者是否愿意在必要时阻止。与`mbuf` 相关的许多其他函数和宏具有相同的参数，因为它们可能在某些时候需要分配新的 `mbufs`。

* **`MGETHDR(mbuf, how, type)`**<br>
Allocate an `mbuf` and initialize it to contain a packet header and internal data.  See `MGET()` for details.<br><br>
分配一个`mbuf`并初始化它以包含一个包头和内部数据。 有关详细信息，请参阅`MGET()`。

* **`MEXTADD(mbuf, buf, size, free, opt_arg1, opt_arg2, flags, type)`**<br>
Associate externally managed data with `mbuf`. Any internal data contained in the `mbuf` will be discarded, and the `M_EXT` flag will be set. The `buf` and `size` arguments are the address and length, respectively, of the data. The `free` argument points to a function which will be called to free the data when the `mbuf` is freed; it is only used if type is `EXT_EXTREF`. The `opt_arg1` and `opt_arg2` arguments will be saved in `ext_arg1` and `ext_arg2` fields of the `struct m_ext` of the `mbuf`. The `flags` argument specifies additional `mbuf` flags; it is not necessary to specify `M_EXT`. Finally, the `type` argument specifies the type of external data, which controls how it will be disposed of when the `mbuf` is freed. In most cases, the correct value is `EXT_EXTREF`.<br><br>
将外部管理的数据与`mbuf`相关联。`mbuf`中包含的任何内部数据都将被丢弃，并且将设置`M_EXT`标志。`buf`和`size`参数分别是数据的地址和长度。`free`参数指向一个函数，当`mbuf`被释放时，该函数将被调用以释放数据; 它仅在类型为`EXT_EXTREF`时使用。`opt_arg1`和`opt_arg2`参数将保存在`mbuf`的`struct m_ext`的`ext_arg1`和`ext_arg2`字段中。`flags`参数指定了额外的`mbuf`标志; 没有必要指定`M_EXT`。最后，`type` 参数指定外部数据的类型，它控制在释放`mbuf`时如何处理它。在大多数情况下，正确的值是 `EXT_EXTREF`。

* **`MCLGET(mbuf, how)`**<br>
Allocate and attach an `mbuf cluster` to `mbuf`. On success, a nonzero value returned; otherwise, 0. Historically, consumers would check for success by testing the `M_EXT` flag on the `mbuf`, but	this is now discouraged to avoid unnecessary awareness of the implementation of external storage in protocol stacks and device drivers.<br><br>
将 `mbuf cluster` 分配并附加到 `mbuf`。成功时，返回非零值; 从历史上看，消费者会通过测试`mbuf`上的`M_EXT`标志来检查是否成功，但现在不鼓励这样做以避免在协议栈和设备驱动程序中不必要地意识到外部存储的实现。

* **`M_ALIGN(mbuf, len)`**<br>
Set the pointer `mbuf-_m_data` to place an object of the size `len` at the end of the internal data area of `mbuf`, `long word` aligned. Applicable only if `mbuf` is newly allocated with `MGET()` or `m_get()`. <br><br>
设置指针`mbuf-_m_data`，在`mbuf`，`long word`对齐的内部数据区末尾放置一个大小为`len`的对象。 仅当`mbuf`新分配了`MGET（）`或`m_get（）`时才适用。

* **`MH_ALIGN(mbuf, len)`**<br>
Serves the same purpose as `M_ALIGN()` does, but only for mbuf newly allocated with `MGETHDR()` or `m_gethdr()`, or initialized by `m_dup_pkthdr()` or `m_move_pkthdr()`.<br><br>
具有与`M_ALIGN()`相同的目的，但仅适用于使用`MGETHDR()`或`m_gethdr()`重新分配的mbuf，或者由`m_dup_pkthdr()`或`m_move_pkthdr()`初始化的mbuf。

* **`m_align(mbuf, len)`**<br>
Services the same purpose as `M_ALIGN()` but handles any type of `mbuf`.<br><br>
提供与`M_ALIGN()`相同的目的，但处理任何类型的`mbuf`。

* **`M_LEADINGSPACE(mbuf)`**<br>
Returns the number of bytes available before the beginning of data in `mbuf`.<br><br>
返回`mbuf`中数据开头之前可用的字节数。

* **`M_TRAILINGSPACE(mbuf)`**<br>
Returns the number of bytes available after the end of data in `mbuf`.<br><br>
返回`mbuf`中数据结尾后可用的字节数。

* **`M_PREPEND(mbuf, len, how)`**<br>
This macro operates on an `mbuf chain`. It is an optimized wrapper for `m_prepend()` that can make use of possible empty space before data (e.g. left after trimming of a link-layer header). The new `mbuf chain` pointer or `NULL` is in `mbuf` after the call.<br><br>
这个宏在`mbuf chain` 上运行。 它是 `m_prepend()` 的优化包装器，它可以在数据之前利用可能的空白空间（例如，在修剪链路层头之后留下）。调用后，新的 `mbuf chain` 指针或`NULL`在`mbuf`中。

* **`M_MOVE_PKTHDR(to, from)`**<br>
Using this macro is equivalent to calling `m_move_pkthdr(to, from)`.<br><br>
使用这个宏相当于调用`m_move_pkthdr(to，from)`。

* **`M_WRITABLE(mbuf)`**<br>
This macro will evaluate true if `mbuf` is not marked `M_RDONLY` and if either `mbuf` does not contain external storage or, if it does, then if the reference count of the storage is not greater	than 1.	The `M_RDONLY` flag can be set in `mbuf-_m_flags`. This can be achieved during setup of the external storage, by passing the	`M_RDONLY` bit as a flags argument to the `MEXTADD()` macro, or can be directly set in individual `mbufs`.<br><br>
如果`mbuf`没有标记为 `M_RDONLY` 并且 `mbuf` 不包含外部存储，或者如果它不包含外部存储，那么这个宏将评估为真，如果存储的引用计数不大于 1. `M_RDONLY` 标志可以在`mbuf-_m_flags`中设置。这可以在外部存储设置期间通过将 `M_RDONLY` 位作为 `flags` 参数传递给`MEXTADD()`宏来实现，或者可以直接设置在单独的 `mbufs` 中。

* **`MCHTYPE(mbuf, type)`**<br>
Change the type of `mbuf` to `type`. This is a relatively expensive operation and should be avoided.<br><br>
将 `mbuf` 的类型更改为 `type`。 这是一项相对昂贵的操作，应该避免。

The functions are:

* **`m_get(how, type)`**<br>
A function version of `MGET()` for non-critical paths.<br><br>
用于非关键路径的函数版本`MGET()`。

* **`m_get2(size, how, type, flags)`**<br>
Allocate an `mbuf` with enough space to hold specified amount of data.<br><br>
分配一个具有足够空间的`mbuf`来保存指定数量的数据。

* **`m_getm(orig, len, how, type)`**<br>
Allocate `len` bytes worth of `mbufs` and `mbuf clusters` if necessary and append the resulting allocated `mbuf chain` to the `mbuf chain orig`, if it is `non-NULL`. If the allocation fails at any point, free whatever was allocated and return `NULL`. If orig is `non-NULL`, it will not be freed. It is possible to use `m_getm()` to either append len bytes to an existing `mbuf` or `mbuf chain` (for example, one which may be sitting in a pre-allocated ring) or to simply perform an all-or-nothing `mbuf` and `mbuf cluster` allocation.<br><br>
如果需要，分配值为`mbufs`和`mbuf clusters`的`len`字节，并将结果分配的`mbuf chain`附加到`mbuf chain orig`，如果它是`non-NULL`。 如果分配在任何时候失败，则释放分配的任何内容并返回`NULL`。 如果`orig`为`non-NULL`，则不会释放它。 可以使用`m_getm()`将`len`字节附加到现有的`mbuf`或`mbuf chain`（例如，可能位于预先分配的环中）或简单地执行全部或  - 没有`mbuf`和`mbuf cluster`分配。

* **`m_gethdr(how, type)`**<br>
A function version of `MGETHDR()` for non-critical paths.<br><br>
用于非关键路径的函数版本`MGETHDR()`。

* **`m_getcl(how, type, flags)`**<br>
Fetch an `mbuf` with a `mbuf cluster` attached to it. If one of the allocations fails, the entire allocation fails. This routine is the preferred way of fetching both the `mbuf` and `mbuf cluster` together, as it avoids having to `unlock/relock` between allocations. Returns `NULL` on failure.<br><br>
获取一个附有`mbuf cluster`的`mbuf`。 如果其中一个分配失败，则整个分配失败。 这个例程是同时获取`mbuf`和`mbuf cluster`的首选方法，因为它避免了在分配之间 `解锁/重新锁定`。 失败时返回`NULL`。

* **`m_getjcl(how, type, flags, size)`**<br>
This is like `m_getcl()` but it the size of the `cluster` allocated will be large enough for size bytes.<br><br>
这就像`m_getcl()`但是它分配的`cluster`的大小对于大小字节来说足够大。

* **`m_free(mbuf)`**<br>
Frees `mbuf`. Returns `m_next` of the freed `mbuf`.<br><br>
释放`mbuf`。 返回释放的`mbuf`的`m_next`。

The functions below operate on `mbuf chains`.

* **`m_freem(mbuf)`**<br>
Free an entire `mbuf chain`, including any external storage.<br><br>
释放整个`mbuf chain`，包括任何外部存储。

* **`m_adj(mbuf, len)`**<br>
Trim len bytes from the head of an `mbuf chain` if `len` is positive, from the tail otherwise.<br><br>
如果`len`大于等于零，则从`mbuf chain`的头部修剪`len`个字节，否则从尾部修剪。

* **`m_append(mbuf, len, cp)`**<br>
Append `len` bytes of data `cp` to the `mbuf chain`. Extend the `mbuf chain` if the `new data` does not fit in existing space. <br><br>
将`len`个字节的数据`cp`附加到`mbuf chain`。 如果新数据不适合现有空间，则扩展`mbuf chain`。

* **`m_prepend(mbuf, len, how)`**<br>
Allocate a new `mbuf` and prepend it to the `mbuf chain`, handle `M_PKTHDR` properly. **Note**: It does not allocate any `mbuf clusters`, so len must be less than `MLEN` or `MHLEN`, depending on the `M_PKTHDR` flag setting.<br><br>
分配一个新的`mbuf`并将其添加到`mbuf chain`，正确处理`M_PKTHDR`。 **注意**：它不分配任何`mbuf clusters`，因此`len`必须小于`MLEN`或`MHLEN`，具体取决于`M_PKTHDR`标志设置。

* **`m_copyup(mbuf, len, dstoff)`**<br>
Similar to `m_pullup()` but copies `len` bytes of data into a new `mbuf` at `dstoff` bytes into the `mbuf`. The `dstoff` argument aligns the data and leaves room for a link layer header. Returns the new `mbuf chain` on success, and frees the `mbuf chain` and returns `NULL` on failure. **Note**: The function does not allocate `mbuf clusters`, so `len + dstoff` must be less than `MHLEN`.<br><br>

* **`m_pullup(mbuf, len)`**<br>
Arrange that the first `len` bytes of an `mbuf chain` are contiguous and lay in the data area of `mbuf`, so they are accessible with `mtod(mbuf, type)`. It is important to remember that this may involve reallocating some `mbufs` and moving data so all pointers referencing `data` within the old `mbuf chain` must be recalculated or made invalid. Return the new `mbuf chain` on success, `NULL` on failure (the `mbuf chain` is freed in this case). **Note**: It does not allocate any `mbuf clusters`, so `len` must be less than or equal to `MHLEN`.

* **`m_pulldown(mbuf, offset, len, offsetp)`**<br>
Arrange that `len` bytes between `offset` and `offset + len` in the `mbuf chain` are contiguous and lay in the data area of `mbuf`, so they are accessible with `mtod(mbuf, type)`. `len` must be smaller than, or equal to, the size of an `mbuf cluster`. Return a pointer to an intermediate `mbuf` in the chain containing the requested region; the `offset` in the data region of the `mbuf chain` to the data contained in the returned `mbuf` is stored in `*offsetp`. If `offsetp` is `NULL`, the region may be accessed using `mtod(mbuf, type)`. If `offsetp` is `non-NULL`, the region may be accessed using `mtod(mbuf, uint8_t) + *offsetp`. The region of the `mbuf chain` between its beginning and `offset` is not modified, therefore it is safe to hold pointers to data within this region before calling `m_pulldown()`.

* **`m_copym(mbuf, offset, len, how)`**<br>
Make a copy of an `mbuf chain` starting `offset` bytes from the beginning, continuing for `len` bytes. If `len` is `M_COPYALL`, copy to the end of the `mbuf chain`. **Note**: The copy is read-only, because the `mbuf clusters` are not copied, only their reference counts are incremented.

* **`m_copypacket(mbuf, how)`**<br>
Copy an entire packet including header, which must be present. This is an optimized version of the common case `m_copym(mbuf, 0, M_COPYALL, how)`. **Note**: the copy is read-only, because the `mbuf clusters` are not copied, only their reference counts are incremented.

* **`m_dup(mbuf, how)`**<br>
Copy a packet header `mbuf chain` into a completely new `mbuf chain`, including copying any `mbuf clusters`. Use this instead of `m_copypacket()` when you need a writable copy of an `mbuf chain`.

* **`m_copydata(mbuf, offset, len, buf)`**<br>
Copy data from an `mbuf chain` starting `off` bytes from the beginning, continuing for `len` bytes, into the indicated buffer `buf`.

* **`m_copyback(mbuf, offset, len, buf)`**<br>
Copy `len` bytes from the buffer `buf` back into the indicated `mbuf chain`, starting at `offset` bytes from the beginning of the `mbuf chain`, extending the `mbuf chain` if necessary. **Note**: It does not allocate any `mbuf clusters`, just adds `mbufs` to the `mbuf chain`. It is safe to set `offset` beyond the current `mbuf chain` end: zeroed `mbufs` will be allocated to fill the space.

* **`m_length(mbuf, last)`**<br>
Return the length of the `mbuf chain`, and optionally a pointer to the last `mbuf`.

* **`m_dup_pkthdr(to, from, how)`**<br>
Upon the function’s completion, the `mbuf` to will contain an identical copy of `from->m_pkthdr` and the per-packet attributes found in the `mbuf chain from`. The `mbuf from` must have the flag `M_PKTHDR` initially set, and to must be empty on entry.

* **`m_move_pkthdr(to, from)`**<br>
Move `m_pkthdr` and the per-packet attributes from the `mbuf chain` from to the `mbuf` to. The `mbuf` from must have the flag `M_PKTHDR` initially set, and to must be empty on entry. Upon the function’s completion, from will have the flag `M_PKTHDR` and the per-packet attributes cleared.

* **`m_fixhdr(mbuf)`**<br>
Set the packet-header length to the length of the `mbuf chain`.

* **`m_devget(buf, len, offset, ifp, copy)`**<br>
Copy data from a device local memory pointed to by `buf` to an `mbuf chain`. The copy is done using a specified copy routine copy, or `bcopy()` if copy is `NULL`.

* **`m_cat(m, n)`**<br>
Concatenate `n` to `m`. Both `mbuf chains` must be of the same type. `n` is not guaranteed to be valid after `m_cat()` returns. `m_cat()` does not update any packet header fields or free `mbuf` tags.

* **`m_catpkt(m, n)`**<br>
A variant of `m_cat()` that operates on packets. Both `m` and `n` must contain packet headers. `n` is not guaranteed to be valid after `m_catpkt()` returns.

* **`m_split(mbuf, len, how)`**<br>
Partition an `mbuf chain` in two pieces, returning the tail: all but the first `len` bytes. In case of failure, it returns `NULL` and attempts to restore the `mbuf chain` to its original state.

* **`m_apply(mbuf, off, len, f, arg)`**<br>
Apply a function to an `mbuf chain`, at offset `off`, for length `len` bytes. Typically used to avoid calls to `m_pullup()` which would otherwise be unnecessary or undesirable. `arg` is a convenience argument which is passed to the callback function `f`.<br><br>
Each time `f()` is called, it will be passed `arg`, a pointer to the data in the current `mbuf`, and the length `len` of the data in this `mbuf` to which the function should be applied.<br><br>
The function should return zero to indicate success; otherwise, if an error is indicated, then `m_apply()` will return the error and stop iterating through the `mbuf chain`.

* **`m_getptr(mbuf, loc, off)`**<br>
Return a pointer to the `mbuf` containing the data located at `loc` bytes from the beginning of the `mbuf chain`. The corresponding offset into the `mbuf` will be stored in `*off`.

* **`m_defrag(m0, how)`**<br>
Defragment an `mbuf chain`, returning the shortest possible chain of `mbufs` and `clusters`. If allocation fails and this can not be completed, `NULL` will be returned and the original chain will be unchanged. Upon success, the original chain will be freed and the new chain will be returned. how should be either `M_WAITOK` or `M_NOWAIT`, depending on the caller’s preference.<br><br>
This function is especially useful in network drivers, where certain long `mbuf chains` must be shortened before being added to `TX descriptor` lists.

* **`m_collapse(m0, how, maxfrags)`**<br>
Defragment an `mbuf chain`, returning a chain of at most maxfrags `mbufs` and `clusters`. If allocation fails or the chain cannot be collapsed as requested, `NULL` will be returned, with the original chain possibly modified. As with `m_defrag()`, how should be one of `M_WAITOK` or `M_NOWAIT`.

* **`m_unshare(m0, how)`**<br>
Create a version of the specified `mbuf chain` whose contents can be safely modified without affecting other users. If allocation fails and this operation can not be completed, `NULL` will be returned. The original `mbuf chain` is always reclaimed and the reference count of any shared `mbuf clusters` is decremented. how should be either `M_WAITOK` or `M_NOWAIT`, depending on the caller’s preference. As a side-effect of this process the returned `mbuf chain` may be compacted. <br><br>
This function is especially useful in the transmit path of network code, when data must be encrypted or otherwise altered prior to transmission.

## HARDWARE-ASSISTED CHECKSUM CALCULATION

This section currently applies to TCP/IP only. In order to save the host CPU resources, computing checksums is offloaded to the network interface hardware if possible. The m_pkthdr member of the leading mbuf of a packet contains two fields used for that purpose, int csum_flags and int csum_data. The meaning of those fields depends on the direction a packet flows in, and on whether the packet is fragmented. Henceforth, csum_flags or csum_data of a packet will denote the corresponding field of the m_pkthdr member of the leading mbuf in the mbuf chain containing the packet.

On output, checksum offloading is attempted after the outgoing interface has been determined for a packet. The interface-specific field ifnet.if_data.ifi_hwassist (see ifnet(9)) is consulted for the capabilities of the interface to assist in computing checksums. The csum_flags field of the packet header is set to indicate which actions the interface is supposed to perform on it. The actions unsupported by the network interface are done in the software prior to passing the packet down to the interface driver; such actions will never be requested through csum_flags.

The flags demanding a particular action from an interface are as follows:

|||
|-|-
|`CSUM_IP`  |The IP header checksum is to be computed and stored in the corresponding field of the packet. The hardware is expected to know the format of an IP header to determine the offset of the IP checksum field.
|`CSUM_TCP` |The TCP checksum is to be computed. (See below.)
|`CSUM_UDP` |The UDP checksum is to be computed. (See below.)

Should a TCP or UDP checksum be offloaded to the hardware, the field csum_data will contain the byte offset of the checksum field relative to the end of the IP header. In this case, the checksum field will be initially set by the TCP/IP module to the checksum of the pseudo header defined by the TCP and UDP specifications.

On input, an interface indicates the actions it has performed on a packet by setting one or more of the following flags in csum_flags associated with the packet:

|||
|-|-|
|`CSUM_IP_CHECKED`| The IP header checksum has been computed.
|`CSUM_IP_VALID`| The IP header has a valid checksum. This flag can appear only in combination with CSUM_IP_CHECKED.
|`CSUM_DATA_VALID` |The checksum of the data portion of the IP packet has been computed and stored in the field csum_data in network byte order.
|`CSUM_PSEUDO_HDR` |Can be set only along with CSUM_DATA_VALID to indicate that the IP data checksum found in csum_data allows for the pseudo header defined by the TCP and UDP specifications. Otherwise the checksum of the pseudo header must be calculated by the host CPU and added to csum_data to obtain the final checksum to be used for TCP or UDP validation purposes.

If a particular network interface just indicates success or failure of TCP or UDP checksum validation without returning the exact value of the checksum to the host CPU, its driver can mark CSUM_DATA_VALID and CSUM_PSEUDO_HDR in csum_flags, and set csum_data to 0xFFFF hexadecimal to indicate a valid checksum. It is a peculiarity of the algorithm used that the Internet checksum calculated over any valid packet will be 0xFFFF as long as the original checksum field is included.

## STRESS TESTING

When running a kernel compiled with the option MBUF_STRESS_TEST, the following sysctl(8)-controlled options may be used to create various failure/extreme cases for testing of network drivers and other parts of the kernel that rely on mbufs.

* `net.inet.ip.mbuf_frag_size` <br>
Causes ip_output() to fragment outgoing mbuf chains into fragments of the specified size. Setting this variable to 1 is an excellent way to test the long mbuf chain handling ability of network drivers.

* `kern.ipc.m_defragrandomfailures` <br>
Causes the function m_defrag() to randomly fail, returning NULL. Any piece of code which uses m_defrag() should be tested with this feature.

## RETURN VALUES

See above.

## SEE ALSO

`ifnet(9)`, `mbuf_tags(9)`

## HISTORY

Mbufs appeared in an early version of BSD. Besides being used for network packets, they were used to store various dynamic structures, such as routing table entries, interface addresses, protocol control blocks, etc. In more recent FreeBSD use of mbufs is almost entirely limited to packet storage, with uma(9) zones being used directly to store other network-related memory.

Historically, the mbuf allocator has been a special-purpose memory allocator able to run in interrupt contexts and allocating from a special kernel address space map. As of FreeBSD 5.3, the mbuf allocator is a wrapper around uma(9), allowing caching of mbufs, clusters, and mbuf + cluster pairs in per-CPU caches, as well as bringing other benefits of slab allocation.

## AUTHORS

The original mbuf manual page was written by Yar Tikhiy. The uma(9) mbuf allocator was written by 
Bosko Milekic.

BSD September 27, 2017 BSD
