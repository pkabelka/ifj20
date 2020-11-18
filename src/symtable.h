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

#define FALSE 0
#define TRUE 1
#define MAXSTACK 1024
#define SYMTABLE_SUCCESS 0

typedef struct stnode{
    char *key; // node key
    struct stnode *lnode; // left subtree
    struct stnode *rnode; // right subtree
} *stnode_ptr;

 // TODO: change stack to a linked list
typedef struct{
    stnode_ptr a[MAXSTACK];
    int top;
} tStackP;

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
