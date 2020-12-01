/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Binary tree symbol table implementation
 *
 * @author Kryštof Glos <xglosk01 at stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "str.h"
#include "stack.h"

typedef enum
{
    NONE,
    INT,
    FLOAT64,
    STRING,
} data_type;

/**
 * @struct Symbol table node data
 */
typedef struct stdata
{
    bool defined;
    data_type type;
} *stdata_p;

/**
 * @struct Symbol table node
 */
typedef struct stnode {
    char *key; // node key
    void *data;
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
 * @param root pointer to the tree to be disposed
 * @param free_data pointer to a function which frees the node data
 */
void symtable_dispose(stnode_ptr *root, void (*free_data)(void *));

/**
 * @brief Deletes a node in symtable
 * @param root pointer to a tree root pointer
 * @param key key of the node to delete
 */
void symtable_delete_node (stnode_ptr *root, const char *key, void (*free_data)(void *));

#endif
