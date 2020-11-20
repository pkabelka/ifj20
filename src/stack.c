/**
 * @brief Generic stack implementation
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "stack.h"

void stack_init(stack *s)
{
    s->top = NULL;
    s->count = 0;
}

bool stack_push(stack *s, void *data)
{
    struct stack_el *tmp = malloc(sizeof(struct stack_el));
    if (tmp == NULL)
    {
        return false;
    }

    tmp->next = s->top;
    tmp->data = data;
    s->top = tmp;
    s->count += 1;
    return true;
}

void stack_pop(stack *s)
{
    if (s->top == NULL)
    {
        return;
    }

    struct stack_el *tmp = s->top;
    s->top = tmp->next;
    s->count -= 1;
    free(tmp);
}

void stack_dispose(stack *s)
{
    while (s->top != NULL)
    {
        stack_pop(s);
    }
}
