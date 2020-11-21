/**
 * @brief Scanner implementation
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include <stdio.h>
#include <ctype.h>
#include "error.h"
#include "scanner.h"

/**
 * @brief Frees dynamic string and returns given exit code
 *
 * This function serves purpose to cut down on lines when exiting get_next_token
 *
 * @param str Pointer to a dynamic string
 * @param code Exit code
 * @return Given exit code
 */
static int cleanup(string *str, int code)
{
    str_free(str);
    return code;
}

/**
 * @brief Frees dynamic string, ungets the char c to stdin and returns given exit code
 *
 * This function serves purpose to cut down on lines when exiting get_next_token
 *
 * @param str Pointer to a dynamic string
 * @param code Exit code
 * @param c Character to ungetc
 * @return Given exit code
 */
static int cleanup_c(string *str, int code, char c)
{
    ungetc(c, stdin);
    str_free(str);
    return code;
}

/**
 * @brief Checks scanned identifier if it is actually a keyword or identifier
 * 
 * If the str is a keyword then the token kw attribute is set to the appropriate keyword
 * @param tok Pointer to a token
 * @param str Pointer to a dynamic string
 * @return SCANNER_SUCCESS for correct token, else appropriate error code
 */
static int keyword_or_identifier(token *tok, string *str)
{
    if (str_cmp_const(str, "int") == 0)            { tok->attr.kw = KW_INT; }
    else if (str_cmp_const(str, "float64") == 0)   { tok->attr.kw = KW_FLOAT64; }
    else if (str_cmp_const(str, "string") == 0)    { tok->attr.kw = KW_STRING; }
    else if (str_cmp_const(str, "nil") == 0)       { tok->attr.kw = KW_NIL; }
    else if (str_cmp_const(str, "_") == 0)         { tok->attr.kw = KW_UNDERSCORE; }
    else if (str_cmp_const(str, "if") == 0)        { tok->attr.kw = KW_IF; }
    else if (str_cmp_const(str, "else") == 0)      { tok->attr.kw = KW_ELSE; }
    else if (str_cmp_const(str, "for") == 0)       { tok->attr.kw = KW_FOR; }
    else if (str_cmp_const(str, "package") == 0)   { tok->attr.kw = KW_PACKAGE; }
    else if (str_cmp_const(str, "func") == 0)      { tok->attr.kw = KW_FUNC; }
    else if (str_cmp_const(str, "return") == 0)    { tok->attr.kw = KW_RETURN; }
    else if (str_cmp_const(str, "print") == 0)     { tok->attr.kw = KW_PRINT; }
    else if (str_cmp_const(str, "inputs") == 0)    { tok->attr.kw = KW_INPUTS; }
    else if (str_cmp_const(str, "inputi") == 0)    { tok->attr.kw = KW_INPUTI; }
    else if (str_cmp_const(str, "inputf") == 0)    { tok->attr.kw = KW_INPUTF; }
    else if (str_cmp_const(str, "int2float") == 0) { tok->attr.kw = KW_INT2FLOAT; }
    else if (str_cmp_const(str, "float2int") == 0) { tok->attr.kw = KW_FLOAT2INT; }
    else if (str_cmp_const(str, "len") == 0)       { tok->attr.kw = KW_LEN; }
    else if (str_cmp_const(str, "substr") == 0)    { tok->attr.kw = KW_SUBSTR; }
    else if (str_cmp_const(str, "ord") == 0)       { tok->attr.kw = KW_ORD; }
    else if (str_cmp_const(str, "chr") == 0)       { tok->attr.kw = KW_CHR; }
    else { tok->type = TOKEN_IDENTIFIER; }

    if (tok->type == TOKEN_KEYWORD) { return cleanup(str, SCANNER_SUCCESS); }

    if (!str_copy(str, tok->attr.str)) { return cleanup(str, ERR_INTERNAL); }
    return cleanup(str, SCANNER_SUCCESS);
}

/**
 * @brief Converts a string to a long integer and sets it as the token attribute
 * @param tok Pointer to a token
 * @param str Pointer to a dynamic string
 * @return SCANNER_SUCCESS for a valid token, else appropriate error code
 */
static int tok_attr_int(token *tok, string *str, int base)
{
    char *end;
    tok->attr.int_val = strtol(str->str, &end, base);
    if (*end != '\0') { return cleanup(str, ERR_LEX_STRUCTURE); }

    tok->type = TOKEN_INT;
    return cleanup(str, SCANNER_SUCCESS);
}

/**
 * @brief Converts a string to a double and sets it as the token attribute
 * @param tok Pointer to a token
 * @param str Pointer to a dynamic string
 * @return SCANNER_SUCCESS for a valid token, else appropriate error code
 */
static int token_attr_float64(token *tok, string *str)
{
    char *end;
    tok->attr.float64_val = strtod(str->str, &end);
    if (*end != '\0') { return cleanup(str, ERR_INTERNAL); }

    tok->type = TOKEN_FLOAT64;
    return cleanup(str, SCANNER_SUCCESS);
}

/**
 * Holds an initialized dynamic string for the token str attribute. The string
 * for all tokens is the same because they have one time use only.
 */
string *token_string_attr;

/**
 * @brief Sets token_string_attr to an initialized dynamic string
 * @param s Pointer to a dynamic string
 */
void set_token_string_attr(string *s)
{
    token_string_attr = s;
}

int get_next_token(token *tok)
{
    string str;
    if (!str_init(&str))
    {
        return ERR_INTERNAL;
    }

    /**
     * Keeps track of the token line number
     */
    static int line = 1;

    // set the token attribute str pointer to an initialized dynamic string
    tok->attr.str = token_string_attr;

    scanner_state state = SCANNER_START;
    tok->type = TOKEN_NONE;
    int c;
    int c_prev = 0;
    char hex_escape_str[2];
    unsigned int int_base = 10;
    tok->line = line;

    while(1)
    {
        c = getchar();
        switch (state)
        {
            case SCANNER_START:
                switch (c)
                {
                    case '\n':
                        state = SCANNER_EOL;
                        break;
                    // rest of isspace(c)
                    case ' ':
                    case '\t':
                    case '\v':
                    case '\f':
                    case '\r':
                        break;
                    case EOF:
                        tok->type = TOKEN_EOF;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case '/':
                        state = SCANNER_COMMENT_OR_DIV;
                        break;
                    case ':':
                        state = SCANNER_COLON;
                        break;
                    case '=':
                        state = SCANNER_EQUAL_OR_REASSIGN;
                        break;
                    case '!':
                        state = SCANNER_NOT_EQUAL;
                        break;
                    case '<':
                        state = SCANNER_LESS_THAN;
                        break;
                    case '>':
                        state = SCANNER_GREATER_THAN;
                        break;
                    case '+':
                        tok->type = TOKEN_ADD;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case '-':
                        tok->type = TOKEN_SUB;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case '*':
                        tok->type = TOKEN_MUL;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case '(':
                        tok->type = TOKEN_PAR_OPEN;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case ')':
                        tok->type = TOKEN_PAR_CLOSE;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case '{':
                        tok->type = TOKEN_CURLY_OPEN;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case '}':
                        tok->type = TOKEN_CURLY_CLOSE;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case ';':
                        tok->type = TOKEN_SEMICOLON;
                        return cleanup(&str, SCANNER_SUCCESS);
                    case ',':
                        tok->type = TOKEN_COMMA;
                        return cleanup(&str, SCANNER_SUCCESS);
                    // isalpha(c) || c == '_'
                    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                    case 'g':case 'h':case 'i':case 'j':case 'k':case 'l':
                    case 'm':case 'n':case 'o':case 'p':case 'q':case 'r':
                    case 's':case 't':case 'u':case 'v':case 'w':case 'x':
                    case 'y':case 'z':case 'A':case 'B':case 'C':case 'D':
                    case 'E':case 'F':case 'G':case 'H':case 'I':case 'J':
                    case 'K':case 'L':case 'M':case 'N':case 'O':case 'P':
                    case 'Q':case 'R':case 'S':case 'T':case 'U':case 'V':
                    case 'W':case 'X':case 'Y':case 'Z':case '_':
                        if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_KEYWORD_OR_IDENTIFIER;
                        break;
                    case '0':
                        c_prev = c;
                        state = SCANNER_INT_BASE;
                        break;
                    // rest of isdigit(c)
                    case '1':case '2':case '3':case '4':case '5':
                    case '6':case '7':case '8':case '9':
                        if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                        c_prev = c;
                        state = SCANNER_INT;
                        break;
                    case '"':
                        state = SCANNER_STRING;
                        break;
                    default:
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                break;

            case SCANNER_EOL:
                switch (c)
                {
                    case '\n':
                        line += 1;
                        break;
                    // rest of isspace(c)
                    case ' ':
                    case '\t':
                    case '\v':
                    case '\f':
                    case '\r':
                        break;
                    default:
                        tok->type = TOKEN_EOL;
                        line += 1;
                        return cleanup_c(&str, SCANNER_SUCCESS, c);
                }
                break;

            case SCANNER_COMMENT_OR_DIV:
                if (c == '/')
                {
                    state = SCANNER_COMMENT_LINE;
                }
                else if (c == '*')
                {
                    state = SCANNER_COMMENT_START;
                }
                else
                {
                    tok->type = TOKEN_DIV;
                    return cleanup_c(&str, SCANNER_SUCCESS, c);
                }
                break;

            case SCANNER_COMMENT_LINE:
                if (c == '\n' || c == EOF)
                {
                    state = SCANNER_START;
                    ungetc(c, stdin);
                }
                break;

            case SCANNER_COMMENT_START:
                if (c == '*')
                {
                    state = SCANNER_COMMENT_END;
                }
                else if (c == EOF)
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                break;

            case SCANNER_COMMENT_END:
                if (c == EOF)
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                else if (c == '/')
                {
                    state = SCANNER_START;
                }
                break;

            case SCANNER_COLON:
                if (c == '=')
                {
                    tok->type = TOKEN_ASSIGN;
                    return cleanup(&str, SCANNER_SUCCESS);
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                break;

            case SCANNER_EQUAL_OR_REASSIGN:
                if (c == '=')
                {
                    tok->type = TOKEN_EQUAL;
                    return cleanup(&str, SCANNER_SUCCESS);
                }
                else
                {
                    tok->type = TOKEN_REASSIGN;
                    return cleanup_c(&str, SCANNER_SUCCESS, c);
                }
                break;

            case SCANNER_NOT_EQUAL:
                if (c == '=')
                {
                    tok->type = TOKEN_NOT_EQUAL;
                    return cleanup(&str, SCANNER_SUCCESS);
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                break;

            case SCANNER_LESS_THAN:
                if (c == '=')
                {
                    tok->type = TOKEN_LESS_OR_EQUAL;
                    return cleanup(&str, SCANNER_SUCCESS);
                }
                else
                {
                    tok->type = TOKEN_LESS_THAN;
                    return cleanup_c(&str, SCANNER_SUCCESS, c);
                }
                break;

            case SCANNER_GREATER_THAN:
                if (c == '=')
                {
                    tok->type = TOKEN_GREATER_OR_EQUAL;
                    return cleanup(&str, SCANNER_SUCCESS);
                }
                else
                {
                    tok->type = TOKEN_GREATER_THAN;
                    return cleanup_c(&str, SCANNER_SUCCESS, c);
                }
                break;

            case SCANNER_KEYWORD_OR_IDENTIFIER:
                if (isalnum(c) || c == '_')
                {
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else
                {
                    ungetc(c, stdin);
                    // We "predict" the token is a keyword, if it is an identifier
                    // it will be set as TOKEN_IDENTIFIER in the
                    // keyword_or_identifier function
                    tok->type = TOKEN_KEYWORD;
                    return keyword_or_identifier(tok, &str);
                }
                break;

            case SCANNER_INT:
                if (isdigit(c))
                {
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '.')
                {
                    state = SCANNER_DECIMAL_POINT;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == 'e' || c == 'E')
                {
                    state = SCANNER_FLOAT64_EXPONENT;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '_')
                {
                    if (!isdigit(c_prev))
                    {
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                    }
                }
                else
                {
                    if (c_prev == '_')
                    {
                        return cleanup_c(&str, ERR_LEX_STRUCTURE, c);
                    }
                    ungetc(c, stdin);
                    return tok_attr_int(tok, &str, 10);
                }
                c_prev = c;
                break;

            case SCANNER_INT_BASE:
                if (c == 'b' || c == 'B')
                {
                    int_base = 2;
                    state = SCANNER_INT_BASE_NUM_FIRST;
                }
                else if (c == 'o' || c == 'O')
                {
                    int_base = 8;
                    state = SCANNER_INT_BASE_NUM_FIRST;
                }
                else if (c == 'x' || c == 'X')
                {
                    int_base = 16;
                    state = SCANNER_INT_BASE_NUM_FIRST;
                }
                else if (isdigit(c))
                {
                    int_base = 8;
                    state = SCANNER_INT_BASE_NUM_OTHER;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '.')
                {
                    state = SCANNER_DECIMAL_POINT;
                    if (!str_add(&str, '0')) { return cleanup(&str, ERR_INTERNAL); }
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '_')
                {
                    int_base = 8;
                    ungetc(c, stdin);
                    state = SCANNER_INT_BASE_NUM_FIRST;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = SCANNER_FLOAT64_EXPONENT;
                    if (!str_add(&str, '0')) { return cleanup(&str, ERR_INTERNAL); }
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else
                {
                    tok->type = TOKEN_INT;
                    tok->attr.int_val = 0;
                    return cleanup_c(&str, SCANNER_SUCCESS, c);
                }
                break;

            case SCANNER_INT_BASE_NUM_FIRST:
                if (isdigit(c) || (int_base == 16 && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))))
                {
                    state = SCANNER_INT_BASE_NUM_OTHER;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '_')
                {
                    if (!isdigit(c_prev))
                    {
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                    }
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                c_prev = c;
                break;

            case SCANNER_INT_BASE_NUM_OTHER:
                if (isdigit(c) || (int_base == 16 && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))))
                {
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '_')
                {
                    if (!isdigit(c_prev) && !(int_base == 16 && ((c_prev >= 'a' && c_prev <= 'f') || (c_prev >= 'A' && c_prev <= 'F'))))
                    {
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                    }
                }
                else
                {
                    if (c_prev == '_')
                    {
                        return cleanup_c(&str, ERR_LEX_STRUCTURE, c);
                    }
                    ungetc(c, stdin);
                    return tok_attr_int(tok, &str, int_base);
                }
                c_prev = c;
                break;

            case SCANNER_DECIMAL_POINT:
                if (isdigit(c))
                {
                    state = SCANNER_FLOAT64;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else
                {
                    return cleanup_c(&str, ERR_LEX_STRUCTURE, c);
                }
                break;

            case SCANNER_FLOAT64:
                if (isdigit(c))
                {
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == 'e' || c == 'E')
                {
                    state = SCANNER_FLOAT64_EXPONENT;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '_')
                {
                    if (!isdigit(c_prev))
                    {
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                    }
                }
                else
                {
                    if (c_prev == '_')
                    {
                        return cleanup_c(&str, ERR_LEX_STRUCTURE, c);
                    }
                    ungetc(c, stdin);
                    return token_attr_float64(tok, &str);
                }
                c_prev = c;
                break;

            case SCANNER_FLOAT64_EXPONENT:
                if (isdigit(c))
                {
                    state = SCANNER_FLOAT64_EXPONENT_NUMBER;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '+' || c == '-')
                {
                    state = SCANNER_FLOAT64_EXPONENT_SIGN;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                c_prev = c;
                break;

            case SCANNER_FLOAT64_EXPONENT_SIGN:
                if (isdigit(c))
                {
                    state = SCANNER_FLOAT64_EXPONENT_NUMBER;
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                c_prev = c;
                break;

            case SCANNER_FLOAT64_EXPONENT_NUMBER:
                if (isdigit(c))
                {
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else if (c == '_')
                {
                    if (!isdigit(c_prev))
                    {
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                    }
                }
                else
                {
                    if (c_prev == '_')
                    {
                        return cleanup_c(&str, ERR_LEX_STRUCTURE, c);
                    }
                    ungetc(c, stdin);
                    return token_attr_float64(tok, &str);
                }
                c_prev = c;
                break;

            case SCANNER_STRING:
                if (c < 32)
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                else if (c == '"')
                {
                    if (!str_copy(&str, tok->attr.str)) { return cleanup(&str, ERR_INTERNAL); }
                    tok->type = TOKEN_STRING;
                    return cleanup(&str, SCANNER_SUCCESS);
                }
                else if (c == '\\')
                {
                    state = SCANNER_STRING_ESCAPE;
                }
                else
                {
                    if (!str_add(&str, c)) { return cleanup(&str, ERR_INTERNAL); }
                }
                break;

            case SCANNER_STRING_ESCAPE:
                switch (c)
                {
                    case 'n':
                        if (!str_add(&str, '\n')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case 'r':
                        if (!str_add(&str, '\r')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case 't':
                        if (!str_add(&str, '\t')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case '\\':
                        if (!str_add(&str, '\\')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case '"':
                        if (!str_add(&str, '"')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case '\'':
                        if (!str_add(&str, '\'')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case 'x':
                        state = SCANNER_STRING_ESCAPE_HEX_FIRST;
                        break;
                    case 'v':
                        if (!str_add(&str, '\v')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case 'a':
                        if (!str_add(&str, '\a')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case 'b':
                        if (!str_add(&str, '\b')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    case 'f':
                        if (!str_add(&str, '\f')) { return cleanup(&str, ERR_INTERNAL); }
                        state = SCANNER_STRING;
                        break;
                    default:
                        return cleanup(&str, ERR_LEX_STRUCTURE);
                        break;
                }
                break;

            case SCANNER_STRING_ESCAPE_HEX_FIRST:
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                {
                    state = SCANNER_STRING_ESCAPE_HEX_SECOND;
                    hex_escape_str[0] = c;
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                break;

            case SCANNER_STRING_ESCAPE_HEX_SECOND:
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                {
                    state = SCANNER_STRING;
                    hex_escape_str[1] = c;
                    char *end;
                    int value = (int) strtol(hex_escape_str, &end, 16);
                    if (*end != '\0') { return cleanup(&str, ERR_INTERNAL); }
                    if (!str_add(&str, value)) { return cleanup(&str, ERR_INTERNAL); }
                }
                else
                {
                    return cleanup(&str, ERR_LEX_STRUCTURE);
                }
                break;
        }
    }
}
