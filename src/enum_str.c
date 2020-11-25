/**
 * @brief Enum to string conversion implementation
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "enum_str.h"

char *keyword_str(int e)
{
    switch (e)
    {
        case KW_INT:
            return "int";
        case KW_FLOAT64:
            return "float64";
        case KW_STRING:
            return "string";
        case KW_NIL:
            return "nil";
        case KW_UNDERSCORE:
            return "_";
        case KW_IF:
            return "if";
        case KW_ELSE:
            return "else";
        case KW_FOR:
            return "for";
        case KW_PACKAGE:
            return "package";
        case KW_FUNC:
            return "func";
        case KW_RETURN:
            return "return";
        case KW_PRINT:
            return "print";
        case KW_INPUTS:
            return "inputs";
        case KW_INPUTI:
            return "inputi";
        case KW_INPUTF:
            return "inputf";
        case KW_INT2FLOAT:
            return "int2float";
        case KW_FLOAT2INT:
            return "float2int";
        case KW_LEN:
            return "len";
        case KW_SUBSTR:
            return "substr";
        case KW_ORD:
            return "ord";
        case KW_CHR:
            return "chr";
    }
    return "";
}

char *token_str(int e)
{
    switch (e)
    {
        case TOKEN_NONE:
            return "NONE";
        case TOKEN_EOL:
            return "EOL";
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_ASSIGN:
            return ":=";
        case TOKEN_REASSIGN:
            return "=";
        case TOKEN_EQUAL:
            return "==";
        case TOKEN_NOT_EQUAL:
            return "!=";
        case TOKEN_LESS_THAN:
            return "<";
        case TOKEN_LESS_OR_EQUAL:
            return "<=";
        case TOKEN_GREATER_THAN:
            return ">";
        case TOKEN_GREATER_OR_EQUAL:
            return ">=";
        case TOKEN_ADD:
            return "+";
        case TOKEN_SUB:
            return "-";
        case TOKEN_DIV:
            return "/";
        case TOKEN_MUL:
            return "*";
        case TOKEN_PAR_OPEN:
            return "(";
        case TOKEN_PAR_CLOSE:
            return ")";
        case TOKEN_CURLY_OPEN:
            return "{";
        case TOKEN_CURLY_CLOSE:
            return "}";
        case TOKEN_SEMICOLON:
            return ";";
        case TOKEN_COMMA:
            return ",";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_INT:
            return "int";
        case TOKEN_FLOAT64:
            return "float64";
        case TOKEN_STRING:
            return "string";
    }
    return "";
}
