// #include <stdio.h>
// #include "str.h"
// #include "scanner.h"
// #include "error.h"

// int main()
// {
//     scanner_set_input(stdin);
//     int result;
//     token_type type;
//     do
//     {
//         string s;
//         str_init(&s);
//         set_token_string_attr(&s);
//         token *tok = (token*) malloc(sizeof(token));
//         result = get_next_token(tok);
//         type = tok->type;
//         printf("--------------------\n");
//         printf("Status: %d\n", result);
//         printf("Token type: %d\n", type);

//         if (tok->type == TOKEN_INT)
//             printf("Token long: %ld\n", tok->attr.int_val);
//         if (tok->type == TOKEN_FLOAT64)
//             printf("Token float64: %f\n", tok->attr.float64_val);
//         if (tok->type == TOKEN_IDENTIFIER || tok->type == TOKEN_STRING)
//             printf("Token str: %s\n", tok->attr.str->str);
//         if (tok->type == TOKEN_KEYWORD)
//             printf("Token kw: %d\n", tok->attr.kw);

//         str_free(&s);
//         free(tok);
//     } while (type != TOKEN_EOF);
//     return 0;
// }
