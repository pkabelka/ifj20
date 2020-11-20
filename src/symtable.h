/**
 * @brief Binary tree symbol table implementation
 *
 * @author Kryštof Glos <xglosk01@stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "str.h"
#include "stack.h"

#define SYMTABLE_SUCCESS 0

typedef enum
{
    NONE,
    INT,
    FLOAT64,
    STRING,
} data_type;

/**
 * @struct Symbol stack element
 *
 * The stack is implemented as a singly linked list
 */
typedef struct symstack_el
{
    struct symstack_el *next;
    string *params;
    bool defined;
    data_type type;
} *symstack_el_p;

/**
 * @brief Initializes symbol stack to NULL
 * @param s Stack pointer
 */
void symstack_init(symstack_el_p *s);

/**
 * @brief Disposes all elements on the symbol stack
 * @param s Stack pointer
 */
void symstack_dispose(symstack_el_p *s);

/**
 * @brief Pushes data on top of the symbol stack
 * @param data Pointer to some data
 * @return true if push completed successfully
 */
bool symstack_push(symstack_el_p *s, bool defined, data_type type);

/**
 * @brief Removes an element from top of the symbol stack
 */
void symstack_pop(symstack_el_p *s);

/**
 * @struct Symbol table node
 */
typedef struct stnode {
    char *key; // node key
    symstack_el_p data;
    struct stnode *lnode; // left subtree
    struct stnode *rnode; // right subtree
} *stnode_ptr;

/**
 * @brief Initializes new symtable
 * @param root tree to be initialized
 */
void symtable_init(stnode_ptr *root);

/**
 * @brief searches const char in given tree, returns matching node
 */
stnode_ptr symtable_search(stnode_ptr root, const char *key);

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

/**
 * @brief Deletes a node in symtable
 * @param root pointer to a tree root pointer
 * @param key key of the node to delete
 */
void symtable_delete_node (stnode_ptr *root, const char *key);

#endif
