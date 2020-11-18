/**
 * @brief Dynamic string implementation
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "str.h"

bool str_init(string *s)
{
    if ((s->str = (char*) malloc(sizeof(STR_ALLOC_CONST))) == NULL)
    {
        return false;
    }

    s->str[0] = '\0';
    s->len = 0;
    s->mem_size = STR_ALLOC_CONST;
    return true;
}

void str_free(string *s)
{
    free(s->str);
    s->mem_size = 0;
    s->len = 0;
}

void str_clear(string *s)
{
    s->str[0] = '\0';
    s->len = 0;
}

bool str_add(string *s, char c)
{
    if ((s->len + sizeof(char)) >= s->mem_size)
    {
        if ((s->str = (char*) realloc(s->str, s->len + STR_ALLOC_CONST)) == NULL)
        {
            return false;
        }
        s->mem_size = s->len + STR_ALLOC_CONST;
    }

    s->str[s->len++] = c;
    s->str[s->len] = '\0';
    return true;
}

bool str_add_const(string *s, const char *cstr)
{
    unsigned int cstr_len = (unsigned int)strlen(cstr);
    if ((s->len + cstr_len + sizeof(char)) >= s->mem_size)
    {
        if ((s->str = (char*) realloc(s->str, s->len + cstr_len + sizeof(char))) == NULL)
        {
            return false;
        }
        s->mem_size = s->len + cstr_len + sizeof(char);
    }

    s->len += cstr_len;
    strcat(s->str, cstr);
    s->str[s->len] = '\0';
    return true;
}

bool str_copy(string *src, string *dst)
{
    if ((src->len + sizeof(char)) >= dst->mem_size)
    {
        if ((dst->str = (char*) realloc(dst->str, src->len + sizeof(char))) == NULL)
        {
            return false;
        }
        dst->mem_size = src->len + sizeof(char);
    }

    strcpy(dst->str, src->str);
    dst->len = src->len;
    return true;
}

int str_cmp(string *s1, string *s2)
{
    return strcmp(s1->str, s2->str);
}

int str_cmp_const(string *s1, const char *s2)
{
    return strcmp(s1->str, s2);
}