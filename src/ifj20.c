#include <stdio.h>
#include "str.h"
#include "scanner.h"
#include "error.h"
#include "parser.h"
#include "enum_str.h"
#include "codegen.h"

int main()
{
    string s;
    GEN(str_init, &s);
    set_token_string_attr(&s);

    data_t data;
    GEN(init_data, &data);
    GEN(gen_codegen_init);
    GEN(str_init, &for_assigns);
    GEN(str_init, &func_declarations);
    GEN(str_init, &func_body);

    int result = parse(&data);
    if (result != 0)
    {
        if (result == ERR_SYNTAX)
        {
            if (data.token.type == TOKEN_KEYWORD)
                fprintf(stderr, "syntax error: unexpected token keyword %s at line %d\n", keyword_str(data.token.attr.kw), data.token.line);
            else if (data.token.type == TOKEN_IDENTIFIER)
                fprintf(stderr, "syntax error: unexpected identifier '%s' at line %d\n", data.token.attr.str->str, data.token.line);
            else
                fprintf(stderr, "syntax error: unexpected token '%s' at line %d\n", token_str(data.token.type), data.token.line);
            fprintf(stderr, "token sequence: %s %s\n", token_str(data.prev_token.type), token_str(data.token.type));
        }
        else
            fprintf(stderr, "error %d - line: %d\n", result, data.token.line);
    }
    else
    {
        GEN(gen_codegen_output);
    }
    
    dispose_data(&data);
    str_free(&s);
    str_free(&ifjcode20_output);
    str_free(&for_assigns);
    str_free(&func_declarations);
    str_free(&func_body);

    return result;
}
