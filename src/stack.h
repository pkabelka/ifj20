/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Generic stack interface
 *
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#ifndef _STACK_H
#define _STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @struct Stack element
 */
struct stack_el
{
    struct stack_el *next;
    void *data;
};

/**
 * @struct Stack
 * 
 * Generic stack implementation. Pointer to any data is stored in stack element's "data"
 * 
 * The stack is implemented as a singly linked list
 */
typedef struct
{
    struct stack_el *top;
    unsigned int count;
} stack;

/**
 * @brief Initializes stack to NULL
 * @param s Stack pointer
 */
void stack_init(stack *s);

/**
 * @brief Disposes all elements on the stack
 * @param s Stack pointer
 */
void stack_dispose(stack *s, void (*free_data)(void *));

/**
 * @brief Pushes data on top of the stack
 * @param data Pointer to some data
 * @return true if push completed successfully
 */
bool stack_push(stack *s, void *data);

/**
 * @brief Removes an element from top of the stack
 */
void stack_pop(stack *s, void (*free_data)(void *));

/**
 * @brief The function does not do anything useful
 * 
 * Useful when popping from stack when you don't want to free the data
 */
void stack_nofree(void *data);

#endif
