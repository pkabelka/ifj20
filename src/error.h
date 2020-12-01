/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Error definitions
 *
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#ifndef _ERROR_H
#define _ERROR_H

#define ERR_LEX_STRUCTURE 1 // lexical analysis error (bad lex structure)
#define ERR_SYNTAX 2 // syntax error (bad syntax, unexpected newline, ...)
#define ERR_SEMANTIC_UNDEF_REDEF 3 // undefined function / variable, redefinition of function / variable, ...
#define ERR_SEMANTIC_VAR_TYPE 4 // error in detecting new variable data type
#define ERR_SEMANTIC_TYPE_COMPAT 5 // type compatibility error in arithmetic, string and relation expressions
#define ERR_SEMANTIC_FUNC_PARAMS 6 // bad (parameter / return) (count / type) in function call or return
#define ERR_SEMANTIC_OTHER 7 // other semantic errors
#define ERR_ZERO_DIVISION 9 // zero division error
#define ERR_INTERNAL 99 // internal compiler error (memory allocation, ...)

#endif
