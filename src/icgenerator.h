#ifndef _ICGENERATOR_H
#define _ICGENERATOR_H

#include "expression.h"

typedef enum
{
	S_ADD = 0, 
	S_SUB = 1,  
	S_MUL = 2,
	S_DIV = 3
} o_type;

typedef enum
{
	SYM_OPERATOR, //+-* /
	SYM_VAR, 
	SYM_INT, 
	SYM_STRING, 
	SYM_FLOAT64
} symbol_type;

typedef struct 
{
	symbol_type sym_type;
	void *data; //pointer to: var_type or o_type or constant
} symbol_t;

bool generate_internal_code(dll_t*);

#endif
