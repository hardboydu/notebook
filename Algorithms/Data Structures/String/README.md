# String

**字符串(string)**：一个以串终结符结束的字符序列，通常，在C语言中以字符数组的形式存在，串终结符为'\0'，在C++中除了兼容C语言的字符串意外，还有STL的内置字符串对象std::string。此外，字符串还可以以链表、树，状态机的形式存在，用以实现对字符串的高级算法。

## Basic Algorithm Refrernce

* C语言需要参照 c standard library [string.h](http://en.cppreference.com/w/c/string/byte) 的相关实现
* C++语言需要参照 STL [std::string](http://en.cppreference.com/w/cpp/string/basic_string) 实现。

## Advanced algorithm Refrernce

* [\[Robert Sedgewick, Kevin Wayne\]\[Algorithms\]\(2014\)](https://www.amazon.com/Algorithms-4th-Robert-Sedgewick/dp/032157351X/ref=sr_1_1?ie=UTF8&qid=1518245090&sr=8-1&keywords=Algorithms+4th) 第5章的讲解
* [\[Thomas H.Cormen, Charles E.Leiserson, Ronald L.Rivest,Clifford Stein\]\[Introduction to Algorithms 3rd\]](https://www.amazon.com/Introduction-Algorithms-3rd-MIT-Press/dp/0262033844/) 第7部分第32章的讲解：string matching
* [\[Steven S. Skiena\]\[The Algorithm Design Manual 2nd\]](https://www.amazon.com/Steven-Skiena-Algorithm-Design-Manual/dp/B00N4EQ1X4/) 第18章的讲解：Set and String Problems
* [\[Robert Sedgewick, Philippe Flajolet\]\[An Introduction to the Analysis of Algorithms, 2nd\] \(2013\)](https://www.amazon.com/Introduction-Analysis-Algorithms-2nd/dp/032190575X) 第8章的讲解：STRINGS AND TRIES
* [\[Anany Levitin\]\[Introduction to the Design and Analysis of Algorithms \(3rd Edition\)\]\(Oct 9, 2011\)](https://www.amazon.com/Introduction-Design-Analysis-Algorithms-3rd/dp/0132316811)  第7章 7.2节：Input Enhancement in String Matching
* [\[Thierry Lecroq\]EXACT STRING MATCHING ALGORITHMS](http://www-igm.univ-mlv.fr/~lecroq/string/)
    * [SMART \(String Matching Algorithms Research Tool\)](https://smart-tool.github.io/smart/)  is a tool which provides a standard framework for researchers in string matching. It helps users to test, design, evaluate and understand existing solutions for the exact string matching problem. Moreover it provides the implementation of (almost) all string matching algorithms and a wide corpus of text buffers.


## Puzzles

* [Leetcode string](https://leetcode.com/tag/string/)

    | No | Title            | Difficulty | Leetcode                                                    | practice |
    |----|------------------|------------|-------------------------------------------------------------|----------|
    | 28 | Implement strStr | Easy       | [Leetcode](https://leetcode.com/problems/implement-strstr/) | practice |
