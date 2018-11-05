#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct stack_s {
	char *stack ;
	int  top ;
	int  size ;
} stack_t ;

stack_t *stack_new(int size)
{
	stack_t *stack = (stack_t *)malloc(sizeof(stack_t));
	if(stack == NULL) {
		return NULL ;
	}
	stack->stack = (char *)malloc(size);
	if(stack->stack == NULL) {
		free(stack);
		return NULL ;
	}

	stack->size = size ;
	stack->top  = -1 ;

	return stack ;
}

void stack_close(stack_t *stack)
{
	free(stack->stack);
	free(stack);
}

int stack_push(stack_t *stack, char c)
{
	stack->top++ ;
	stack->stack[stack->top] = c ;
	return 0 ;
}

char stack_pop(stack_t *stack)
{
	char c ;

	if(stack->top == -1) {
		return -1 ;
	}
	c = stack->stack[stack->top] ;
	stack->stack[stack->top] = 0 ;
	stack->top-- ;

	return c ;
}

char stack_get(stack_t *stack)
{
	if(stack->top == -1) {
		return -1 ;
	}
	return stack->stack[stack->top] ;
}

int stack_empty(stack_t *stack)
{
	if(stack->top == -1) 
		return 1 ;
	return 0 ;
}

void stack_paint(stack_t *stack)
{
	if(stack->top == -1) 
		printf("%-20s", "empty");
	else 
		printf("%-20s", stack->stack);

}

int priority(char c)
{
	if(c == '^'            ) return 3 ;
	if(c == '*' || c == '/') return 2 ;
	if(c == '+' || c == '-') return 1 ;

	return 0 ;
}

int infix2postfix(const char *infix, char *postfix, int postfix_size)
{
	char *p   = (char *)infix ;
	char *out = postfix ;
	char  c   = 0 ;

	stack_t *stack = stack_new(1000);

	while(*p != 0) {
		if(*p == ' ' || *p == '\t') {
			p++ ;
			continue ;
		}

		if(isdigit(*p) || isalpha(*p)) {
			*out = *p ;
			out++ ;
		} else if (*p == '(') {
			stack_push(stack, *p) ;
		} else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '^') {
			do {
				c = stack_get(stack);
				if(c == -1) {
					stack_push(stack, *p);
					break ;
				} else if(c == '('){
					stack_push(stack, *p);
					break ;
				} else if(priority(*p) > priority(c)) {
					stack_push(stack, *p);
					break ;
				} else {
					c = stack_pop(stack);
					*out = c ;
					out++ ;
				}
			} while(1);
		} else if (*p == ')') {
			c = stack_pop(stack);
			while (c != -1){
				if(c != '(') {
					*out = c ;
					out++ ;
				} else {
					break ;
				}

				c = stack_pop(stack);
			}
		}

		printf("%-5c", *p);
		stack_paint(stack);
		printf("%-20s", postfix);
		printf("\n");

		p++ ;
	}

	c = stack_pop(stack);
	while (c != -1){
		*out = c ;
		out++ ;

		c = stack_pop(stack);
	}

	*out = 0 ;

	stack_close(stack);

	return 0 ;
}

int main(int argc, char *argv[])
{
	char out[100] = {0} ;

	infix2postfix(argv[1], out, 100);

	printf("\n%s\n", out);

	return 0;
}

