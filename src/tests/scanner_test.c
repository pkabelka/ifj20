#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "str.h"
#include "scanner.h"
#include "error.h"
#include "symtable.h"
#include "stack.h"

void print_tree(stnode_ptr TempTree, char* sufix, char fromdir)
{
    if (TempTree != NULL)
    {
        char* suf2 = (char*) malloc(strlen(sufix) + 4);
        strcpy(suf2, sufix);
        if (fromdir == 'L')
        {
            suf2 = strcat(suf2, "  |");
            printf("%s\n", suf2);
        }
        else
        {
            suf2 = strcat(suf2, "   ");
        }
        print_tree(TempTree->rnode, suf2, 'R');
        printf("%s  +-[%s]\n", sufix, TempTree->key);
        strcpy(suf2, sufix);
        if (fromdir == 'R')
        {
            suf2 = strcat(suf2, "  |");
        }
        else
        {
            suf2 = strcat(suf2, "   ");
        }
        print_tree(TempTree->lnode, suf2, 'L');
        if (fromdir == 'R')
        {
            printf("%s\n", suf2);
        }
        free(suf2);
    }
}

#define NEXT_TOKEN() \
    result = get_next_token(tok); \
    printf("--------------------\n"); \
    printf("Result: %d\n", result); \
    printf("Token type: %d\n", tok->type);

#define PRINT_VALS() if (tok->type == TOKEN_INT) \
            printf("Token long: %ld\n", tok->attr.int_val); \
        if (tok->type == TOKEN_FLOAT64) \
            printf("Token float64: %f\n", tok->attr.float64_val); \
        if (tok->type == TOKEN_IDENTIFIER || tok->type == TOKEN_STRING) \
            printf("Token str: %s\n", tok->attr.str->str); \
        if (tok->type == TOKEN_KEYWORD) \
            printf("Token kw: %d\n", tok->attr.kw);

#define EOL() NEXT_TOKEN() \
        PRINT_VALS() \
        assert(result == SCANNER_SUCCESS); \
        assert(tok->type == TOKEN_EOL);

#define TOK(TYPE) NEXT_TOKEN() \
        PRINT_VALS() \
        assert(result == SCANNER_SUCCESS); \
        assert(tok->type == TYPE);

#define KW(KEY) NEXT_TOKEN() \
        PRINT_VALS() \
        assert(result == SCANNER_SUCCESS); \
        assert(tok->type == TOKEN_KEYWORD); \
        assert(tok->attr.kw == KEY);

#define ID(STR) NEXT_TOKEN() \
        PRINT_VALS() \
        assert(result == SCANNER_SUCCESS); \
        assert(tok->type == TOKEN_IDENTIFIER); \
        assert(str_cmp_const(tok->attr.str, STR) == 0);

#define STR(STRV) NEXT_TOKEN() \
        PRINT_VALS() \
        assert(result == SCANNER_SUCCESS); \
        assert(tok->type == TOKEN_STRING); \
        assert(str_cmp_const(tok->attr.str, STRV) == 0);

#define INT(VAL) NEXT_TOKEN() \
        PRINT_VALS() \
        assert(result == SCANNER_SUCCESS); \
        assert(tok->type == TOKEN_INT); \
        assert(tok->attr.int_val == VAL);

int main(int argc, char *argv[])
{
    int result;
    if (argc <= 1)
    {
        string s;
        str_init(&s);
        set_token_string_attr(&s);
        token tok;
        stnode_ptr tree;
        symtable_init(&tree);
        do
        {
            bool err;
            result = get_next_token(&tok);
            printf("--------------------\n");
            printf("Result: %d\n", result);
            printf("Token type: %d\n", tok.type);
            if (tok.type == TOKEN_INT)
                printf("Token long: %ld\n", tok.attr.int_val);
            if (tok.type == TOKEN_FLOAT64)
                printf("Token float64: %f\n", tok.attr.float64_val);
            if (tok.type == TOKEN_IDENTIFIER)
            {
                printf("Token str: %s\n", tok.attr.str->str);
                stnode_ptr new = symtable_insert(&tree, tok.attr.str->str, &err);
                if (new != NULL){
                    new->data = malloc(sizeof(struct stdata));
                }
            }
            if (tok.type == TOKEN_KEYWORD)
                printf("Token kw: %d\n", tok.attr.kw);
        } while (tok.type != TOKEN_EOF);
        print_tree(tree, "", 'X');

        stnode_ptr del = symtable_search(tree, "main");
        if (del != NULL)
        {
            symtable_delete_node(&tree, "main", free);
        }
        symtable_dispose(&tree, free);
        str_free(&s);

        stack st;
        stack_init(&st);
        int *a = malloc(sizeof(int));
        stack_push(&st, a);
        stack_dispose(&st, free);
    }
    else if (strcmp("num", argv[1]) == 0)
    {
        string s;
        str_init(&s);
        set_token_string_attr(&s);
        token *tok = (token*) malloc(sizeof(token));

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 123L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 37L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 37L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 37L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 37L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 37L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 171L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 171L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 171L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 5L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 5L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 5L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 5L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 5L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 5L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_INT);
        assert(tok->attr.int_val == 222L);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 0.0);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 0.0);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);
        
        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 0.4);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 0.345);
        
        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);
        
        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 314.1592);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 0.03141592);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 3141.592);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 1000000.101);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 3334544.44);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 1.0);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_FLOAT64);
        assert(tok->attr.float64_val == 0.011);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_IDENTIFIER);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_ASSIGN);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == ERR_LEX_STRUCTURE);
        assert(tok->type == TOKEN_NONE);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_IDENTIFIER);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_ASSIGN);

        NEXT_TOKEN()

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOL);

        NEXT_TOKEN()
        PRINT_VALS()
        assert(result == SCANNER_SUCCESS);
        assert(tok->type == TOKEN_EOF);

        str_free(&s);
        free(tok);
    }
    else if (strcmp("factorial", argv[1]) == 0)
    {
        string s;
        str_init(&s);
        set_token_string_attr(&s);
        token *tok = (token*) malloc(sizeof(token));

        EOL()
        KW(KW_PACKAGE)
        ID("main")
        EOL()
        KW(KW_FUNC)
        ID("factorial")
        TOK(TOKEN_PAR_OPEN)
        ID("n")
        KW(KW_INT)
        TOK(TOKEN_PAR_CLOSE)
        KW(KW_INT)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        ID("dec_n")
        TOK(TOKEN_ASSIGN)
        ID("n")
        TOK(TOKEN_SUB)
        INT(1)
        EOL()
        KW(KW_IF)
        ID("n")
        TOK(TOKEN_LESS_THAN)
        INT(2)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        KW(KW_RETURN)
        INT(1)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        KW(KW_ELSE)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        ID("tmp")
        TOK(TOKEN_ASSIGN)
        INT(0)
        EOL()
        ID("tmp")
        TOK(TOKEN_REASSIGN)
        ID("factorial")
        TOK(TOKEN_PAR_OPEN)
        ID("dec_n")
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        KW(KW_RETURN)
        ID("n")
        TOK(TOKEN_MUL)
        ID("tmp")
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        EOL()
        KW(KW_FUNC)
        ID("main")
        TOK(TOKEN_PAR_OPEN)
        TOK(TOKEN_PAR_CLOSE)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        KW(KW_PRINT)
        TOK(TOKEN_PAR_OPEN)
        STR("Zadejte cislo pro vypocet faktorialu: ")
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        ID("a")
        TOK(TOKEN_ASSIGN)
        INT(0)
        EOL()
        ID("err")
        TOK(TOKEN_ASSIGN)
        INT(0)
        EOL()
        ID("a")
        TOK(TOKEN_COMMA)
        ID("err")
        TOK(TOKEN_REASSIGN)
        KW(KW_INPUTI)
        TOK(TOKEN_PAR_OPEN)
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        KW(KW_IF)
        ID("err")
        TOK(TOKEN_EQUAL)
        INT(0)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        KW(KW_IF)
        ID("a")
        TOK(TOKEN_LESS_THAN)
        INT(0)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        KW(KW_PRINT)
        TOK(TOKEN_PAR_OPEN)
        STR("Faktorial nejde spocitat!")
        TOK(TOKEN_COMMA)
        STR("\n")
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        KW(KW_ELSE)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        ID("vysl")
        TOK(TOKEN_ASSIGN)
        INT(0)
        EOL()
        ID("vysl")
        TOK(TOKEN_REASSIGN)
        ID("factorial")
        TOK(TOKEN_PAR_OPEN)
        ID("a")
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        KW(KW_PRINT)
        TOK(TOKEN_PAR_OPEN)
        STR("Vysledek je ")
        TOK(TOKEN_COMMA)
        ID("vysl")
        TOK(TOKEN_COMMA)
        STR("\n")
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        KW(KW_ELSE)
        TOK(TOKEN_CURLY_OPEN)
        EOL()
        KW(KW_PRINT)
        TOK(TOKEN_PAR_OPEN)
        STR("Chyba pri nacitani celeho cisla!\n")
        TOK(TOKEN_PAR_CLOSE)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        EOL()
        TOK(TOKEN_CURLY_CLOSE)
        EOL()
        TOK(TOKEN_EOF)

        str_free(&s);
        free(tok);
    }
    else if (strcmp("escape", argv[1]) == 0)
    {
        string s;
        str_init(&s);
        set_token_string_attr(&s);
        token *tok = (token*) malloc(sizeof(token));

        ID("a")
        TOK(TOKEN_REASSIGN)
        STR("\a\tAhoj\n\"Sve'te\x45\x53 \\\x22")
        EOL()
        TOK(TOKEN_EOF)

        str_free(&s);
        free(tok);
    }
}
