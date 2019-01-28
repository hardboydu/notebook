# The Virtual Filesystem

The Virtual Filesystem (sometimes called the Virtual File Switch or more commonly simply the VFS) is the subsystem of the kernel that implements the file and filesystem-related interfaces provided to user-space programs.All filesystems rely on the VFS to enable them not only to coexist, but also to interoperate.This enables programs to use standard Unix system calls to read and write to different filesystems, even on different media, as shown in Figure 13.1.

虚拟文件系统（有时也称作虚拟文件交换，更常见的是简称VSF）作为内核子系统，为用户空间程序提供了文件和文件系统相关的接口。系统中所有文件系统不但依赖VFS共存，而且也依靠VSF系统协同工作。通过虚拟文件系统，程序可以利用标准的unix 系统调用对不同的文件系统，甚至不同介质上的文件系统进行读写操作，如图 13-1 所示：

![13-1](CHAPTER_13/13-1.PNG)

*Figure 13.1 The VFS in action: Using the cp(1) utility to move data from a hard disk mounted as ext3 to a removable disk mounted as ext2. Two different filesystems, two different media, one VFS.*

## Common Filesystem Interface

The VFS is the glue that enables system calls such as open() , read() , and write() to work regardless of the filesystem or underlying physical medium.These days,that might not sound novel—we have long been taking such a feature for granted—but it is a nontrivial feat for such generic system calls to work across many diverse filesystems and varying media. More so, the system calls work between these different filesystems and media — we can use standard system calls to copy or move files from one filesystem to another. In older operating systems, such as DOS, this would never have worked; any access to a nonnative filesystem required special tools. It is only because modern operating systems, such as Linux, abstract access to the filesystems via a virtual interface that such interoperation and generic access is possible.

VFS 是的用户可以直接使用 `open()`、`read()`和 `write()` 这样的系统调用而无需考虑具体文件系统和实际物理介质。现在听起来这并没有什么新奇的（我们早就认为这是理所当然的），但是，使得这些通用的系统调用可以跨越各种文件系统和不同介质执行，绝非微不足道的成绩。更了不起的是，系统调用可以在这些不同的文件系统和介质之间执行 —— 我们可以使用标准的系统调用从一个文件系统拷贝或移动数据到另外一个文件系统。老式的操作系统（比如DOS）是无力完成上述工作的，任何对非本地文件系统的访问都必须依靠特殊的工具才能完成。正是由于现代操作系统引入抽象层，比如Linux，通过虚拟接口访问文件系统，才使得这种协作性和泛型存取成为可能。

New filesystems and new varieties of storage media can find their way into Linux,and programs need not be rewritten or even recompiled. In this chapter, we will discuss the VFS, which provides the abstraction allowing myriad filesystems to behave as one. In the next chapter, we will discuss the block I/O layer, which allows various storage devices — CD to Blu-ray discs to hard drives to  CompactFlash.Together, the VFS and the block I/O layer provide the abstractions, interfaces, and glue that allow user-space programs to issue generic system calls to access files via a uniform naming policy on any filesystem, which itself exists on any storage medium.

新的文件系统和新类型的存储介质都能找到进入 Linux 之路，程序无需重写，甚至无需重新编译。在本章中。我们将讨论 VFS，它把各种不同的文件系统抽象后采用统一的方式进行操作。在第 14 章中，我们将讨论 块I/O 层，它支持各种各样的存储设备 —— 从 CD 到蓝光光盘，从硬件设备再到压缩闪存。VFS 与块 I/O 相结合，提供抽象、接口以及粘合层，使得用户空间的程序调用统一的系统调用访问各种文件，不管文件系统是什么，也不管文件系统位于何种介质，采用命名策略是统一的。

## Filesystem Abstraction Layer

Such a generic interface for any type of filesystem is feasible only because the kernel implements an abstraction layer around its low-level filesystem interface.This abstraction layer enables Linux to support different filesystems, even if they differ in supported features or behavior.This is possible because the VFS provides a common file model that can represent any filesystem’s general feature set and behavior. Of course, it is biased toward Unix-style filesystems. (You see what constitutes a Unix-style filesystem later in this chapter.) Regardless, wildly differing filesystem types are still supportable in Linux, from DOS’s FAT to Windows’s NTFS to many Unix-style and Linux-specific filesystems.

之所以可以使用这种通用接口对所有类型的文件系统进行操作，是因为内核在它的底层文件系统接口上建立了一个抽象层。该抽象层使 Linux 能够支持各种文件系统，即便是它们在功能和行为上存在很大差别。为了支持多文件系统，VFS 提供了一个通用的文件系统模型，该模型囊括了任何文件系统的常用功能集和行为。当然，该模型偏重于 Unix 风格的文件系统（我们将在后边的小节看到 Unix 风格的文件系统构成）。但即使这样， Linux 仍然可以支持很多种差异很大的文件系统，从DOS 系统的 FAT 到Windows 系统的NTFS，再到各种 Unix风格文件系统和 Linux 特有的文件系统。

The abstraction layer works by defining the basic conceptual interfaces and data structures that all filesystems support.The filesystems mold their view of concepts such as “this is how I open files” and “this is what a directory is to me” to match the expectations of the VFS.The actual filesystem code hides the implementation details.To the VFS layer and the rest of the kernel, however, each filesystem looks the same.They all support notions such as files and directories, and they all support operations such as creating and deleting files.

VFS 抽象层之所以能衔接各种各样的文件系统，是因为它定义了所有文件系统都支持的、基本的、概念上的接口和数据结构。同时实际文件系统也将自身的诸如“如何打开文件”，“目录是什么”等概念在形式上与VFS 的定义保持一致。因为实际文件系统的代码在统一的接口和数据结构下隐藏了具体的实现细节，所以在VFS层和内核的其他部分看来，所有文件系统都是相同的。它们都支持像文件和目录这样的概念，同时也支持像创建文件和删除文件这样的操作。

The result is a general abstraction layer that enables the kernel to support many types of filesystems easily and cleanly.The filesystems are programmed to provide the abstracted interfaces and data structures the VFS expects; in turn, the kernel easily works with any filesystem and the exported user-space interface seamlessly works on any filesystem.

内核通过抽象层能够方便，简单地支持各种类型文件系统。实际文件系统通过编程提供 VFS 所期望的抽象接口和数据结构，这样，内核就可以毫不费力地和任何文件系统协同工作，并且这样提供给用户空间的接口，也可以和任何文件系统无缝地连接在一起，完成实际工作。

In fact, nothing in the kernel needs to understand the underlying details of the filesystems, except the filesystems themselves. For example, consider a simple user-space program that does :

其实在内核中，除了文件系统本身外，其他部分并不需要了解文件系统的内部细节。比如一个简单的用户空间程序执行如下的操作：

```c
ret = write (fd, buf, len);
```

This system call writes the len bytes pointed to by buf into the current position in the file represented by the file descriptor fd .This system call is first handled by a generic `sys_write()` system call that determines the actual file writing method for the filesystem on which fd resides.The generic write system call then invokes this method, which is part of the filesystem implementation, to write the data to the media (or whatever this filesystem does on write). Figure 13.2 shows the flow from user-space’s write() call through the data arriving on the physical media. On one side of the system call is the generic VFS interface, providing the frontend to user-space; on the other side of the system call is the filesystem-specific backend, dealing with the implementation details.The rest of this chapter looks at how theVFS achieves this abstraction and provides its interfaces.

该系统调用将 buf 指针指向长度为 len 字节的数据写入文件描述符 fd 对应的文件的当前位置。这个系统调用首先被一个通用系统调用 `sys_write()` 处理，`sys_write()` 函数要找到 fd 所在的文件系统实际给出的是哪个写操作，然后再执行该操作。实际文件系统的写方法是文件系统实现的一部分，数据最终通过该操作写入介质（或执行这个文件系统想要完成的写动作）。图 13-2 描述了从用户空间的 `write()` 调用到数据被写入磁盘介质的整个流程。一方面，系统调用时通用 VFS接口，提供给用户空间的前端；另一方面，系统调用时具体文件系统的后端，处理时限细节。接下来的小节中我们会具体看到 VFS 抽象模型以及它提供的接口。

![13-2](CHAPTER_13/13-2.PNG)

## Unix Filesystems

Historically, Unix has provided four basic filesystem-related abstractions: files, directory entries, inodes, and mount points.

Unix 使用了四种和文件系统相关的抽象概念：文件、目录项、索引节点和挂载点。

A filesystem is a hierarchical storage of data adhering to a specific structure. Filesystems contain files, directories, and associated control information. Typical operations performed on filesystems are creation, deletion, and mounting. In Unix, filesystems are mounted at a specific mount point in a global hierarchy known as a namespace. 1 This enables all mounted filesystems to appear as entries in a single tree. Contrast this single, unified tree with the behavior of DOS and Windows, which break the file namespace up into drive letters, such as C: .This breaks the namespace up among device and partition boundaries, “leaking” hardware details into the filesystem abstraction.As this delineation may be arbitrary and even confusing to the user, it is inferior to Linux’s unified namespace.

从本质上讲文件系统是个数的数据分层存储结构，它包含文件、目录和相关的控制信息。文件系统的通用操作包括创建、删除和安装等。在 Unix 中，文件系统被安装在一个特定的挂载点上，该挂载点在全局层次结构中被称为命名空间，所有的已安装文件系统都作为根文件系统树的枝叶出现在系统中。与这种单一、统一的树形成鲜明对比的就是 DOS 和 Windows 的表现，它们将文件的命名空间分类为驱动字母，例如 `C:`，这种将命名空间划分为设备和分区的做法，相当于把硬件细节“泄露”给文件系统抽象层。对用户而言，如此的描述有点随意，甚至产生混淆，这是 Linux 统一命名空间所不屑一顾的。

> 1 Recently, Linux has made this hierarchy per-process, to give a unique namespace to each process. Because each process inherits its parent’s namespace (unless you specify otherwise), there is seemingly one global namespace.
>
> 近来，Linux已经将这种层次化概念引入和单个进程中，每个进程都指定一个唯一的命名空间。因为每个进程都会集成父进程的命名空间（除非是特别声明的情况），所以所有进程往往都只有一个全局命名空间。

A file is an ordered string of bytes.The first byte marks the beginning of the file, and the last byte marks the end of the file. Each file is assigned a human-readable name for identification by both the system and the user.Typical file operations are read, write, create, and delete.The Unix concept of the file is in stark contrast to record-oriented filesystems, such as OpenVMS’s Files-11. Record-oriented filesystems provide a richer, more structured representation of files than Unix’s simple byte-stream abstraction, at the cost of simplicity and flexibility.

一个文件其实可以看做是一个有序字节串，字节串的第一个字节是文件的头，最后一个字节是文件的尾。每一个文件为了便于系统和用户识别，都被分配了一个便于理解的名字。典型的文件操作有读、写、创建、和删除等。Unix 文件的概念与面向记录的文件系统（如 OpenVMS 的 File-11）形成了鲜明的对照。面型记录的文件系统提供更丰富、更结构化的表示，而简单的面向字节流抽象的文件则以简单性和相当的灵活性为代价。

Files are organized in directories.A directory is analogous to a folder and usually contains related files. Directories can also contain other directories, called subdirectories. In this fashion, directories may be nested to form paths. Each component of a path is called a directory entry. A path example is `/home/wolfman/butter` — the root directory `/` , the directories `home` and `wolfman` , and the file `butter` are all directory entries, called dentries.In Unix, directories are actually normal files that simply list the files contained therein. Because a directory is a file to the VFS, the same operations performed on files can be performed on directories.

文件通过目录组织起来。文件目录好比一个文件夹，用来容纳相关的文件。因为目录也可以包含其他目录，即子目录，所以目录可以层层嵌套，形成文件路径。路径中每一部分都被称为目录条目。`/home/wolfman/butter` 是文件路径的一个例子 —— 根目录 `/` ，目录 `home`，`wolfman` 和文件 `buffer` 都是目录条目，它们统称为目录项。在 Unix 中，目录属于普通文件，它列出包含在其中的所有文件。由于VFS 把目录当做文件对待，所以可以对目录执行和文件相同的操作。

Unix systems separate the concept of a file from any associated information about it, such as access permissions, size, owner, creation time, and so on. This information is sometimes called file metadata (that is, data about the file’s data) and is stored in a separate data structure from the file, called the inode. This name is short for index node, although these days the term inode is much more ubiquitous.

Unix 系统将文件的相关信息和文件本身这两个概念加以区分，例如访问控制权限、大小、拥有者、创建时间等信息。文件相关信息，有时被称作文件的元数据（也就是文件的相关数据），被存储在一个单独的数据结构中，该结构被称为索引节点（inode），它其实是  index node 的缩写，不过近来术语 “inode” 使用的更普遍一些。

All this information is tied together with the filesystem’s own control information, which is stored in the superblock.The superblock is a data structure containing information about the filesystem as a whole. Sometimes the collective data is referred to as filesystem metadata. Filesystem metadata includes information about both the individual files and the filesystem as a whole.

所有这些信息都和文件系统的控制信息密切相关，文件系统的控制信息存储在超级块中，超级块是一种包含文件系统信息的数据结构。有时，把这些收集起来的信息称为文件系统数据元，它集单独文件信息和文件系统的信息于一身。

Traditionally, Unix filesystems implement these notions as part of their physical on-disk layout. For example, file information is stored as an inode in a separate block on the disk; directories are files; control information is stored centrally in a superblock, and so on.The Unix file concepts are physically mapped on to the storage medium.The Linux VFS is designed to work with filesystems that understand and implement such concepts. Non-Unix filesystems, such as FAT or NTFS, still work in Linux, but their filesystem code must provide the appearance of these concepts. For example, even if a filesystem does not support distinct inodes, it must assemble the inode data structure in memory as if it did. Or if a filesystem treats directories as a special object, to the VFS they must represent directories as mere files. Often, this involves some special processing done on-the-fly by the non-Unix filesystems to cope with the Unix paradigm and the requirements of the VFS. Such filesystems still work, however, and the overhead is not unreasonable.

一直以来，Unix 文件系统在它们物理磁盘布局中也是按照上述概念实现的。比如说在磁盘上，文件（目录也属于文件）信息按照索引节点形式存储在单独的块中：控制信息被击中存储在磁盘的超级块中，等等。Unix 中文件的概念从物理上被映射到存储介质中。 Linux 的 VFS 的设计目标就是要保证能与支持和实现了这些概念的文件系统协同工作。像如 FAT 或 NTFS 这样的非 Unix 风格的文件系统，虽然也可以在 Linux 上工作，但是它们必须经过封装，提供一个符合这些概念的界面。比如，即使一个文件系统不再支持索引节点，它也必须在内存中装配索引节点结构体，就像它本身包含索引节点一样。在比如，如果一个文件系统将目录看做一种特殊对象，那么要想使用 VFS ，就必须将目录重新表示为文件形式。通常这种转换需要在使用现场（on the fly）引入一些特殊处理，使得非 Unix 文件系统能够兼容 Unix 文件系统的使用规则并满足 VFS 的需求。这种文件系统当然仍能工作，但是其带来的开销则不可思议（开销太大了）。

## VFS Objects and Their Data Structures

The VFS is object-oriented. 2 A family of data structures represents the common file model.These data structures are akin to objects. Because the kernel is programmed strictly in C, without the benefit of a language directly supporting object-oriented paradigms, the data structures are represented as C structures.The structures contain both data and pointers to filesystem-implemented functions that operate on the data.

VFS 其实采用的是面向对象 2 的设计思路，使用一组数据结构来代表通用文件对象。这些数据结构类似于对象。因为内核纯粹使用 C 代码实现，没有直接利用面相对象的语言，所以内核中的数据结构都使用 C 语言的结构体实现，而这些结构体包含数据的同时也包含操作这些数据的函数指针，其中的操作函数由具体文件系统实现。

> 2 People often miss this, or even deny it, but there are many examples of object-oriented programming in the kernel. Although the kernel developers may shun C++ and other explicitly object-oriented languages, thinking in terms of objects is often useful. The VFS is a good example of how to do clean and efficient OOP in C, which is a language that lacks any OOP constructs.
>
> 人们时常忽略，甚至会否认，但是在内核中确实存在很多利用面向对象思想编程的例子。虽然内核开发者可能有意避免 C++ 和其他的面相对象语言，但是面相对象的思想仍然经常被借鉴 —— 虽然 C 语言缺乏面相对象的机制。VFS 就是一个利用 C 代码来有效和简洁地实现 OOP 的例子。

The four primary object types of the VFS are

VFS 中有四个主要的对象类型，它们分别是：

* The superblock object, which represents a specific mounted filesystem. <br> 超级块对象，它代表一个具体的已安装文件系统。
* The inode object, which represents a specific file. <br> 索引节点对象，它代表一个具体文件
* The dentry object, which represents a directory entry, which is a single component of a path. <br> 目录项对象，它代表一个目录项，是路径的一个组成部分。
* The file object, which represents an open file as associated with a process. <br> 文件对象，它代表由进程打开的文件。

Note that because the VFS treats directories as normal files, there is not a specific directory object. Recall from earlier in this chapter that a dentry represents a component in a path, which might include a regular file. In other words, a dentry is not the same as a directory, but a directory is just another kind of file. Got it?

注意，因为 VFS 将目录作为一个文件来处理，所以不存在目录对象。回忆本章前面提到的目录项代表的是路径中的一个组成部分，它可能包括一个普通文件。换句话说，目录项不同于目录，但目录确实另一种形式的文件，明白了吗？

An operations object is contained within each of these primary objects.These objects describe the methods that the kernel invokes against the primary objects:

每个主要对象中都包含一个操作对象，这些操作对象描述了内核针对主要对象可以使用的方法：

* The `super_operations` object, which contains the methods that the kernel can invoke on a specific filesystem, such as `write_inode()` and `sync_fs()` <br> `super_operations` 对象，其中包括内核针对特定文件系统所能调用的方法，比如 `write_inode()` 和 `sync_fs()` 等方法。
* The `inode_operations` object, which contains the methods that the kernel can invoke on a specific file, such as `create()` and `link()` <br> `inode_operations` 对象，其中包括内核针对特定文件所能调用的方法，比如 `create()` 和 `link()` 等方法。
* The `dentry_operations` object, which contains the methods that the kernel can invoke on a specific directory entry, such as `d_compare()` and `d_delete()` <br> `dentry_operations` 对象，其中包括内核针对特定目录所能调用的方法，比如 `d_compare()` 和 `d_delete()` 等方法。
* The `file_operations` object, which contains the methods that a process can invoke on an open file, such as `read()` and `write()` <br> `file_operations` 对象，其中包括进程针对已打开文件所能调用的方法，比如 `read()` 和 `write()` 等方法。

The operations objects are implemented as a structure of pointers to functions that operate on the parent object. For many methods, the objects can inherit a generic function if basic functionality is sufficient. Otherwise, the specific instance of the particular filesystem fills in the pointers with its own filesystem-specific methods.

操作对象作为一个结构体指针来实现，此结构体中包含指向操作其父对象的函数指针。对于其中许多方法来说，可以继承使用 VFS 提供的通用函数，如果通用函数提供的基本功能无法满足需要，那么就必须使用实际文件系统的独有方法来填充这些函数指针，使其指向文件系统实例。

Again, note that objects refer to structures—not explicit class types, such as those in C++ or Java.These structures, however, represent specific instances of an object, their associated data, and methods to operate on themselves.They are very much objects.

再次提醒，我们这里所说的对象就是指结构体，而不是像 C++ 或者 JAVA 那样的真正面向对象数据类类型。但是这些结构体的确代表的是一个对象，它含有相关的数据和对这些数据的操作，所以可以说它们就是对象。

The VFS loves structures, and it is comprised of a couple more than the primary objects previously discussed. Each registered filesystem is represented by a `file_system_type` structure. This object describes the filesystem and its capabilities. Furthermore, each mount point is represented by the `vfsmount` structure.This structure contains information about the mount point, such as its location and mount flags.

VFS 使用了大量结构体对象，它所包括的对象远远多于上面提到的这几种主要对象，比如，每个注册的文件系统都由 `file_system_type` 结构体来表示，它描述了文件系统及其性能：另外，每一个挂接点也都用 `vfsmount` 结构体表示，它包含的是挂接点的相关信息，如位置和挂接标志。

Finally, two per-process structures describe the filesystem and files associated with a process.They are, respectively, the `fs_struct` structure and the `file` structure.

在本章的最后还要介绍两个与进程相关的结构体，它们描述了文件系统以及和进程相关的文件，分别是 `fs_struct` 结构体和 `file` 结构体

The rest of this chapter discusses these objects and the role they play in implementing the VFS layer.

13.5 节将讨论这些对象以及它们在 VFS 层的实现中扮演的角色。