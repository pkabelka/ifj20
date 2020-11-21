/**
 * @brief Syntax analysis
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 */

#include "parser.h"
#include "scanner.h"
#include "string.h"
#include "error.h"

#define NEXT_TOKEN() data->prev_token = data->token; if (get_next_token(&data->token) != SCANNER_SUCCESS) return ERR_LEX_STRUCTURE;
#define APPLY_RULE(func) data->result = func(data); if (data->result != 0) return data->result;
#define TKN data->token
#define EXPECT_TOKEN(token) if (TKN.type != token) return ERR_SYNTAX;

static int func_header(data *data);
static int func_arg(data *data);
static int next_func_arg(data *data);
static int func_return_vals(data *data);
static int var_type(data *data);
static int expression(data *data);
static int expression_old_token(data *data);
static int end_of_expression(data *data);
static int func_calling(data *data);
static int scope(data *data);
static int assignment(data *data);
static int list_of_vars(data *data);
static int statement(data *data);
static int cycle(data *data);
static int end_of_cycle(data *data);
static int returned_vals(data *data);
static int next_returned_val(data *data);

static bool is_inter_func(token token);

int init_data(data *data)
{
	stack_init(&data->stack);
	int zero = 0;
	stack_push(&data->stack, &zero);
	data->result = 0;

	return true;
}

int dispose_data(data *data)
{
	stack_dispose(&data->stack);
	return true;
}

int parse(data *data)
{
	//program starts with 'package main'
	NEXT_TOKEN()
	if (TKN.type == TOKEN_EOL)
	{
		NEXT_TOKEN()
	}

	if (TKN.type != TOKEN_KEYWORD)
		return ERR_SYNTAX;
	else if (TKN.attr.kw != KW_PACKAGE)
		return ERR_SYNTAX;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_IDENTIFIER && !str_cmp_const(TKN.attr.str, "main"))
		return 10;

	//parsing all functions
	while (TKN.type != TOKEN_EOF)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_FUNC)
		{
			APPLY_RULE(func_header)
			APPLY_RULE(scope)
		}
		else if (TKN.type != TOKEN_EOL && TKN.type != TOKEN_EOF)
			return ERR_SYNTAX;
	}

	return 0;
}

static int func_header(data *data)
{
	NEXT_TOKEN()
	if (TKN.type != TOKEN_IDENTIFIER) //name of function 
		return ERR_SYNTAX;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_PAR_OPEN) // func name(
		return ERR_SYNTAX;

	APPLY_RULE(func_arg)

	NEXT_TOKEN()
	if (TKN.type == TOKEN_CURLY_OPEN) //start of body
		return 0;
	else if (TKN.type == TOKEN_PAR_OPEN) //return vals
	{
		APPLY_RULE(func_return_vals)
		NEXT_TOKEN()
		if (TKN.type == TOKEN_CURLY_OPEN) //start of body
			return 0;
	}
	else if (TKN.type == TOKEN_KEYWORD)
	{
		if (TKN.attr.kw == KW_INT || TKN.attr.kw == KW_STRING || TKN.attr.kw == KW_FLOAT64) //one returned value withou ())
		{
			NEXT_TOKEN()
			if (TKN.type == TOKEN_CURLY_OPEN) //starts of body
				return 0;
		}
	}
	return ERR_SYNTAX;
}

static int func_arg(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE) //no arguments
		return 0;
	else if (TKN.type == TOKEN_IDENTIFIER) //one or more
	{
		APPLY_RULE(var_type)
		return next_func_arg(data);
	}
	return ERR_SYNTAX;
}

static int next_func_arg(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE)
		return 0;
	else if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_IDENTIFIER)
		{
			APPLY_RULE(var_type)
			return next_func_arg(data);
		}
	}
	return ERR_SYNTAX;
}

static int func_return_vals(data *data)
{
	APPLY_RULE(var_type)
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE)
		return 0;
	else if (TKN.type == TOKEN_COMMA)
		return func_return_vals(data);
	return ERR_SYNTAX;
}

static int var_type(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_KEYWORD)
	{
		switch (TKN.attr.kw)
		{
			case KW_INT:

				return 0;
				break;
			case KW_FLOAT64:

				return 0;
				break;
			case KW_STRING:

				return 0;
				break;

			case KW_NIL:

				return 0;
				break;

			default:
				return ERR_SYNTAX;
		}
	}
	return ERR_SYNTAX;
}

static int expression(data *data)
{
	NEXT_TOKEN()
	return expression_old_token(data);
}

static int expression_old_token(data *data) 
{
	if (TKN.type == TOKEN_IDENTIFIER) //func or var
	{
		return end_of_expression(data);
	}
	else if (TKN.type == TOKEN_KEYWORD)
	{
		if (is_inter_func(TKN))
			return end_of_expression(data);
	}
	else if (TKN.type == TOKEN_ADD || TKN.type == TOKEN_SUB) //unary operators
	{
		if (TKN.type == data->prev_token.type) // ++ / --
			return ERR_SYNTAX;
		return expression(data);
	}
	else if (TKN.type == TOKEN_INT || TKN.type == TOKEN_STRING || TKN.type == TOKEN_FLOAT64) //constants
	{
		return end_of_expression(data);
	}
	else if (TKN.type == TOKEN_PAR_OPEN)
	{	
		*(int*)data->stack.top->data += 1;
		return expression(data);
	}
	else if (TKN.type == TOKEN_PAR_CLOSE && data->prev_token.type == TOKEN_PAR_OPEN && 
			*(int*)data->stack.top->data == 0 && data->stack.count > 1) //none argmunet in function call
	{
		return 0;
	}

	return ERR_SYNTAX;
}

static int end_of_expression(data *data)
{
	NEXT_TOKEN()
	switch (TKN.type)
	{
		//arithmetic operators
		case TOKEN_ADD: case TOKEN_SUB: case TOKEN_MUL: case TOKEN_DIV:
			return expression(data);
			break;
		//logic operators
		case TOKEN_GREATER_THAN: case TOKEN_GREATER_OR_EQUAL:
		case TOKEN_LESS_THAN: case TOKEN_LESS_OR_EQUAL: 
		case TOKEN_EQUAL: case TOKEN_NOT_EQUAL: 
			return expression(data);
			break;
		// )
		case TOKEN_PAR_CLOSE:
			*(int*)data->stack.top->data -= 1;
			if (*(int*)data->stack.top->data >= 0)
				return end_of_expression(data); 
			else if (*(int*)data->stack.top->data == -1 && data->stack.count > 1) //end of expression ... end of calling function
				return 0;
			break;
		//function
		case TOKEN_PAR_OPEN:
			if (data->prev_token.type == TOKEN_IDENTIFIER) //function
			{
				APPLY_RULE(func_calling)
				return end_of_expression(data);	
			}
			else if (data->prev_token.type == TOKEN_KEYWORD && is_inter_func(data->prev_token)) //inter functions
			{
				APPLY_RULE(func_calling)
				return end_of_expression(data);
			}
			break;
		//end of expression
		case TOKEN_COMMA: case TOKEN_EOL: case TOKEN_CURLY_OPEN: case TOKEN_SEMICOLON:
			return 0;
			break;

		default:
			return ERR_SYNTAX;
	}
	return ERR_SYNTAX;
}

static int func_calling(data *data)
{	
	int zero = 0;
	stack_push(&data->stack, &zero);
	APPLY_RULE(expression)
	if (TKN.type == TOKEN_COMMA)
		return func_calling(data);
	else if (TKN.type == TOKEN_PAR_CLOSE)
	{
		stack_pop(&data->stack);
		return 0;
	}
	return ERR_SYNTAX;
}

static int scope(data *data)
{
	NEXT_TOKEN();
	if (data->prev_token.type == TOKEN_CURLY_OPEN && TKN.type == TOKEN_CURLY_CLOSE)
		return false;

	do
	{
		if (TKN.type == TOKEN_EOL)
		{
			NEXT_TOKEN()
			continue;
		}
		
		if (TKN.type == TOKEN_CURLY_CLOSE)
			return 0;
		else if (TKN.type == TOKEN_IDENTIFIER)
		{
			NEXT_TOKEN()
			if (TKN.type == TOKEN_COMMA) //vars assignment
			{
				APPLY_RULE(list_of_vars)
				APPLY_RULE(assignment)
				EXPECT_TOKEN(TOKEN_EOL)
			}
			else if (TKN.type == TOKEN_PAR_OPEN) //function
			{
				APPLY_RULE(func_calling)
				NEXT_TOKEN()
				EXPECT_TOKEN(TOKEN_EOL)
			}
			else if (TKN.type == TOKEN_ASSIGN || TKN.type == TOKEN_REASSIGN) //assignment
			{
				APPLY_RULE(assignment)
				EXPECT_TOKEN(TOKEN_EOL)
			}
			else
				return ERR_SYNTAX;
		}
		else if (TKN.type == TOKEN_KEYWORD)
		{
			if (TKN.attr.kw == KW_UNDERSCORE)
			{
				NEXT_TOKEN()
				if (TKN.type == TOKEN_COMMA)
				{
					APPLY_RULE(list_of_vars)
					APPLY_RULE(assignment)
					EXPECT_TOKEN(TOKEN_EOL)
				}
				else if (TKN.type == TOKEN_ASSIGN || TKN.type == TOKEN_REASSIGN) //assignment
				{
					APPLY_RULE(assignment)
					EXPECT_TOKEN(TOKEN_EOL)
				}
				else
					return ERR_SYNTAX;
			}
			else if (is_inter_func(TKN)) //calling inter functions
			{
				NEXT_TOKEN()
				if (TKN.type == TOKEN_PAR_OPEN)
				{
					APPLY_RULE(func_calling)
					NEXT_TOKEN()
					EXPECT_TOKEN(TOKEN_EOL)
				}
				else
					return ERR_SYNTAX;				
			}
			else if (TKN.attr.kw == KW_IF)
			{
				APPLY_RULE(statement)
			}
			else if (TKN.attr.kw == KW_FOR)
			{
				APPLY_RULE(cycle)
				APPLY_RULE(scope)
				NEXT_TOKEN()
			}
			else if (TKN.attr.kw == KW_RETURN)
			{
				APPLY_RULE(returned_vals)
			}
			else
				return ERR_SYNTAX;
		}
		else
			return ERR_SYNTAX;
	} while (TKN.type != TOKEN_CURLY_CLOSE);

	return 0;
}

static int assignment(data *data)
{
	APPLY_RULE(expression)
	if (TKN.type == TOKEN_COMMA)
		return assignment(data);	
	else if (TKN.type == TOKEN_EOL || 
			TKN.type == TOKEN_SEMICOLON ||
			TKN.type == TOKEN_CURLY_OPEN)
		return 0;
	return ERR_SYNTAX;
}

static int list_of_vars(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_IDENTIFIER || (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE))
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_ASSIGN || TKN.type == TOKEN_REASSIGN)
		{
			return 0;
		}
		else if (TKN.type == TOKEN_COMMA) 
		{
			return list_of_vars(data);
		}
	}
	return ERR_SYNTAX;
}

static int statement(data *data)
{
	APPLY_RULE(expression)
	if (TKN.type == TOKEN_CURLY_OPEN)
	{
		NEXT_TOKEN()
		if (TKN.type != TOKEN_EOL)
			return ERR_SYNTAX;

		APPLY_RULE(scope) //new scope if

		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_ELSE)
		{
			NEXT_TOKEN()
			if (TKN.type != TOKEN_CURLY_OPEN)
				return ERR_SYNTAX;

			NEXT_TOKEN()
			if (TKN.type != TOKEN_EOL)
				return ERR_SYNTAX;

			APPLY_RULE(scope) //new scope else
			NEXT_TOKEN();
			if (TKN.type == TOKEN_EOL)
				return 0;
		}
	}
	return ERR_SYNTAX;
}

static int cycle(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_SEMICOLON) //empty
	{
		APPLY_RULE(expression)
		if (TKN.type == TOKEN_SEMICOLON)
			return end_of_cycle(data);
	}
	else if (TKN.type == TOKEN_IDENTIFIER)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA)
		{
			APPLY_RULE(list_of_vars)
		}
		else if (TKN.type != TOKEN_ASSIGN && TKN.type != TOKEN_REASSIGN)
			return ERR_SYNTAX;

		APPLY_RULE(assignment)
		if (TKN.type == TOKEN_SEMICOLON)
		{
			APPLY_RULE(expression)
			if (TKN.type == TOKEN_SEMICOLON)
				return end_of_cycle(data);
		}
	}

	return ERR_SYNTAX;
}

static int end_of_cycle(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_CURLY_OPEN) //empty
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_EOL)
			return 0;
	}
	else if (TKN.type == TOKEN_IDENTIFIER)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA)
		{
			APPLY_RULE(list_of_vars)
			if (TKN.type != TOKEN_REASSIGN)
				return ERR_SYNTAX;
		}
		else if (TKN.type != TOKEN_REASSIGN)
			return ERR_SYNTAX;

		APPLY_RULE(assignment)
		if (TKN.type == TOKEN_CURLY_OPEN)
		{
			NEXT_TOKEN()
			if (TKN.type == TOKEN_EOL)
			{
				return 0;
			}
		}
	}

	return ERR_SYNTAX;
}

static int returned_vals(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_EOL) //empty
	{
		return 0;
	}

	APPLY_RULE(expression_old_token)
	if (TKN.type == TOKEN_COMMA)
	{
		return next_returned_val(data);
	}
	else if (TKN.type == TOKEN_EOL)
	{
		return 0;
	}

	return ERR_SYNTAX;
}

static int next_returned_val(data *data)
{
	APPLY_RULE(expression)
	if (TKN.type == TOKEN_COMMA)
	{
		return next_returned_val(data);
	}
	else if (TKN.type == TOKEN_EOL)
	{
		return 0;
	}
	return ERR_SYNTAX;
}

static bool is_inter_func(token token)
{
	switch (token.attr.kw)
	{
		case KW_CHR: case KW_FLOAT2INT: case KW_INPUTF: case KW_PRINT: case KW_SUBSTR:
		case KW_INPUTI: case KW_INPUTS: case KW_INT2FLOAT: case KW_ORD: case KW_LEN: 
			return true;
		default:
			return false;
	}

	return false;
}
