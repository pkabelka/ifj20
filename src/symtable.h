/**
 * @brief Binary tree symbol table implementation
 *
 * @author Kryštof Glos <xglosk01@stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "str.h"

#define SYMTABLE_SUCCESS 0

typedef struct stnode{
    char *key; // node key
    struct stnode *lnode; // left subtree
    struct stnode *rnode; // right subtree
} *stnode_ptr;

/**
 * @struct Stack element pointer
 * 
 * The stack is implemented as a singly linked list.
 */
typedef struct stack_el {
    struct stack_el *next;
    stnode_ptr data;
} *stack_el_p;

/**
 * @brief Initializes stack to NULL
 * @param s Pointer to stack element pointer
 */
void stack_init(stack_el_p *s);

/**
 * @brief Disposes all elements on the stack
 * @param s Pointer to stack element pointer
 */
void stack_dispose(stack_el_p *s);

/**
 * @brief Pushes data on top of the stack
 * @param data Pointer to stnode struct
 * @return SYMTABLE_SUCCESS for correct allocation, ERR_INTERNAL otherwise
 */
int stack_push(stack_el_p *s, stnode_ptr data);

/**
 * @brief Removes the element from top of the stack
 * @param data Pointer to stnode struct
 */
void stack_pop(stack_el_p *s);

/**
 * @brief Initializes new symtable
 * @param root tree to be initialized
*/
void symtable_init(stnode_ptr *root);

/**
 * @brief searches const char in given tree, returns matching node
**/
stnode_ptr symtable_search(stnode_ptr, const char *key);

/**
 * @brief Inserts in symtable new node with value of Content
 * @param c const char to be inserted as string
*/
stnode_ptr symtable_insert (stnode_ptr *root, const char *key, bool *error);

/**
 * @brief Disposes all nodes in tree and frees memory
 * @param root tree to be disposed
*/
void symtable_dispose(stnode_ptr *root);
