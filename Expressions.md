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
 * **前缀表示**：前缀表示也叫做波兰表示法，它是波兰数学家Jan Lukasiewicz在20世纪20年代发明的一种符号逻辑，正如名字所定义的那样，前缀表示的操作符在操作数之前，例如+xy，*+xyz等等。
 * **POSTFIX**: Postfix notation are also Known as Reverse Polish notation. They are different from the infix and prefix notations in the sense that in the postfix notation, operator comes after the operands, e.g. xy+, xyz+* etc.
 * **后缀表示**：后缀表示也可以叫做逆波兰表示法，它与中缀和前缀表示法不同，后缀表示的操作符在操作数的后边，例如xy+、xyz+*等等。

Now, the obvious question that comes in our mind is, Why to use these weird looking PREFIX and POSTFIX notations when we have a sweet and simple INFIX notation?

现在在我们心里有个明显的问题，为什么要用看起来很奇怪的前缀表示和后缀表示，而不用我们熟悉和简单的中缀表示。

To our surprise INFIX notations are not as simple as they seem specially while evaluating them. To evaluate an infix expression we need to consider Operators’ Priority and Associativity.

在求解中缀表达式并没有那么简单，求解中缀表达式需要考虑操作符的优先级和结合律。

For example, will expression 3+5*4 evaluate to 32 i.e. (3+5)*4 or to 23 i.e. 3+(5*4). To solve this problem Precedence or Priority of the operators were defined. Operator precedence governs evaluation order. An operator with higher precedence is applied before an operator with lower precedence.

例如，表达式 3 + 5 * 4 的解为32，即：(3 + 5) * 4 或者为 23，即：3 + (5 * 4)。在解决这个问题之前，需要定义操作符的优先级。操作符优先级规定了求解顺序。高优先级操作符先求解，低优先级操作符后求解。

Following figure shows operator Precedence in descending order.

下图展示了操作符优先级的逆序表示

Now, as we know the precedence of the operators, we can now evaluate the expression 3+5*4 as 23. But wait, it doesn't end here what about the expression 6/3*2? Will this expression evaluate to 4 i.e. (6/3)*2 or to 1 i.e. 6/(3*2).As both * and the / have same priorities, to solve this conflict, we now need to use Associativity of the operators. Operator Associativity governs evaluation order of the operators of same priority. For an operator with left-Associativity, evaluation is from left to right and for an operator with right-Associativity; evaluation is from right to left.

现在，我们知道了操作符的优先级，我们现在可以求解表达式 3 + 5 * 4 为 23。等一下，还没结束，对于表达式 6 / 3 * 2，可以求解成4，即：(6 / 3) * 2，求解成1，即：6 / (3 *2) 。* 和 / 有相同的优先级，要解决这个冲突，我们需要使用操作符的结合律。操作符的结合律规定了相同优先级的操作符的求解顺序。左相关的操作符，求解顺序从左到右，对于右相关的操作符，求解顺序从右到左。

*, /, +, - have left Associativity. So the expression will evaluate to 4 and not 1.

*、/、+、-都是左相关的，所以上边的表达式解为4而不是1.

**N.B: We use Associativity of the operators only to resolve conflict between operators of same priority.**

**注意：我们使用操作符的结合律只为了解决相同优先级的操作符之间的冲突**
