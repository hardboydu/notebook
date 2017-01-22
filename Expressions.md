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

Due to above mentioned problem of considering operators' Priority and Associativity while evaluating an expression using infix notation, we use prefix and postfix notations. Both prefix and postfix notations have an advantage over infix that while evaluating an expression in prefix or postfix form we need not consider the Priority and Associativity of the operators. E.g. x/y*z becomes */xyz in prefix and xy/z* in postfix. Both prefix and postfix notations make Expression Evaluation a lot easier. (we will discuss this in detail, later in this tutorial)

由于上述问题，在求解中缀表达式时需要考虑到操作符的优先级和结合律，所以我们使用前缀表示和后缀表示。对于中缀表示法来说，前缀表示和后缀表示的优势就是在求解表达式时不用考虑操作符的优先级和结合律。例如 x / y * z 前缀表达式为 */xyz，后缀表达式为 xy/z*。前缀表示和后缀表示可以使表达式求解变得非常容易。（接下来我们会详细讨论）

But it is not easy to remember and manually write expressions in prefix or postfix form e.g. which one of following equations is easy to remember (x+y)/z*a (Infix) or xy+z/a* (Postfix)?

但是前缀表示和后缀表示不是很容易记忆和手写，例如可以比较下边两个表达式那个容易记忆 (x + y) / z * a（中缀表示）或者 xy+z/a*（后缀表示）？

So, what is actually done is expression is scanned from user in infix form; it is converted into prefix or postfix form and then evaluated without considering the parenthesis and priority of the operators.

所以我们实际上要做的就是扫描用户以中缀表示的表达式，将其转换成前缀或后缀形式，然后求解时就不用考虑括号和运算符的优先级了。

Now let us move on the programming part, How to convert an expression from one notation to another? Well there are two ways to convert expression from one notation to another. First one uses **Stack** and second method uses **Expression trees**.

现在，让我们转移到编程部分，怎样将表达式从一种形式转换到另一种？有两种方法转换表达式从一种形式转换成另一种。第一种方法是使用**堆栈**，第二种方法是使用**表达式树**。

As there are 3 notations namely prefix, infix, postfix , there are a total of 6 conversions that can be done ,i.e. infix -> prefix, infix -> postfix, prefix -> infix, prefix -> postfix, postfix -> prefix, postfix -> infix.

有三种表示方法，前缀表示，中缀表示，后缀表示，有六种转换方法，中缀->前缀、中缀->后缀、前缀->中缀、前缀->后缀、后缀->前缀、后缀->中缀。

For the first 2 conversions we will be using stack and for remaining 6 conversions we will be using Binary Expression Trees.

前两种转换方法使用堆栈，剩余的使用二叉表达式树。

To convert an expression from infix to prefix and postfix, we are going to use stack. Those who do not know what is a stack, here are a few words about it. Stack is a special type of data structure in which items are removed in reverse order from that they are added. Stack follows Last In First Out (LIFO) pattern. Adding an element to stack is called PUSH and removing an item from stack is called POP.

将表达式从中缀表示转化成前缀和后缀，我们将会使用堆栈，对于不了解堆栈的读者，接下来我会简短的描述它，堆栈是一种特殊类型的数据结构，删除元素的顺序和添加顺序相反，堆栈遵循后进先出（LIFO）模式。添加一个元素调用PUSH删除一个元素调用POP。

### Converting Expression from Infix to Postfix using STACK

To convert an expression from infix to postfix, we are going to use stack.

将一个表达式从中缀表示转换成后缀表示，我们将使用堆栈。

**Algorithm**
 1. Examine the next element in the input. 检查下一个输入的元素
 2. If it is operand, output it. 如果是操作数，输出
 3. If it is opening parenthesis, push it on stack. 如果是左括号，PUSH到堆栈
 4. If it is an operator, then 如果是操作符，则
   1. If stack is empty, push operator on stack. 如果堆栈是空的，PUSH操作符到堆栈
   2. If the top of stack is opening parenthesis, push operator on stack. 如果栈顶是左括号，PUSH操作符到堆栈
   3. If it has higher priority than the top of stack, push operator on stack. 如果操作符的优先级大于栈顶的操作符，PUSH操作符到堆栈。
   4. Else pop the operator from the stack and output it, repeat step 4. 其他情况，从堆栈POP出操作符并输出，然后跳到步骤4。
 5. If it is a closing parenthesis, pop operators from stack and output them until an opening parenthesis is encountered. pop and discard the opening parenthesis. 如果是右括号，从堆栈POP出操作符并输出，直到遇到左括号，POP出左括号并抛弃。
 6. If there is more input go to step 1. 如果有输入，跳到步骤1
 7. If there is no more input, unstack the remaining operators to output. 如果没有输入，将堆栈剩余的操作符POP，并输出。

**Example**

Suppose we want to convert 2*3/(2-1)+5*(4-1) into Postfix form, following table shows how it works:

我们将表达式 2*3/(2-1)+5*(4-1) 转换到后缀表示，下边的表格描述了详细过程：

| Char Scanned | Stack Contents | Postfix Expression |
|--------------|----------------|--------------------|
| 2            | Empty          | 2                  |
| *            | *              | 2                  |
| 3            | *              | 23                 |
| /            | /              | 23*                |
| (            | /(             | 23*                |
| 2            | /(             | 23*2               |
| -            | /(-            | 23*2               |
| 1            | /(-            | 23*21              |
| )            | /              | 23*21-             |
| +            | +              | 23*21-/            |
| 5            | +              | 23*21-/5           |
| *            | +*             | 23*21-/5           |
| (            | +*(            | 23*21-/5           |
| 4            | +*(            | 23*21-/54          |
| -            | +*(-           | 23*21-/54          |
| 1            | +*(-           | 23*21-/541         |
| )            | +*             | 23*21-/541-        |
|              | Empty          | 23\*21-/541-\*+    |

So, the Postfix Expression is 23*21-/541-*+

最终，后缀表达式为 23*21-/541-*+

Refer program #1 for infix to postfix Conversion

程序1 为中缀转后缀

### Converting Expression from Infix to Prefix using STACK

It is a bit trickier algorithm, in this algorithm we first reverse input expression so that a+b*c will become c*b+a and then we do the conversion and then again output string is reversed. Doing this has an advantage that except for some minor modifications algorithm for Infix->Prefix remains almost same as the one for Infix->Postfix.

这是一个有点棘手的算法，在这个算法里，我们首先要反转输入的表达式，例如 a+b*c 会被转换成 c*b+a，然后再进行转换，最后输出的字符串是反向的。这样做的好处是从中缀转后缀算法修改成中缀转前缀算法，不用做什么修改。

**Algorithm**
 1. Reverse the input string.
 2. Examine the next element in the input.
 3. If it is operand, add it to output string.
 4. If it is Closing parenthesis, push it on stack.
 5. If it is an operator, then
   1. If stack is empty, push operator on stack.
   2. If the top of stack is closing parenthesis, push operator on stack.
   3. If it has same or higher priority than the top of stack, push operator on stack.
   4. Else pop the operator from the stack and add it to output string, repeat step 5.
 6. If it is a opening parenthesis, pop operators from stack and add them to output string until a closing parenthesis is encountered. Pop and discard the closing parenthesis.
 7. If there is more input go to step 2
 8. If there is no more input, unstack the remaining operators and add them to output string.
 9. Reverse the output string.



| Char Scanned | Stack Contents(Top on right) | Prefix Expression(right to left) |
|--------------|------------------------------|----------------------------------|
|              |                              |                                  |
| )            | )                            |                                  |
| 1            | )                            | 1                                |
| -            | )-                           | 1                                |
| 4            | )-                           | 14                               |
| (            | Empty                        | 14-                              |
| *            | *                            | 14-                              |
| 5            | *                            | 14-5                             |
| +            | +                            | 14-5*                            |
| )            | +)                           | 14-5*                            |
| 1            | +)                           | 14-5*1                           |
| -            | +)-                          | 14-5*1                           |
| 2            | +)-                          | 14-5*12                          |
| (            | +                            | 14-5*12-                         |
| /            | +/                           | 14-5*12-                         |
| 3            | +/                           | 14-5*12-3                        |
| *            | +/*                          | 14-5*12-3                        |
| 2            | +/*                          | 14-5*12-32                       |
|              | Empty                        | 14-5\*12-32\*/+                  |

