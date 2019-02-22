### 2.3.2 Two’s-Complement Addition

With two’s-complement addition, we must decide what to do when the result is either too large (positive) or too small (negative) to represent. Given integer values x and y in the range $- 2 ^ { w - 1 } \leq x , y \leq 2 ^ { w - 1 } - 1$, their sum is in the range $- 2 ^ { w } \leq x + y \leq 2 ^ { w } - 2$, potentially requiring w + 1 bits to represent exactly. As before, we avoid ever-expanding data sizes by truncating the representation to w bits. The result is not as familiar mathematically as modular addition, however. Let us define $x + _ { w } ^ { t } y$ to be the result of truncating the integer sum $x + y$ to be $w$ bits long and then viewing the result as a two’s-complement number.

principle: Two’s-complement addition 

For integer values $x$ and $y$ in the range $- 2 ^ { w - 1 } \leq x , y \leq 2 ^ { w - 1 } - 1$

$$
x + _ { w } ^ { \mathrm { t } } y = \left.\{ \begin{array} { l l }
    { x + y - 2 ^ { w } , } & { 2 ^ { w - 1 } \leq x + y } & { \text { Positive overflow } } \\
    { x + y , } & { - 2 ^ { w - 1 } \leq x + y < 2 ^ { w - 1 } } & { \text { Normal } } \\
    { x + y + 2 ^ { w } , } & { x + y < - 2 ^ { w - 1 } } & { \text { Negative overflow } } \end{array} \right.
$$

$x + _ { w } ^ { \mathrm { t } } y = \left\{ \begin{array} { l l } { x + y - 2 ^ { w } , } & { 2 ^ { w - 1 } \leq x + y } & { \text { Positive overflow } } \\ { x + y , } & { - 2 ^ { w - 1 } \leq x + y < 2 ^ { w - 1 } \text { Normal } } \\ { x + y + 2 ^ { w } , } & { x + y < - 2 ^ { w - 1 } } & { \text { Negative overflow } } \end{array} \right.$
