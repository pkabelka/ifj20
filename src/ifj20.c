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
        fprintf(stderr, "%d", result);

    dispose_data(&data);
    str_free(&s);

    return result;
}
