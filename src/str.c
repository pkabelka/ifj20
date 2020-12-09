/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Dynamic string implementation
 * 
 * Implementation of this library was inspired by str.c found in
 * jednoduchy_interpret.zip on the IFJ course website:
 * https://www.fit.vutbr.cz/study/courses/IFJ/public/project/
 *
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#include "str.h"

bool str_init(string *s)
{
    if ((s->str = (char*) malloc(sizeof(char) * STR_ALLOC_CONST)) == NULL)
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
    if ((s->len + 1) >= s->mem_size)
    {
        if ((s->str = (char*) realloc(s->str, (s->len + STR_ALLOC_CONST) * sizeof(char))) == NULL)
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
    if ((s->len + cstr_len + 1) >= s->mem_size)
    {
        if ((s->str = (char*) realloc(s->str, (s->len + cstr_len + 1) * sizeof(char))) == NULL)
        {
            return false;
        }
        s->mem_size = s->len + cstr_len + 1;
    }

    s->len += cstr_len;
    strcat(s->str, cstr);
    s->str[s->len] = '\0';
    return true;
}

bool str_add_var(string *s, ...)
{
    va_list ap;
    va_start(ap, s);
    char *cstr;
    while ((cstr = va_arg(ap, char *)) != NULL)
    {
        unsigned int cstr_len = (unsigned int)strlen(cstr);
        if ((s->len + cstr_len + 1) >= s->mem_size)
        {
            if ((s->str = (char*) realloc(s->str, (s->len + cstr_len + 1) * sizeof(char))) == NULL)
            {
                return false;
            }
            s->mem_size = s->len + cstr_len + 1;
        }

        s->len += cstr_len;
        strcat(s->str, cstr);
        s->str[s->len] = '\0';
    }
    va_end(ap);
    return true;
}

bool str_add_str(string *s1, string *s2)
{
    if ((s1->len + s2->len + 1) >= s1->mem_size)
    {
        if ((s1->str = (char*) realloc(s1->str, (s1->len + s2->len + 1) * sizeof(char))) == NULL)
        {
            return false;
        }
        s1->mem_size = (s1->len + s2->len + 1) * sizeof(char);
    }

    s1->len += s2->len;
    strcat(s1->str, s2->str);
    s1->str[s1->len] = '\0';
    return true;
}

bool str_copy(string *src, string *dst)
{
    if ((src->len + 1) >= dst->mem_size)
    {
        if ((dst->str = (char*) realloc(dst->str, (src->len + 1) * sizeof(char))) == NULL)
        {
            return false;
        }
        dst->mem_size = src->len + 1;
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

void str_swap(string *s1, string *s2)
{
    string tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}
