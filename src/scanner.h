/**
 * @brief Scanner interface
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#ifndef _SCANNER_H
#define _SCANNER_H

#include "str.h"

#define SCANNER_SUCCESS 0

/**
 * @enum Scanner state
 */
typedef enum
{
    // literal symbols in definitions are quoted with '
    SCANNER_START, // initial state
    SCANNER_EOL, // end of line / newline
    SCANNER_COMMENT_OR_DIV, // '/' - division or start of a comment
    SCANNER_COMMENT_LINE, // '//' - line comment, everything is ignored until EOL
    SCANNER_COMMENT_START, // '/*' - start of a block comment
    SCANNER_COMMENT_END, // '*/' - end of a block comment
    SCANNER_COLON, // ':' - start of assignment, expects '=' as next character
    SCANNER_EQUAL_OR_REASSIGN, // '=' - reassignment or equality operator ('==')
    SCANNER_NOT_EQUAL, // '!' - start of not equal, expects '=' as next character
    SCANNER_LESS_THAN, // '<' - "less than" or "less or equal"
    SCANNER_GREATER_THAN, // '>' - "greater than" or "greater or equal"
    SCANNER_KEYWORD_OR_IDENTIFIER, // scanned characters could lead to a reserved keyword or an identifier
    SCANNER_INT, // number could be integer or float64
    SCANNER_INT_BASE, // integer base expected
    SCANNER_INT_BASE_NUM_FIRST, // fist integer expected
    SCANNER_INT_BASE_NUM_OTHER, // processes other digits
    SCANNER_DECIMAL_POINT, // '.' - decimal point, expects another number and determines the number is a float64
    SCANNER_FLOAT64, // a number was entered after SCANNER_DECIMAL_POINT
    SCANNER_FLOAT64_EXPONENT, // 'e' or 'E' - expects an optional sign or a number
    SCANNER_FLOAT64_EXPONENT_SIGN, // a sign was found, expects only numbers next
    SCANNER_FLOAT64_EXPONENT_NUMBER, // expects only numbers next
    SCANNER_STRING, // '"' - start of a string
    SCANNER_STRING_ESCAPE, // '\' - escape symbol
    SCANNER_STRING_ESCAPE_HEX_FIRST, // '\x' - hex escape sequence, expects h where h is a hexadecimal digit
    SCANNER_STRING_ESCAPE_HEX_SECOND, // '\xh' - hex escape sequence, expects a second h where h is a hexadecimal digit
} scanner_state;

/**
 * @enum Token type
 */
typedef enum
{
    TOKEN_NONE, // empty token
    TOKEN_EOL, // EOL - end of line
    TOKEN_EOF, // EOF - end of file
    TOKEN_ASSIGN, // ':=' - assigning to a new variable
    TOKEN_REASSIGN, // '=' - assigning to an existing variable
    TOKEN_EQUAL, // '==' - equality operator
    TOKEN_NOT_EQUAL, // '!=' - not equal
    TOKEN_LESS_THAN, // '<' - less than
    TOKEN_LESS_OR_EQUAL, // '<=' - less or equal
    TOKEN_GREATER_THAN, // '>' - greater than
    TOKEN_GREATER_OR_EQUAL, // '>=' - greater or equal
    TOKEN_ADD, // '+' - addition
    TOKEN_SUB, // '-' - subtraction
    TOKEN_DIV, // '/' - division
    TOKEN_MUL, // '*' - multiplication
    TOKEN_PAR_OPEN, // '(' - opening parenthesis
    TOKEN_PAR_CLOSE, // ')' - closing parenthesis
    TOKEN_CURLY_OPEN, // '{' - opening curly brace
    TOKEN_CURLY_CLOSE, // '}' - closing curly brace
    TOKEN_SEMICOLON, // ';' - semicolon
    TOKEN_COMMA, // ',' - comma
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_INT, // 64 bit number - long int
    TOKEN_FLOAT64, // 64 bit decimal number - double
    TOKEN_STRING,
} token_type;

/**
 * @enum Keyword
 */
typedef enum
{
    KW_INT,
    KW_FLOAT64,
    KW_STRING,
    KW_NIL,
    KW_UNDERSCORE,
    KW_IF,
    KW_ELSE,
    KW_FOR,
    KW_PACKAGE,
    KW_FUNC,
    KW_RETURN,
    KW_PRINT,
    KW_INPUTS,
    KW_INPUTI,
    KW_INPUTF,
    KW_INT2FLOAT,
    KW_FLOAT2INT,
    KW_LEN,
    KW_SUBSTR,
    KW_ORD,
    KW_CHR,
} keyword;

/**
 * @union Token attribute
 * 
 * Union type lets us use only one member at a time, creating a "generic" data type
 */
typedef union
{
    long int int_val;
    double float64_val;
    string *str;
    keyword kw;
} token_attr;

/**
 * @struct Token
 */
typedef struct
{
    token_type type;
    token_attr attr;
    int line;
} token;

/**
 * @brief Sets the token attribute *str to a preallocated dynamic string pointer
 * @param s Pointer to a preallocated string
 */
void set_token_string_attr(string *s);

/**
 * @brief Scans input for a valid token, processes it and returns an appropriate exit code
 *
 * A dynamic string must be initialized and set with set_token_string_attr
 * first before using get_next_token
 *
 * @param tok Pointer to a token
 * @return SCANNER_SUCCESS for a valid token, else appropriate error code
 */
int get_next_token(token *tok);

#endif
