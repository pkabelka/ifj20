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
#include "codegen.h"

#define TKN data->token
#define CHECK_RESULT() if (data->result != 0) return data->result;
#define NEXT_TOKEN() data->prev_token = data->token; if (get_next_token(&data->token) != SCANNER_SUCCESS) return ERR_LEX_STRUCTURE;
#define APPLY_RULE(func) data->result = func(data, list, sym_stack); CHECK_RESULT()
#define APPLY_NEXT_RULE(func) NEXT_TOKEN() APPLY_RULE(func)
#define EXPECT_TOKEN(token) if (TKN.type != token) return ERR_SYNTAX;
#define EXPECT_NEXT_TOKEN(token) NEXT_TOKEN() if (TKN.type != token) return ERR_SYNTAX;

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

//precedence table
const int precedence[3][3] = {
	//+-  */   r
	{  0,  1,  1 }, // +-
	{ -1,  0,  1 }, // */
	{ -1, -1,  0 }  // relational operators
};

static int end_of_expression(data_t *data, dll_t *list, stack *sym_stack);
static int bracket_scope(data_t *data, dll_t *list, stack *sym_stack);
static int push_const(data_t *data, token token, dll_t *list);
static int push_id(data_t *data, token token, dll_t *list);
static int push_o(token token, dll_t *list, stack *sym_stack);
static int start_of_expression(data_t *data, dll_t *list, stack *sym_stack);
static int generate_expression(data_t *data, dll_t *list);
static o_type token_to_type(token_type type);
static bool push_rest(stack *sym_stack, dll_t *list);
static int gpi(o_type type);

void free_symbol(void *ptr)
{
	symbol_t *sym = (symbol_t*)ptr;
	if (sym->sym_type == SYM_VAR)
	{ 
		//data will be cleared from sym tabel
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
	free(ptr);
}

int expression(data_t *data)
{
	dll_t *list = dll_init();
	if (list == NULL)
		return ERR_INTERNAL;

	stack sym_stack;
	stack_init(&sym_stack);

	if (data->vdata == NULL)
		data->current_type = 't';
	else
		data->current_type = data->vdata->type;

	data->used_relations = false;
	int r = start_of_expression(data, list, &sym_stack);
	if (r == 0 && TKN.type != TOKEN_PAR_OPEN)
	{
		if (data->used_relations && !data->allow_relations)
			r = ERR_SEMANTIC_TYPE_COMPAT;
		else if (!data->used_relations && data->allow_relations)
			r = ERR_SEMANTIC_TYPE_COMPAT;
		else
		{
			//TODO: generate internal code and optimize
			if (data->vdata != NULL)
				data->vdata->type = data->current_type;

			r = generate_expression(data, list);
		}
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

		stack sub_stack;
		stack_init(&sub_stack);

		data->result = bracket_scope(data, sub_list, &sub_stack);
		if (data->result != 0)
		{
			dll_dispose(sub_list, free_symbol);
			return data->result;
		}

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
	if (TKN.type != TOKEN_PAR_OPEN && data->prev_token.type == TOKEN_IDENTIFIER)
	{
		data->result = push_id(data, data->prev_token, list);
		CHECK_RESULT()
	}

	switch (TKN.type)
	{
		//arithmetic operators
		case TOKEN_ADD: case TOKEN_SUB:  case TOKEN_MUL:  case TOKEN_DIV:
			if (data->current_type == 's' && TKN.type != TOKEN_ADD) //unallowed operations with string
				return ERR_SEMANTIC_TYPE_COMPAT;

			data->result = push_o(TKN, list, sym_stack);
			CHECK_RESULT()

			data->allow_func = false;
			APPLY_NEXT_RULE(start_of_expression)
			return 0;

		//relational operators
		case TOKEN_EQUAL: case TOKEN_NOT_EQUAL: 
		case TOKEN_LESS_OR_EQUAL: case TOKEN_LESS_THAN:
		case TOKEN_GREATER_OR_EQUAL: case TOKEN_GREATER_THAN:
			if (data->used_relations) //relational operators was used more that once
				return ERR_SEMANTIC_TYPE_COMPAT;
			data->used_relations = true;

			data->result = push_o(TKN, list, sym_stack);
			CHECK_RESULT()

			data->allow_func = false;
			APPLY_NEXT_RULE(start_of_expression)
			return 0;

		//end of expression
		case TOKEN_PAR_CLOSE: case TOKEN_COMMA: case TOKEN_EOL: case TOKEN_CURLY_OPEN: case TOKEN_SEMICOLON:
			if (!push_rest(sym_stack, list))
				return ERR_INTERNAL;
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
	if (sym == NULL) return ERR_INTERNAL;
	sym->sym_type = SYM_VAR;
	sym->data = var;

	data->current_type = compare_types(var->type, data->current_type);
	if (data->current_type == '0')
	{
		free_symbol(sym);
		return ERR_SEMANTIC_TYPE_COMPAT;
	}

	if (!dll_insert_last(list, sym))
	{
		free_symbol(sym);
		return ERR_INTERNAL;
	}
	return 0;
}

static int push_const(data_t *data, token token, dll_t *list)
{
	symbol_t *sym = malloc(sizeof(symbol_t));
	if (sym == NULL)
		return ERR_INTERNAL;

	if (token.type == TOKEN_INT) //int64 (long)
	{
		sym->sym_type = SYM_INT;
		sym->data = malloc(sizeof(long));
		if (sym->data == NULL)
		{
			free(sym);
			return ERR_INTERNAL;
		}

		*(long*)sym->data = token.attr.int_val;
		data->current_type = compare_types('i', data->current_type);
	}
	else if (token.type == TOKEN_FLOAT64) //float64 (double)
	{
		sym->sym_type = SYM_FLOAT64;
		sym->data = malloc(sizeof(double));
		if (sym->data == NULL)
		{
			free(sym);
			return ERR_INTERNAL;
		}

		*(double*)sym->data = token.attr.float64_val;
		data->current_type = compare_types('f', data->current_type);
	}
	else //string
	{
		sym->sym_type = SYM_STRING;
		string *str = malloc(sizeof(string));
		if (str == NULL)
			return ERR_INTERNAL;

		if (!str_init(str))
		{
			free(str);
			free(sym);
			return ERR_INTERNAL;
		}

		if (!str_add_const(str, token.attr.str->str))
		{
			free_symbol(sym);
			return ERR_INTERNAL;
		}

		sym->data = str;
		data->current_type = compare_types('s', data->current_type);
	}

	if (data->current_type == '0')
	{
		free_symbol(sym);
		return ERR_SEMANTIC_TYPE_COMPAT;
	}

	if (!dll_insert_last(list, sym))
	{
		free_symbol(sym);
		return ERR_INTERNAL;
	}
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
		
		if (!push_rest(sym_stack, list))
			return ERR_INTERNAL;
		return 0;
	}
	return ERR_SYNTAX;
}


static o_type token_to_type(token_type type)
{
	switch (type)
	{
		case TOKEN_ADD:
			return S_ADD;
		case TOKEN_SUB:
			return S_SUB;
		case TOKEN_MUL:
			return S_MUL;
		case TOKEN_DIV:
			return S_DIV;
		case TOKEN_EQUAL:
			return S_EQ;
		case TOKEN_NOT_EQUAL:
			return S_NEQ;
		case TOKEN_LESS_OR_EQUAL:
			return S_LTE;
		case TOKEN_LESS_THAN:
			return S_LT;
		case TOKEN_GREATER_OR_EQUAL:
			return S_GTE;
		case TOKEN_GREATER_THAN:
			return S_GT;
		default:
			return 0;
	}
}

static int push_o(token token, dll_t *list, stack *sym_stack)
{
	o_type *ot = malloc(sizeof(o_type)); //currently loaded operator
	if (ot == NULL)
		return ERR_INTERNAL;
	*ot = token_to_type(token.type);

	//pushing all operators with greater priority than current
	while (sym_stack->count > 0 && precedence[gpi(*(o_type*)sym_stack->top->data)][gpi(*ot)] != 1)
	{
		symbol_t *sym = malloc(sizeof(symbol_t));
		if (sym == NULL)
		{
			free(ot);
			return ERR_INTERNAL;
		}

		sym->sym_type = SYM_OPERATOR;
		sym->data = sym_stack->top->data;
		if (!dll_insert_last(list, sym))
		{
			free(ot);
			free_symbol(sym);
			return ERR_INTERNAL;
		}

		stack_pop(sym_stack, stack_nofree);
	}

	if (!stack_push(sym_stack, ot))
	{
		free(ot);
		return ERR_INTERNAL;
	}
	return 0;
}

static bool push_rest(stack *sym_stack, dll_t *list)
{
	//pushing rest of operators
	while (sym_stack->top != NULL)
	{
		symbol_t *sym = malloc(sizeof(symbol_t));
		if (sym == NULL)
			return false;

		sym->sym_type = SYM_OPERATOR;
		sym->data = sym_stack->top->data;

		if (!dll_insert_last(list, sym))
		{
			free_symbol(sym);
			return false;
		}

		stack_pop(sym_stack, stack_nofree);
	}
	return true;
}

static int gpi(o_type type)
{
	switch (type)
	{
		case S_ADD: case S_SUB:
			return 0;
		case S_MUL: case S_DIV:
			return 1;
		default:
			return 2;
	}
}

static int generate_expression(data_t *data, dll_t *list)
{
	//TODO: generate code
	token tmp_tok;
	string tmp_str;
	tmp_tok.attr.str = &tmp_str;
	dll_node_t *tmp = list->first;

	if (data->assign_for && data->assign_for_swap_output)
	{
		string tmp_swap = ifjcode20_output;
		ifjcode20_output = for_assigns;
		for_assigns = tmp_swap;
		data->assign_for_swap_output = false;
	}

	symbol_t prev_sym1;
	symbol_t prev_sym2;
	prev_sym1.sym_type = SYM_NONE;
	prev_sym2.sym_type = SYM_NONE;

	while (tmp != NULL)
	{
		switch (((symbol_t*)tmp->data)->sym_type)
		{
			case SYM_OPERATOR:
				switch (*((o_type*)((symbol_t*)tmp->data)->data))
				{
					case S_ADD:
						if (prev_sym1.sym_type == SYM_STRING && prev_sym2.sym_type == SYM_STRING)
						{
							CODE_INT("POPS GF@%%tmp2\n"\
									"POPS GF@%%tmp1\n"\
									"CONCAT GF@%%tmp0 GF@%%tmp1 GF@%%tmp2\n"\
									"PUSHS GF@%%tmp0\n");
						}
						else
						{
							CODE_INT("ADDS\n");
						}
						break;
					case S_SUB:
						CODE_INT("SUBS\n");
						break;
					case S_MUL:
						CODE_INT("MULS\n");
						break;
					case S_DIV:
						if (prev_sym1.sym_type == SYM_INT && prev_sym2.sym_type == SYM_INT)
						{
							CODE_INT("IDIVS\n");
						}
						else
						{
							CODE_INT("DIVS\n");
						}
						break;
					case S_EQ:
						CODE_INT("EQS\n");
						prev_sym1.sym_type = SYM_NONE;
						break;
					case S_NEQ:
						CODE_INT("EQS\nNOTS\n");
						prev_sym1.sym_type = SYM_NONE;
						break;
					case S_LT:
						CODE_INT("LTS\n");
						prev_sym1.sym_type = SYM_NONE;
						break;
					case S_GT:
						CODE_INT("GTS\n");
						prev_sym1.sym_type = SYM_NONE;
						break;
					case S_LTE:
						CODE_INT("POPS GF@%%tmp0\n"\
								"POPS GF@%%tmp1\n"\
								"PUSHS GF@%%tmp1\n"\
								"PUSHS GF@%%tmp0\n"\
								"LTS\n"\
								"PUSHS GF@%%tmp1\n"\
								"PUSHS GF@%%tmp0\n"\
								"EQS\n"\
								"ORS\n");
								prev_sym1.sym_type = SYM_NONE;
						break;
					case S_GTE:
						CODE_INT("POPS GF@%%tmp0\n"\
								"POPS GF@%%tmp1\n"\
								"PUSHS GF@%%tmp1\n"\
								"PUSHS GF@%%tmp0\n"\
								"GTS\n"\
								"PUSHS GF@%%tmp1\n"\
								"PUSHS GF@%%tmp0\n"\
								"EQS\n"\
								"ORS\n");
								prev_sym1.sym_type = SYM_NONE;
						break;
					default:
						break;
				}
				prev_sym2.sym_type = SYM_NONE;
				break;
			case SYM_INT:
				tmp_tok.type = TOKEN_INT;
				tmp_tok.attr.int_val = *((long*)((symbol_t*)tmp->data)->data);
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok);
				CODE_INT("\n");

				prev_sym2.sym_type = prev_sym1.sym_type;
				prev_sym1.sym_type = SYM_INT;
				break;
			case SYM_FLOAT64:
				tmp_tok.type = TOKEN_FLOAT64;
				tmp_tok.attr.float64_val = *((double*)((symbol_t*)tmp->data)->data);
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok);
				CODE_INT("\n");

				prev_sym2.sym_type = prev_sym1.sym_type;
				prev_sym1.sym_type = SYM_FLOAT64;
				break;
			case SYM_STRING:
				tmp_tok.type = TOKEN_STRING;
				tmp_tok.attr.str = (string*)((symbol_t*)tmp->data)->data;
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok);
				CODE_INT("\n");

				prev_sym2.sym_type = prev_sym1.sym_type;
				prev_sym1.sym_type = SYM_STRING;
				break;
			case SYM_VAR:
				tmp_tok.type = TOKEN_IDENTIFIER;
				tmp_tok.attr.str = &((var_data_t*)((symbol_t*)tmp->data)->data)->name;
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok); CODE("%%"); CODE_NUM(((var_data_t*)((symbol_t*)tmp->data)->data)->scope_idx);
				CODE_INT("\n");

				prev_sym2.sym_type = prev_sym1.sym_type;
				switch (((var_data_t*)((symbol_t*)tmp->data)->data)->type)
				{
					case 'i':
						prev_sym1.sym_type = SYM_INT;
						break;
					case 'f':
						prev_sym1.sym_type = SYM_FLOAT64;
						break;
					case 's':
						prev_sym1.sym_type = SYM_STRING;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		tmp = tmp->next;
	}
	return 0;
}
