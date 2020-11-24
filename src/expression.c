/**
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 * @brief Implementation of parsing expressions
 */

#include "expression.h"
#include "dll.h"
#include "error.h"
#include "scanner.h"
#include "stack.h"
#include "str.h"

#define TKN data->token
#define CHECK_RESULT() if (data->result != 0) return data->result;
#define NEXT_TOKEN() data->prev_token = data->token; if (get_next_token(&data->token) != SCANNER_SUCCESS) return ERR_LEX_STRUCTURE;
#define APPLY_RULE(func) data->result = func(data, list, sym_stack); CHECK_RESULT()
#define APPLY_NEXT_RULE(func) NEXT_TOKEN() APPLY_RULE(func)
#define EXPECT_TOKEN(token) if (TKN.type != token) return ERR_SYNTAX;
#define EXPECT_NEXT_TOKEN(token) NEXT_TOKEN() if (TKN.type != token) return ERR_SYNTAX;

typedef enum
{
	S_ADD = 0, 
	S_SUB = 1,  
	S_MUL = 2,
	S_DIV = 3
} o_type;

typedef enum
{
	SYM_OPERATOR, //+-*/
	SYM_VAR, 
	SYM_INT, 
	SYM_STRING, 
	SYM_FLOAT
} symbol_type;

typedef struct 
{
	symbol_type sym_type;
	void *data; //pointer to: var_type or o_type or constant
} symbol_t;

//precedence table
const int precedence[4][4] = {
	// +   -  *  /
	{  0,  0, 1, 1 }, // +
	{  0,  0, 1, 1 }, // -
	{ -1, -1, 0, 0 }, // *
	{ -1, -1, 0, 0 }  // /
};

static int end_of_expression(data_t *data, dll_t *list, stack *sym_stack);
static int bracket_scope(data_t *data, dll_t *list, stack *sym_stack);
static int push_const(data_t *data, token token, dll_t *list);
static int push_id(data_t *data, token token, dll_t *list);
static int push_o(token token, dll_t *list, stack *sym_stack);
static int start_of_expression(data_t *data, dll_t *list, stack *sym_stack);

void free_symbol(void *ptr)
{
	symbol_t *sym = (symbol_t*)ptr;
	if (sym->sym_type == SYM_VAR)
	{ 
		//data will be cleaer from sym tabel
	}
	else if (sym->sym_type == SYM_OPERATOR)
		free(sym->data);
	else if (sym->sym_type == SYM_STRING)
	{
		string *s = sym->data;
		str_free(s);
		free(s);
	}
	else
		free(sym->data);
	free(sym);
}

int expression(data_t *data)
{
	dll_t *list = dll_init();
	if (list == NULL)
		return ERR_INTERNAL;

	stack sym_stack;
	stack_init(&sym_stack);

	int r = start_of_expression(data, list, &sym_stack);
	if (r == 0 && TKN.type != TOKEN_PAR_OPEN)
	{
		//pushing rest of operators
		struct stack_el *elem = sym_stack.top;
		while (elem != NULL)
		{
			symbol_t *sym = malloc(sizeof(symbol_t));
			sym->sym_type = SYM_OPERATOR;
			sym->data = sym_stack.top->data;
			if (!dll_insert_last(list, sym))
				return ERR_INTERNAL;

			stack_pop(&sym_stack, stack_nofree);
		}

		//TODO: generate internal code and optimize
		data->vdata->type = data->current_type;
		//TODO: generate code
	}

	stack_dispose(&sym_stack, free);
	dll_dispose(list, free_symbol);
	return r;
}

static int start_of_expression(data_t *data, dll_t *list, stack *sym_stack)
{
	if (TKN.type == TOKEN_IDENTIFIER) //func or var
	{
		APPLY_NEXT_RULE(end_of_expression)
		return 0;
	}
	else if (TKN.type == TOKEN_INT || TKN.type == TOKEN_STRING || TKN.type == TOKEN_FLOAT64) //constants
	{
		data->result = push_const(data, TKN, list);
		CHECK_RESULT()

		data->allow_func = false;
		APPLY_NEXT_RULE(end_of_expression)
		return 0;
	}
	else if (TKN.type == TOKEN_PAR_OPEN)
	{
		data->allow_func = false;
		dll_t *sub_list = dll_init(); //expression inside of brackets
		if (sub_list == NULL)
			return ERR_INTERNAL;

		data->result = bracket_scope(data, sub_list, sym_stack);
		CHECK_RESULT()

		dll_join_lists(list, sub_list);
		
		APPLY_NEXT_RULE(end_of_expression)
		return 0;
	}
	else if (data->allow_func && is_inter_func(TKN))
	{
		EXPECT_NEXT_TOKEN(TOKEN_PAR_OPEN)
		return 0;
	}

	return ERR_SYNTAX;
}

static int end_of_expression(data_t *data, dll_t *list, stack *sym_stack)
{
	switch (TKN.type)
	{
		//arithmetic operators
		case TOKEN_ADD: case TOKEN_SUB:  case TOKEN_MUL:  case TOKEN_DIV:
			if (data->current_type == 's' && TKN.type != TOKEN_ADD) //unallowed operations with string
				return ERR_SEMANTIC_TYPE_COMPAT;

			if (data->prev_token.type == TOKEN_IDENTIFIER)
			{
				data->result = push_id(data, data->prev_token, list);
				CHECK_RESULT()
			}

			data->result = push_o(TKN, list, sym_stack);
			CHECK_RESULT()

			data->allow_func = false;
			APPLY_NEXT_RULE(start_of_expression)
			return 0;

		//end of expression
		case TOKEN_PAR_CLOSE: case TOKEN_COMMA: case TOKEN_EOL: case TOKEN_CURLY_OPEN: case TOKEN_SEMICOLON:
			return 0;

		//func
		case TOKEN_PAR_OPEN:
			if (data->allow_func)
				return 0;
			return ERR_SYNTAX;

		default:
			return ERR_SYNTAX;
	}
	return ERR_SYNTAX;
}

static int push_id(data_t *data, token token, dll_t *list)
{
	var_data_t *var = find_var(data, token.attr.str->str, false);
	if (var == NULL)
		return ERR_SEMANTIC_UNDEF_REDEF;

	symbol_t *sym = malloc(sizeof(symbol_t));
	sym->sym_type = SYM_VAR;
	sym->data = var;

	data->current_type = compare_types(var->type, data->current_type);
	if (data->current_type == '0')
		return ERR_SEMANTIC_TYPE_COMPAT;

	if (!dll_insert_last(list, sym))
		return ERR_INTERNAL;
	return 0;
}

static int push_const(data_t *data, token token, dll_t *list)
{
	symbol_t *sym = malloc(sizeof(symbol_t));
	if (sym == NULL)
		return ERR_INTERNAL;

	if (token.type == TOKEN_INT) //int
	{
		sym->sym_type = SYM_INT;
		sym->data = malloc(sizeof(int));
		if (sym->data == NULL)
			return ERR_INTERNAL;

		*(int*)sym->data = token.attr.int_val;
		data->current_type = compare_types('i', data->current_type);
	}
	else if (token.type == TOKEN_FLOAT64) //float
	{
		sym->sym_type = SYM_FLOAT;
		sym->data = malloc(sizeof(float));
		if (sym->data == NULL)
			return ERR_INTERNAL;

		*(float*)sym->data = token.attr.float64_val;
		data->current_type = compare_types('f', data->current_type);
	}
	else //string
	{
		sym->sym_type = SYM_STRING;
		string *str = malloc(sizeof(string));
		if (str == NULL)
			return ERR_INTERNAL;

		if (!str_init(str))
			return ERR_INTERNAL;

		if (!str_add_const(str, token.attr.str->str))
			return ERR_INTERNAL;

		sym->data = str;
		data->current_type = compare_types('s', data->current_type);
	}

	if (data->current_type == '0')
		return ERR_SEMANTIC_TYPE_COMPAT;

	if (!dll_insert_last(list, sym))
		return ERR_INTERNAL;
	return 0;
}

static int bracket_scope(data_t *data, dll_t *list, stack *sym_stack) //preference of () using recursion
{
	APPLY_NEXT_RULE(start_of_expression)
	if (TKN.type == TOKEN_PAR_CLOSE)
	{
		if (data->prev_token.type == TOKEN_IDENTIFIER)
		{
			data->result = push_id(data, data->prev_token, list);
			CHECK_RESULT()
		}

		//pushing rest of operators
		struct stack_el *elem = sym_stack->top;
		while (elem != NULL)
		{
			symbol_t *sym = malloc(sizeof(symbol_t));
			sym->sym_type = SYM_OPERATOR;
			sym->data = sym_stack->top->data;
			dll_insert_last(list, sym);
			stack_pop(sym_stack, stack_nofree);
		}
		return 0;
	}
	return ERR_SYNTAX;
}

static int push_o(token token, dll_t *list, stack *sym_stack)
{
	o_type *ot = malloc(sizeof(o_type)); //currently loaded operator
	if (ot == NULL)
		return ERR_INTERNAL;

	if (token.type == TOKEN_ADD)
		*ot = S_ADD;
	else if (token.type == TOKEN_SUB)
	 	*ot = S_SUB;
	else if (token.type == TOKEN_MUL)
		*ot = S_MUL;
	else
		*ot = S_DIV;

	//pushing all operators with greater priority than current
	struct stack_el *elem = sym_stack->top;
	while (elem != NULL && precedence[*ot][*(o_type*)elem->data] == -1)
	{
		symbol_t *sym = malloc(sizeof(symbol_t));
		if (sym == NULL)
			return ERR_INTERNAL;

		sym->sym_type = SYM_OPERATOR;
		sym->data = sym_stack->top->data;
		if (!dll_insert_last(list, sym))
			return ERR_INTERNAL;
			
		stack_pop(sym_stack, stack_nofree);
	}

	if (!stack_push(sym_stack, ot))
		return ERR_INTERNAL;
	return 0;
}