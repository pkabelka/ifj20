/**
 * @brief Binary tree symbol table interface
 *
 * @author Kryštof Glos <xglosk01@stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "symtable.h"
#include "error.h"

void stack_init(stack_el_p *s)
{
    *s = NULL;
}

int stack_push(stack_el_p *s, stnode_ptr data)
{
    stack_el_p tmp = malloc(sizeof(struct stack_el));
    if (tmp == NULL)
    {
        return ERR_INTERNAL;
    }

    tmp->next = *s;
    tmp->data = data;
    *s = tmp;
    return SYMTABLE_SUCCESS;
}

void stack_pop(stack_el_p *s)
{
    if (*s == NULL)
    {
        return;
    }

    stack_el_p tmp = (*s)->next;
    free(*s);
    *s = tmp;
}

void stack_dispose(stack_el_p *s)
{
    stack_el_p tmp = *s;
    while (tmp != NULL)
    {
        stack_el_p next = tmp->next;
        free(tmp);
        tmp = next;
    }
    *s = NULL;
}

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
        if (strcmp(key, tmp->key) == 0)
        {
            return tmp;
        }
        else if (strcmp(key, tmp->key) < 0)
        {
            tmp = tmp->lnode;
        }
        else if (strcmp(key, tmp->key) > 0)
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
static void leftmost_inorder(stnode_ptr ptr, stack_el_p *stack)
{
    while (ptr != NULL)
    {
        stack_push(stack, ptr);
        ptr = ptr->lnode;
    }
}

void symtable_dispose(stnode_ptr *root)
{
    if (*root == NULL)
    {
        return;
    }

    stack_el_p stack;
    stack_init(&stack);

    stnode_ptr ptr = *root;
    leftmost_inorder(ptr, &stack);
    while (stack != NULL)
    {
        ptr = stack->data;
        stack_pop(&stack);
        leftmost_inorder(ptr->rnode, &stack);
        free(ptr->key);
        free(ptr);
        ptr = NULL;
    }
    *root = NULL;
}

static void replace_by_rightmost (stnode_ptr ptr_replaced, stnode_ptr *root)
{
    if (*root != NULL)
    {
        if ((*root)->rnode != NULL)
        {
            replace_by_rightmost(ptr_replaced, &(*root)->rnode);
        }
        else
        {
            free(ptr_replaced->key);
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

void symtable_delete_node (stnode_ptr *root, const char *key)
{
    if (*root == NULL)
    {
        return;
    }
    int comp = strcmp(key, (*root)->key);
    if (comp < 0)
    {
        symtable_delete_node(&(*root)->lnode, key);
    }
    else if (comp > 0)
    {
        symtable_delete_node(&(*root)->rnode, key);
    }
    else
    {
        if ((*root)->lnode == NULL && (*root)->rnode == NULL)
        {
            free(*root);
            *root = NULL;
        }
        else if ((*root)->lnode != NULL && (*root)->rnode != NULL)
        {
            replace_by_rightmost(*root, &(*root)->lnode);
        }
        else if ((*root)->lnode == NULL)
        {
            stnode_ptr old_root = *root;
            *root = (*root)->rnode;
            free(old_root);
        }
        else if ((*root)->rnode == NULL)
        {
            stnode_ptr old_root = *root;
            *root = (*root)->lnode;
            free(old_root);
        }
    }
}
