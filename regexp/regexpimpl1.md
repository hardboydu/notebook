# Regular Expression Matching Can Be Simple And Fast 
## (but is slow in Java, Perl, PHP, Python, Ruby, ...)

## Introduction

This is a tale of two approaches to regular expression matching. One of them is in widespread use in the standard interpreters for many languages, including Perl. The other is used only in a few places, notably most implementations of awk and grep. The two approaches have wildly different performance characteristics:

这里有两种正则表达式匹配方法，其中一种广泛的应用于多种语言的标准解释器中，包括Perl。另一种仅应用在了很少地方，主要应用在awk和grep中。这两种方法的性能差距非常大：


