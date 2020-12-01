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
	S_ADD, S_SUB,
	S_MUL, S_DIV,
	S_EQ, S_NEQ, S_GT, S_LT, S_GTE, S_LTE
} o_type;

typedef enum
{
	SYM_OPERATOR, // o_type
	SYM_VAR, 
	SYM_INT, 
	SYM_STRING, 
	SYM_FLOAT64,
	SYM_NONE
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
