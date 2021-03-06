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

注意，Perl在匹配29个字符时使用了超过60秒的时间，另一种方法Thompson NFA，使用了20微妙的时间，这不是笔误，原因我们将在之后进行解释。上图中Perl使用的是秒，Thompson NFA使用的是微妙：Thompson NFA仅仅在匹配29字符长度的字符串时比Perl快了100万倍。从图的趋势可以看出：在匹配100个字符的字符串时Thompson NFA使用了200毫秒，而Perl可能需要超过1015年。（Perl只不过是使用这种算法的一个比较突出的例子；上图也可能是Python、PHP、Ruby或者其他语言，在本文后我们将提供其他实现的数据图）

It may be hard to believe the graphs: perhaps you've used Perl, and it never seemed like regular expression matching was particularly slow. Most of the time, in fact, regular expression matching in Perl is fast enough. As the graph shows, though, it is possible to write so-called “pathological” regular expressions that Perl matches very very slowly. In contrast, there are no regular expressions that are pathological for the Thompson NFA implementation. Seeing the two graphs side by side prompts the question, “why doesn't Perl use the Thompson NFA approach?” It can, it should, and that's what the rest of this article is about.

上边的图很难让人相信：也许你习惯使用Perl，并且Perl的正则表达式的匹配看起来并不慢，事实上，Perl的正则表达式匹配已经足够快了。如上图所示，Perl匹配的非常慢的原因是因为我们使用了病态的正则表达式。但对于Thompson NFA实现来说没有病态正则表达式一说。通过对比上边两个图我们提出一个问题，为什么Perl不适用Thompson NFA的实现？，它可以，它应该，这也是本文接下来将要介绍的。

Historically, regular expressions are one of computer science's shining examples of how using good theory leads to good programs. They were originally developed by theorists as a simple computational model, but Ken Thompson introduced them to programmers in his implementation of the text editor QED for CTSS. Dennis Ritchie followed suit in his own implementation of QED, for GE-TSS. Thompson and Ritchie would go on to create Unix, and they brought regular expressions with them. By the late 1970s, regular expressions were a key feature of the Unix landscape, in tools such as ed, sed, grep, egrep, awk, and lex.

从历史上看，正则表达式是计算机科学中理论结合实践的一个非常好的例子。它们最开始是由理论学家研究的一个简单的计算模型，然后由Ken Thompson通过由他自己在CTSS上实现的编辑器QED，将正则表达式介绍给了程序员，接着Dennis Ritchie在GE-TSS上实现了自己的QED，Thompson和Ritchie接下来创建了Unix，并为Unix实现了正则表达式，在20世纪70年代，正则表达式成为了Unix上的关键功能，在Unix的一些工具里，例如ed，sed，grep，egrep，awk，lex。

Today, regular expressions have also become a shining example of how ignoring good theory leads to bad programs. The regular expression implementations used by today's popular tools are significantly slower than the ones used in many of those thirty-year-old Unix tools.

今天，正则表达式也是一个好的理论导致坏的实现的典型例子，在今天流行的工具中实现的正则表达式明显的比30年前的Unix工具的实现慢。

This article reviews the good theory: regular expressions, finite automata, and a regular expression search algorithm invented by Ken Thompson in the mid-1960s. It also puts the theory into practice, describing a simple implementation of Thompson's algorithm. That implementation, less than 400 lines of C, is the one that went head to head with Perl above. It outperforms the more complex real-world implementations used by Perl, Python, PCRE, and others. The article concludes with a discussion of how theory might yet be converted into practice in the real-world implementations.

本文首先回顾了好的理论：正则表达式，有限自动机，还有由 Ken Thompson 在20世纪60年代中期实现的正则表达式搜索算法，接下来将理论付诸实现，也就是Thompson算法的简单实现。这个实现使用了不到200行的C代码，也就是上边和Perl比较的那个实现。这个实现也由于现在Perl，Python，PCRE等使用的更复杂的实现。最后本文总结性的讨论了如何将理论转换成实践。

## Regular Expressions 正则表达式

Regular expressions are a notation for describing sets of character strings. When a particular string is in the set described by a regular expression, we often say that the regular expression matches the string.

正则表达式是描述字符串集合的符号。当一个特定的字符串在正则表达式描述的集合中，那么我们通常说正则表达式匹配字符串。

The simplest regular expression is a single literal character. Except for the special metacharacters * + ? ( ) |, characters match themselves. To match a metacharacter, escape it with a backslash: \\+ matches a literal plus character.

最简单的正则表达式就是一个单一可见字符。除了特殊的元字符 * + ? ( ) |，还包括字符本身。如果要匹配元字符本身，需要使用反斜杠进行转义：\\+ 可以匹配 + 号。

Two regular expressions can be alternated or concatenated to form a new regular expression: if e1 matches s and e2 matches t, then e1|e2 matches s or t, and e1e2 matches st.

两个正则表达式可以串联或并联成一个新的正则表达式：如果e1可以匹配s，e2可以匹配t，则e1|e2 可以匹配 s 或 t，e1e2可以匹配st。

The metacharacters \*, +, and ? are repetition operators: e1\* matches a sequence of zero or more (possibly different) strings, each of which match e1; e1+ matches one or more; e1? matches zero or one.

元字符 \*，+，? 为重复运算符：e1\* 匹配零个或者多个字符串（可能不同）序列，包括e1；e1+ 匹配一个或者多个；e1?匹配另个或者一个。

The operator precedence, from weakest to strongest binding, is first alternation, then concatenation, and finally the repetition operators. Explicit parentheses can be used to force different meanings, just as in arithmetic expressions. Some examples: ab|cd is equivalent to (ab)|(cd); ab\* is equivalent to a(b\*).

运算符的优先级，从弱到强进行结合，先串联，然后并联，最后是重复运算符。圆括号可以改变优先级顺序，就像算数表达式那样。例如：ab|cd 等价于 (ab)|(cd)；ab\*等价于 a(b\*)。

The syntax described so far is a subset of the traditional Unix egrep regular expression syntax. This subset suffices to describe all regular languages: loosely speaking, a regular language is a set of strings that can be matched in a single pass through the text using only a fixed amount of memory. Newer regular expression facilities (notably Perl and those that have copied it) have added many new operators and escape sequences. These additions make the regular expressions more concise, and sometimes more cryptic, but usually not more powerful: these fancy new regular expressions almost always have longer equivalents using the traditional syntax.

上文描述的语法是传统Unixegrep正则表达式语法的一个子集，这个子集足以描述所有的正则语言：不严格的说，一个正则语言是字符串的集合，在只需要固定内存，并且只需要遍历一遍文本即可匹配到所有的字符串。新的正则表达式引擎（尤其是Perl以及从Perl派生的工具）添加了很多新的运算符和转义序列。这些新的特性使得正则表达式变得很简洁，有时候变得很神秘，但通常并不是很有用：这些新奇的新的正则表达式都有一个等价的传统与法表达式，这些传统语法表达式通常都很长。

One common regular expression extension that does provide additional power is called backreferences. A backreference like \1 or \2 matches the string matched by a previous parenthesized expression, and only that string: (cat|dog)\1 matches catcat and dogdog but not catdog nor dogcat. As far as the theoretical term is concerned, regular expressions with backreferences are not regular expressions. The power that backreferences add comes at great cost: in the worst case, the best known implementations require exponential search algorithms, like the one Perl uses. Perl (and the other languages) could not now remove backreference support, of course, but they could employ much faster algorithms when presented with regular expressions that don't have backreferences, like the ones considered above. This article is about those faster algorithms.

一种常见的正则表达式扩展提供了一种有用的特性，叫做反向引用。一个反向引用，例如 \\1或者 \\2匹配的字符串，是之前括号中表达式匹配的字符串：(cat|dog)\1 匹配 catcat 和 dogdog，但不能匹配 catdog 或者 dogcat。就理论术语而言，带有反向引用的正则表达式并不是正则表达式。反向引用引入带来了很高的成本，在最坏的情况，最有名的实现，就像Perl正在使用的，搜索算法需要指数级的消耗。Perl(包括其他语言)现在不能移除反向引用支持，当然，如果没有反向引用，他们可以使用更快的算法，正如上文所述，本文是关于更快算法的。
