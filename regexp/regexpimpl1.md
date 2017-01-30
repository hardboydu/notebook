# Regular Expression Matching Can Be Simple And Fast 
## (but is slow in Java, Perl, PHP, Python, Ruby, ...)

## Introduction

This is a tale of two approaches to regular expression matching. One of them is in widespread use in the standard interpreters for many languages, including Perl. The other is used only in a few places, notably most implementations of awk and grep. The two approaches have wildly different performance characteristics:

这里有两种正则表达式匹配方法，其中一种广泛的应用于多种语言的标准解释器中，包括Perl。另一种仅应用在了很少地方，主要应用在awk和grep中。这两种方法的性能差距非常大：

![](https://raw.githubusercontent.com/hardboydu/notebook/master/regexp/regexpimpl/grep3p.png) ![](https://raw.githubusercontent.com/hardboydu/notebook/master/regexp/regexpimpl/grep4p.png)

Time to match a?<sup>n</sup>a<sup>n</sup> against a<sup>n</sup>. 使用 a?<sup>n</sup>a<sup>n</sup> 匹配字符串 a<sup>n</sup> 所花费的时间

Let's use superscripts to denote string repetition, so that a?<sup>3</sup>a<sup>3</sup> is shorthand for a?a?a?aaa. The two graphs plot the time required by each approach to match the regular expression a?<sup>n</sup>a<sup>n</sup> against the string a<sup>n</sup>.

让我们使用上标来表示重复字符，例如 a?<sup>3</sup>a<sup>3</sup> 为 a?a?a?aaa 的缩写。上边两幅图表示了正则表达式a?<sup>n</sup>a<sup>n</sup> 匹配字符串a<sup>n</sup> 分别使用两种方法所花费的时间。

Notice that Perl requires over sixty seconds to match a 29-character string. The other approach, labeled Thompson NFA for reasons that will be explained later, requires twenty microseconds to match the string. That's not a typo. The Perl graph plots time in seconds, while the Thompson NFA graph plots time in microseconds: the Thompson NFA implementation is a million times faster than Perl when running on a miniscule 29-character string. The trends shown in the graph continue: the Thompson NFA handles a 100-character string in under 200 microseconds, while Perl would require over 1015 years. (Perl is only the most conspicuous example of a large number of popular programs that use the same algorithm; the above graph could have been Python, or PHP, or Ruby, or many other languages. A more detailed graph later in this article presents data for other implementations.)
注意，Perl在匹配29个字符串
