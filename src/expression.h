/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Interface for parsing expression
 * 
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 */

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "parser.h"

typedef enum
{
	S_ADD = 0, S_SUB = 1,
	S_MUL = 2, S_DIV = 3,
	S_EQ = 4, S_NEQ = 5, S_GT = 6, S_LT = 7, S_GTE = 8, S_LTE = 9
} o_type;

typedef enum
{
	SYM_OPERATOR = 0, // o_type
	SYM_VAR = 1,
	SYM_INT = 2,
	SYM_STRING = 3,
	SYM_FLOAT64 = 4,
	SYM_STOP = 5, //$
	SYM_OPEN = 6, //(
	SYM_CLOSE = 7 //)
} symbol_type;

typedef struct 
{
	symbol_type sym_type;
	void *data; //pointer to: var_type or o_type or constant
} symbol_t;

/**
 * @brief Parse expression
 * 
 * @param data
 * @return int 
 */
int expression(data_t* data);

void free_symbol(void *ptr);

#endif
