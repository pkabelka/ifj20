/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Binary tree symbol table interface
 *
 * @author Kryštof Glos <xglosk01 at stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#include "symtable.h"
#include "error.h"
#include "stack.h"

void symtable_init(stnode_ptr *root)
{
    *root = NULL;
}

stnode_ptr symtable_insert(stnode_ptr *root, const char *key, bool *error)
{
    *error = false;

    if (*root == NULL)
    {
        stnode_ptr new = malloc(sizeof(struct stnode));
        if (new == NULL)
        {
            *error = true;
            return NULL;
        }
        new->lnode = NULL;
        new->rnode = NULL;
        new->key = (char *)malloc(strlen(key) + sizeof(char));
        if (new->key == NULL)
        {
            *error = true;
            return NULL;
        }
        strcpy(new->key, key);

        *root = new;
        return new;
    }

    stnode_ptr tmp = *root;
    while (tmp != NULL)
    {
        int comp = strcmp(tmp->key, key);
        if (comp > 0) // New node will be inserted on the left side of current
        {
            if (tmp->lnode != NULL)
            {
                tmp = tmp->lnode; // No free space, we go further
            }
            else
            {
                stnode_ptr new = malloc(sizeof(struct stnode));
                if (new == NULL)
                {
                    *error = true;
                    return NULL;
                }

                new->lnode = NULL;
                new->rnode = NULL;
                new->key = (char *)malloc(strlen(key) + sizeof(char));
                if (new->key == NULL)
                {
                    *error = true;
                    return NULL;
                }
                strcpy(new->key, key);

                tmp->lnode = new; // new node is on the left
                return new;
            }
        }
        else if (comp < 0) // new node will be inserted on the left side of current
        {
            if (tmp->rnode != NULL)
            {
                tmp = tmp->rnode; // No free space, we go futher
            }
            else
            {
                stnode_ptr new = malloc(sizeof(struct stnode));
                if (new == NULL)
                {
                    *error = true;
                    return NULL;
                }

                new->lnode = NULL;
                new->rnode = NULL;
                new->key = (char *)malloc(strlen(key) + sizeof(char));
                if (new->key == NULL)
                {
                    *error = true;
                    return NULL;
                }
                strcpy(new->key, key);

                tmp->rnode = new; // new node will be inserted on the right side of current
                return new;
            }
        }
        else
        {
            return NULL;
        }
    }
    return NULL; // prevents warning: control reaches end of non-void function
}

stnode_ptr symtable_search(stnode_ptr root, const char *key)
{
    if (root == NULL)
    {
        return NULL;
    }

    stnode_ptr tmp = root;
    while (tmp != NULL)
    {
        int comp = strcmp(key, tmp->key);
        if (comp == 0)
        {
            return tmp;
        }
        else if (comp < 0)
        {
            tmp = tmp->lnode;
        }
        else if (comp > 0)
        {
            tmp = tmp->rnode;
        }
    }
    return NULL;
}

/**
 * @brief Goes trhough left brand of subtree untill gets on the most left node
 * @param ptr node to walk through
*/
static void leftmost_inorder(stnode_ptr ptr, stack *st)
{
    while (ptr != NULL)
    {
        stack_push(st, ptr);
        ptr = ptr->lnode;
    }
}

void symtable_dispose(stnode_ptr *root, void (*free_data)(void *))
{
    if (*root == NULL)
    {
        return;
    }

    stack st;
    stack_init(&st);

    stnode_ptr ptr = *root;
    leftmost_inorder(ptr, &st);
    while (st.top != NULL)
    {
        ptr = st.top->data;
        stack_pop(&st, stack_nofree);
        leftmost_inorder(ptr->rnode, &st);
        free_data(ptr->data);
        free(ptr->key);
        free(ptr);
    }
    *root = NULL;
}

static void replace_by_rightmost (stnode_ptr ptr_replaced, stnode_ptr *root, void (*free_data)(void *))
{
    if (*root != NULL)
    {
        if ((*root)->rnode != NULL)
        {
            replace_by_rightmost(ptr_replaced, &(*root)->rnode, free_data);
        }
        else
        {
            free(ptr_replaced->key);
            free_data(ptr_replaced->data);
            ptr_replaced->data = (*root)->data;
            ptr_replaced->key = (*root)->key;
            if ((*root)->lnode != NULL)
            {
                stnode_ptr old_root = *root;
                *root = (*root)->lnode;
                free(old_root);
            }
            else
            {
                free(*root);
                *root = NULL;
            }
        }
    }
}

void symtable_delete_node (stnode_ptr *root, const char *key, void (*free_data)(void *))
{
    if (*root == NULL)
    {
        return;
    }
    int comp = strcmp(key, (*root)->key);
    if (comp < 0)
    {
        symtable_delete_node(&(*root)->lnode, key, free_data);
    }
    else if (comp > 0)
    {
        symtable_delete_node(&(*root)->rnode, key, free_data);
    }
    else
    {
        if ((*root)->lnode == NULL && (*root)->rnode == NULL)
        {
            free_data((*root)->data);
            free((*root)->key);
            free(*root);
            *root = NULL;
        }
        else if ((*root)->lnode != NULL && (*root)->rnode != NULL)
        {
            replace_by_rightmost(*root, &(*root)->lnode, free_data);
        }
        else if ((*root)->lnode == NULL)
        {
            stnode_ptr old_root = *root;
            *root = (*root)->rnode;
            free_data(old_root->data);
            free(old_root->key);
            free(old_root);
        }
        else if ((*root)->rnode == NULL)
        {
            stnode_ptr old_root = *root;
            *root = (*root)->lnode;
            free_data(old_root->data);
            free(old_root->key);
            free(old_root);
        }
    }
}
