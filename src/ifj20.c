#include <stdio.h>
#include "str.h"
#include "scanner.h"
#include "error.h"
#include "parser.h"

int main()
{
    string s;
    str_init(&s);
    set_token_string_attr(&s);

    data data;
    init_data(&data);

    int result = parse(&data);
    if (result != 0)
    {
        if (result == ERR_SYNTAX)
        {
            if (data.token.type == TOKEN_KEYWORD)
                fprintf(stderr, "syntax error: unexpected token keyword %d at line %d\n", data.token.attr.kw, data.token.line);
            else
                fprintf(stderr, "syntax error: unexpected token %d at line %d\n", data.token.type, data.token.line);
            fprintf(stderr, "token sequence: %d %d\n", data.prev_token.type, data.token.type);
        }
        else
            fprintf(stderr, "error %d - line: %d\n", result, data.token.line);
    }
    dispose_data(&data);
    str_free(&s);

    return result;
}
