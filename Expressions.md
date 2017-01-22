# Expressions, Conversion and Evaluation with C

In this tutorial, I will be writing in detail about an important programming concept i.e. Algebraic Expressions, Different expression notations like Prefix, Postfix and Infix and Evaluation of the expressions. How to convert an expression from one notation to another? How to evaluate Algebraic Expression in Computers?

在这篇教程里，我将会详细的介绍一个重要的编程概念，代数表达式，表达式表示方法，例如前缀表示，后缀表示和中缀表示，表达式的求值。怎样将表达式从一种表示法转换到另一种表示法？在计算机中怎样对代数表达式求值？

Each and every concept is backed by Algorithms, illustrative examples and a programs in C to help new programmers understand concepts more clearly.

每个概念都是由算法支撑的，说明性的实例和C语言程序能够帮助程序员更清楚的理解这些概念。

We will be using the concepts of Stack and Binary Tree to convert and evaluate the expressions, so the reader is required to be clear with fundamentals of these concepts.

我们在对表达式进行转换和求值时使用了堆栈和二叉树，所以读者需要这些概念的基本原理。

Topics covered by this tutorial

本教程涵盖的主题

  * What is an Algebraic Expression?
  * 什么是代数表达式
  * What are different notations of representing expressions like Infix, Prefix, and Postfix?
  * 代数表达式的表示方法，例如中缀表示，前缀表示和后缀表示，之间有什么不同？
  * Why do we need different notations for the same expression?
  * 为什么同样的一个表达式需要多种不同的表示方法？
  * Why do we need to convert the expressions from one notation to another?
  * 为什么需要将表达式从一种表示方法转换到另一种？
  * How can we convert the expressions from one notation to another? (Algorithms, Programs, examples)
  * 我们怎样将表达式从一种表示方法转换到另一种（算法，程序，实例）
  * Expression Trees
  * 表达式树
  * How can we evaluate an expression? (Algorithm, Program)
  * 我们怎么样对表达式求值
  * Reader's Exercise.
  * 读者练习

## Algebraic Expressions, an Introduction:

An algebraic expression is a legal combination of operands and the operators. Operand is the quantity (unit of data) on which a mathematical operation is performed. Operand may be a variable like x, y, z or a constant like 5, 4,0,9,1 etc. Operator is a symbol which signifies a mathematical or logical operation between the operands. Example of familiar operators include +,-,*, /, ^ ( throughout the tutorial '^' will be referred to as Exponential Operator ) etc. Considering these definitions of operands and operators now we can write an example of expression as x+y*z. Note the phrase "LEGAL combination" in the definition of the Algebraic Expression, in aforementioned example of expression x+y*z, the operands x , y, z and the operators +,* form some legal combination. Take another example +xyz*, in this expression operators and operands do not make any LEGAL combination; this expression is not a valid algebraic expression.

代数表达式是操作数和操作符的合法组合，操作数是数据运算的量（数据单元）。操作数可以是变量，例如x、y、z，或者是一个常量，例如5、4、0、9、1等等。操作符是表示操作数之间算数或者逻辑运算的符号，例如常见的操作符，包括+、-、*、/、^（本教程 '^'为指数操作符）等等。根据操作符和操作数的定义我们可以写出一个表达式的例子 x + y * z。注意代数表达式定义中的“合法组合”，在上边例子中的表达式 x + y * z，操作数是x、y、z，操作符是 +、*，操作数和操作符形成了合法的组合。举另一个例子 +xyz*，在这个表达式中操作符和操作数没有组成一个合法组合；这个表达式不是一个有效的代数表达式。

An Algebraic Expression can be represented using three different notations:

一个代数表达式可以有三种表示方法：

 * **INFIX**: From our schools times we have been familiar with the expressions in which operands surround the operator, e.g. x+y, 6*3 etc this way of writing the Expressions is called infix notation.
 * **中缀表示** ：操作符在操作数的两边，在学校时我们已经很熟悉这种方法，例如 x + y、6 * 3等等，这种书写表达式的方法叫做中缀表示法
 * **PREFIX**: Prefix notation also Known as Polish notation, is a symbolic logic invented by Polish mathematician Jan Lukasiewicz in the 1920's. In the prefix notation, as the name only suggests, operator comes before the operands, e.g. +xy, *+xyz etc.
 * **前缀表示**：前缀表示也叫做波兰表示法，它是波兰数学家Jan Lukasiewicz在20世纪20年代发明的一种符号逻辑，正如名字所定义的
 * **POSTFIX**: Postfix notation are also Known as Reverse Polish notation. They are different from the infix and prefix notations in the sense that in the postfix notation, operator comes after the operands, e.g. xy+, xyz+* etc.
