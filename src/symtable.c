/**
 * @brief Binary tree symbol table interface
 *
 * @author Kryštof Glos <xglosk01@stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "symtable.h"
#include "error.h"

// TODO: change stack to a linked list
void SInitP(tStackP *S)
{
    S->top = 0;
}

void SPushP(tStackP *S, stnode_ptr ptr)
/*   ------
** Inserts value on top of stack
**/
{
    /* Při implementaci v poli může dojít k přetečení zásobníku. */
    if (S->top == MAXSTACK)
        printf("Chyba: Došlo k přetečení zásobníku s ukazateli!\n");
    else
    {
        S->top++;
        S->a[S->top] = ptr;
    }
}

stnode_ptr STopPopP(tStackP *S)
/*         --------
** Removes value from top and return its value
**/
{
    /* causes error if stack is empty. */
    if (S->top == 0)
    {
        printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
        return (NULL);
    }
    else
    {
        return (S->a[S->top--]);
    }
}

bool SEmptyP(tStackP *S)
/*   -------
** returns true if stack is empty
**/
{
    return (S->top == 0);
}

void symtable_init(stnode_ptr *root)
{
    *root = NULL;
}

stnode_ptr symtable_insert(stnode_ptr *root, const char *key, bool *error)
{
    *error = false;

    char *s = (char *)malloc(strlen(key) + sizeof(char));
    if (s == NULL)
    {
        *error = true;
        return NULL;
    }

    strcpy(s, key);

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
        new->key = s;
        if (new->key == NULL)
        {
            *error = true;
            return NULL;
        }
        *root = new;
        return new;
    }

    stnode_ptr tmp = *root;
    while (tmp != NULL)
    {
        int comp = strcmp(tmp->key, s);
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
                new->key = s;
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
                new->key = s;
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
static void Leftmost_Inorder(stnode_ptr ptr, tStackP *Stack)
{
    while (ptr != NULL)
    {
        SPushP(Stack, ptr);
        ptr = ptr->lnode;
    }
}

void symtable_dispose(stnode_ptr *root)
{
    if (*root == NULL)
    {
        return;
    }

    // same passage as symtable_inorder
    tStackP s;
    SInitP(&s);

    stnode_ptr ptr = *root;
    Leftmost_Inorder(ptr, &s);
    while (!SEmptyP(&s))
    {
        ptr = STopPopP(&s);
        Leftmost_Inorder(ptr->rnode, &s);
        free(ptr);
        ptr = NULL;
    }
    *root = NULL;
}
