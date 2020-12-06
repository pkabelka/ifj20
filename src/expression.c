/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Implementation of parsing expressions
 * 
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#include "expression.h"
#include "dll.h"
#include "error.h"
#include "scanner.h"
#include "stack.h"
#include "str.h"
#include "codegen.h"
#include "optimizer.h"

#define RET() return data->result;
#define TKN data->token
#define CHECK_RESULT() if (data->result != 0) return data->result;
#define CHECK_RESULT_ERR() if (data->result != 0)
#define NEXT_TOKEN() data->prev_token = data->token; if (get_next_token(&data->token) != SCANNER_SUCCESS) return ERR_LEX_STRUCTURE;
#define APPLY_RULE(func) data->result = func(data, list, sym_stack); CHECK_RESULT()
#define APPLY_NEXT_RULE(func) NEXT_TOKEN() APPLY_RULE(func)
#define EXPECT_TOKEN(token) if (TKN.type != token) return ERR_SYNTAX;
#define EXPECT_NEXT_TOKEN(token) NEXT_TOKEN() if (TKN.type != token) return ERR_SYNTAX;
#define NEW_SYMBOL_ERR(...) data->result = create_symbol(__VA_ARGS__); CHECK_RESULT_ERR()
#define NEW_SYMBOL(...) NEW_SYMBOL_ERR(__VA_ARGS__) CHECK_RESULT()

//precedence table
// 1 = reduce >
// 0 = shouldnt happen
//-1 = shift right <
//10 = equal
//11 = stop parsing
const int precedence[7][7] = {
	//+-  */   (   )   r  id   $
	{  1,  1, -1,  1, -1,  1, -1 }, // +-
	{ -1,  1, -1,  1, -1,  1, -1 }, // */
	{ -1, -1, -1,  0, -1,  0, -1 }, // (
	{  1,  1, 10,  1,  1,  1,  0 }, // )
	{  1,  1, -1,  1,  0,  1, -1 }, // r - relational operators
	{ -1, -1, -1,  0, -1,  0, -1 }, // id - var or constant
	{  1,  1,  0,  1,  1,  1, 11 }  // $
};

static int end_of_expression(data_t *data, dll_t *list, stack *sym_stack);
static int start_of_expression(data_t *data, dll_t *list, stack *sym_stack);
static int generate_expression(data_t *data, dll_t *list);
static int create_symbol(data_t *data, token token, symbol_type type, symbol_t **sym_ptr);
static o_type token_to_type(token_type type);
static int push_symbol(dll_t *list, stack *sym_stack, symbol_t *sym);
static int gpi(symbol_t *sym);

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
	else if (sym->sym_type == SYM_INT || sym->sym_type == SYM_FLOAT64)
		free(sym->data);
	free(ptr);
}

int expression(data_t *data)
{
	dll_t *list = dll_init();
	if (list == NULL)
		return ERR_INTERNAL;

	if (data->vdata == NULL)
		data->current_type = 't';
	else
		data->current_type = data->vdata->type;

	symbol_t *start;
	NEW_SYMBOL_ERR(data, TKN, SYM_STOP, &start)
	{
		dll_dispose(list, stack_nofree);
		RET()
	}

	stack sym_stack;
	stack_init(&sym_stack);
	if (!stack_push(&sym_stack, start)) //push $
	{
		dll_dispose(list, stack_nofree);
		return ERR_INTERNAL;
	}

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
			if (data->vdata != NULL)
				data->vdata->type = data->current_type;

			r = optimize(data, list);
			if (r == 0)
			{
				r = generate_expression(data, list);
			}
		}
	}
	stack_dispose(&sym_stack, free_symbol);
	dll_dispose(list, free_symbol);
	return r;
}

static symbol_type token_to_sym(token_type type)
{
	if (type == TOKEN_INT)
		return SYM_INT;
	else if (type == TOKEN_STRING)
		return SYM_STRING;
	else if (type == TOKEN_FLOAT64)
		return SYM_FLOAT64;
	return 0;
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
		symbol_t *sym;
		NEW_SYMBOL(data, TKN, token_to_sym(TKN.type), &sym);
		data->result = push_symbol(list, sym_stack, sym);
		CHECK_RESULT()

		data->allow_func = false;
		APPLY_NEXT_RULE(end_of_expression)
		return 0;
	}
	else if (TKN.type == TOKEN_PAR_OPEN) //priority brackets
	{	
		symbol_t *sym;
		NEW_SYMBOL(data, TKN, SYM_OPEN, &sym);
		data->result = push_symbol(list, sym_stack, sym);
		CHECK_RESULT()

		data->allow_func = false;	
		APPLY_NEXT_RULE(start_of_expression)
		return 0;
	}
	else if (data->allow_func && is_inter_func(TKN))
	{
		EXPECT_NEXT_TOKEN(TOKEN_PAR_OPEN)
		return 0;
	}
	else if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE)
	{
		return ERR_SEMANTIC_UNDEF_REDEF;
	}

	return ERR_SYNTAX;
}

static int end_of_expression(data_t *data, dll_t *list, stack *sym_stack)
{
	symbol_t *sym;
	if (TKN.type != TOKEN_PAR_OPEN && data->prev_token.type == TOKEN_IDENTIFIER)
	{
		NEW_SYMBOL(data, data->prev_token, SYM_VAR, &sym);
		data->result = push_symbol(list, sym_stack, sym);
		CHECK_RESULT()
	}

	switch (TKN.type)
	{
		//arithmetic operators
		case TOKEN_ADD: case TOKEN_SUB:  case TOKEN_MUL:  case TOKEN_DIV:
			if (data->current_type == 's' && TKN.type != TOKEN_ADD) //unallowed operations with string
				return ERR_SEMANTIC_TYPE_COMPAT;

			NEW_SYMBOL(data, TKN, SYM_OPERATOR, &sym);
			data->result = push_symbol(list, sym_stack, sym);
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

			NEW_SYMBOL(data, TKN, SYM_OPERATOR, &sym);
			data->result = push_symbol(list, sym_stack, sym);
			CHECK_RESULT()

			data->allow_func = false;
			APPLY_NEXT_RULE(start_of_expression)
			return 0;

		//end of priority brackets
		case TOKEN_PAR_CLOSE:
			NEW_SYMBOL(data, TKN, SYM_CLOSE, &sym);
			data->result = push_symbol(list, sym_stack, sym);
			CHECK_RESULT()

			APPLY_NEXT_RULE(end_of_expression)
			return 0;			

		//end of expression
		case TOKEN_COMMA: case TOKEN_EOL: case TOKEN_CURLY_OPEN: case TOKEN_SEMICOLON:
			NEW_SYMBOL(data, TKN, SYM_STOP, &sym);
			data->result = push_symbol(list, sym_stack, sym); //$
			CHECK_RESULT()

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

static int push_symbol(dll_t *list, stack *sym_stack, symbol_t *sym)
{
	int current = gpi(sym);
	while (true)
	{
		symbol_t *sym2 = (symbol_t*)sym_stack->top->data;

		int prec = precedence[current][gpi(sym2)];
		if (prec == 1) //reduce
		{
			if (sym2->sym_type < SYM_STOP)
				if (!dll_insert_last(list, sym2))
					return ERR_INTERNAL;
		}
		else if (prec == 10) //()
		{
			stack_pop(sym_stack, free_symbol);
			free_symbol(sym);
			return 0;
		}
		else if (prec == -1) //shift
			break;
		else if (prec == 11) //$$
		{
			free_symbol(sym);
			return 0;
		}
		else //missing ( or )
		{
			free_symbol(sym);
			return ERR_SYNTAX;
		}

		stack_pop(sym_stack, stack_nofree);
	}

	if (!stack_push(sym_stack, sym))
		return ERR_INTERNAL;
	return 0;
}

static int create_symbol(data_t *data, token token, symbol_type type, symbol_t **sym_ptr)
{
	*sym_ptr = malloc(sizeof(symbol_t));
	symbol_t *sym = *sym_ptr;
	if (sym == NULL)
		return ERR_INTERNAL;

	sym->sym_type = type;
	if (type == SYM_OPERATOR) //operator
	{
		sym->data = malloc(sizeof(o_type)); //currently loaded operator
		if (sym->data == NULL)
		{
			free(sym);
			return ERR_INTERNAL;
		}
		*(o_type*)sym->data = token_to_type(token.type);
	}
	else if (type == SYM_INT) //int64 (long)
	{
		sym->data = malloc(sizeof(long));
		if (sym->data == NULL)
		{
			free(sym);
			return ERR_INTERNAL;
		}
		*(long*)sym->data = token.attr.int_val;
		data->current_type = compare_types('i', data->current_type);
	}
	else if (type == SYM_FLOAT64) //float64 (double)
	{
		sym->data = malloc(sizeof(double));
		if (sym->data == NULL)
		{
			free(sym);
			return ERR_INTERNAL;
		}
		*(double*)sym->data = token.attr.float64_val;
		data->current_type = compare_types('f', data->current_type);
	}
	else if (type == SYM_STRING) //string
	{
		string *str = malloc(sizeof(string));
		if (str == NULL)
		{
			free(sym);
			return ERR_INTERNAL;
		}

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
	else if (type == SYM_VAR) //identifier
	{
		var_data_t *var = find_var(data, token.attr.str->str, false);
		if (var == NULL)
		{
			free(sym);
			return ERR_SEMANTIC_UNDEF_REDEF;
		}

		sym->data = var;
		data->current_type = compare_types(var->type, data->current_type);
		if (data->current_type == '0')
		{
			free_symbol(sym);
			return ERR_SEMANTIC_TYPE_COMPAT;
		}
	}

	if (data->current_type == '0')
	{
		free_symbol(sym);
		return ERR_SEMANTIC_TYPE_COMPAT;
	}
	return 0;
}

static int gpi(symbol_t *sym)
{
	o_type *type;
	switch (sym->sym_type)
	{
		case SYM_OPERATOR:
			type = (o_type*)sym->data;
			switch (*type)
			{
				case S_ADD: case S_SUB:
					return 0;
				case S_MUL: case S_DIV:
					return 1;
				default: // relational operators
					return 4;
			}
		case SYM_OPEN:
			return 2;
		case SYM_CLOSE:
			return 3;
		case SYM_STOP:
			return 6;
		default: // var or constants
			return 5;
	}
}

static int generate_expression(data_t *data, dll_t *list)
{
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

	while (tmp != NULL)
	{
		switch (((symbol_t*)tmp->data)->sym_type)
		{
			case SYM_OPERATOR:
				switch (*((o_type*)((symbol_t*)tmp->data)->data))
				{
					case S_ADD:
						if (data->current_type == 's')
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
						if (data->current_type == 'i')
						{
							if (*((long*)((symbol_t*)tmp->prev->data)->data) == 0L)
								return ERR_ZERO_DIVISION;

							// Detect int zero division at runtime
							CODE_INT("POPS GF@%%tmp0\n"\
									"JUMPIFNEQ $", data->fdata->name.str, "$"); CODE_NUM(data->label_idx+1);
							CODE_INT("$diverr GF@%%tmp0 int@0\n"\
									"EXIT int@9\n"\
									"LABEL $", data->fdata->name.str, "$");
							CODE_NUM(++data->label_idx); CODE_INT("$diverr\n"\
									"PUSHS GF@%%tmp0\n");

							CODE_INT("IDIVS\n");
						}
						else
						{
							if (*((double*)((symbol_t*)tmp->prev->data)->data) == 0.0)
								return ERR_ZERO_DIVISION;

							// Detect float zero division at runtime
							CODE_INT("POPS GF@%%tmp0\n"\
									"JUMPIFNEQ $", data->fdata->name.str, "$"); CODE_NUM(data->label_idx+1);
							CODE_INT("$diverr GF@%%tmp0 float@0x0p+0\n"\
									"EXIT int@9\n"\
									"LABEL $", data->fdata->name.str, "$");
							CODE_NUM(++data->label_idx); CODE_INT("$diverr\n"\
									"PUSHS GF@%%tmp0\n");

							CODE_INT("DIVS\n");
						}
						break;
					case S_EQ:
						CODE_INT("EQS\n");
						break;
					case S_NEQ:
						CODE_INT("EQS\nNOTS\n");
						break;
					case S_LT:
						CODE_INT("LTS\n");
						break;
					case S_GT:
						CODE_INT("GTS\n");
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
						break;
					default:
						break;
				}
				break;
			case SYM_INT:
				tmp_tok.type = TOKEN_INT;
				tmp_tok.attr.int_val = *((long*)((symbol_t*)tmp->data)->data);
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok);
				CODE_INT("\n");
				break;
			case SYM_FLOAT64:
				tmp_tok.type = TOKEN_FLOAT64;
				tmp_tok.attr.float64_val = *((double*)((symbol_t*)tmp->data)->data);
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok);
				CODE_INT("\n");
				break;
			case SYM_STRING:
				tmp_tok.type = TOKEN_STRING;
				tmp_tok.attr.str = (string*)((symbol_t*)tmp->data)->data;
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok);
				CODE_INT("\n");
				break;
			case SYM_VAR:
				tmp_tok.type = TOKEN_IDENTIFIER;
				tmp_tok.attr.str = &((var_data_t*)((symbol_t*)tmp->data)->data)->name;
				CODE_INT("PUSHS ");
				GEN(gen_token_value, &tmp_tok); CODE("%%"); CODE_NUM(((var_data_t*)((symbol_t*)tmp->data)->data)->scope_idx);
				CODE_INT("\n");
				break;
			default:
				break;
		}
		tmp = tmp->next;
	}
	return 0;
}
