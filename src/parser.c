/**
 * @brief Syntax analysis
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 */

#include "parser.h"
#include "scanner.h"
#include "string.h"

#define CHECK(result) if (result != SCANNER_SUCCESS) return false;
#define NEXT_TOKEN() data->prev_token = data->token; CHECK(get_next_token(&data->token));
#define APPLY_RULE(func) if (func(data) == false) return false;
#define TKN data->token

static int func_header(data *data);
static int func_arg(data *data);
static int next_func_arg(data *data);
static int func_return_vals(data *data);
static int var_type(data *data);
static int expression_start(data *data);
static int expression(data *data);
static int func_calling(data *data);
static int scope(data *data);
static int assignment(data *data);
static int list_of_vars(data *data);
static int statement(data *data);
static int cycle(data *data);
static int end_of_cycle(data *data);

static int is_inter_func(token token);

static int init_data(data *data)
{
	stack_init(&data->stack);
	return true;
}

static int dispose_data(data *data)
{
	stack_dispose(&data->stack);
	return true;
}

static int parse(data *data)
{
	//program starts with 'package main'
	NEXT_TOKEN()
	if (TKN.type != TOKEN_KEYWORD)
		return false;
	else if (TKN.attr.kw != KW_PACKAGE)
		return false;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_IDENTIFIER && !str_cmp_const(TKN.attr.str, "main"))
		return false;

	//parsing all functions
	while (TKN.type != TOKEN_EOF)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_FUNC)
		{
			APPLY_RULE(func_header)
			APPLY_RULE(scope)
		}
		else if (TKN.type != TOKEN_EOL)
			return false;
	}

	return true;
}

static int func_header(data *data)
{
	NEXT_TOKEN()
	if (TKN.type != TOKEN_IDENTIFIER) //name of function 
		return false;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_PAR_OPEN) // func name(
		return false;

	APPLY_RULE(func_arg)

	NEXT_TOKEN()
	if (TKN.type == TOKEN_CURLY_OPEN) //start of body
		return true;
	else if (TKN.type == TOKEN_PAR_OPEN) //return vals
	{
		APPLY_RULE(func_return_vals)
		NEXT_TOKEN()
		if (TKN.type == TOKEN_CURLY_OPEN) //start of body
			return true;
	}
	return false;
}

static int func_arg(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE) //no arguments
		return true;
	else if (TKN.type == TOKEN_IDENTIFIER) //one or more
	{
		APPLY_RULE(var_type)
		return next_func_arg(data);
	}
	return false;
}

static int next_func_arg(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE)
		return true;
	else if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_IDENTIFIER)
		{
			APPLY_RULE(var_type)
			return next_func_arg(data);
		}
	}
	return false;
}

static int func_return_vals(data *data)
{
	APPLY_RULE(var_type)
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE)
		return true;
	else if (TKN.type == TOKEN_COMMA)
		return func_return_vals(data);
	return false;
}

static int var_type(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_KEYWORD)
	{
		switch (TKN.attr.kw)
		{
			case KW_INT:

				return true;
				break;
			case KW_FLOAT64:

				return true;
				break;
			case KW_STRING:

				return true;
				break;

			case KW_NIL:

				return true;
				break;

			default:
				return false;
		}
	}
	return false;
}

static int expression_start(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_IDENTIFIER) //func or var
	{
		return expression(data);
	}
	else if (TKN.type == TOKEN_KEYWORD)
	{
		if (is_inter_func(TKN))
			return expression(data);
	}
	else if (TKN.type == TOKEN_ADD || TKN.type == TOKEN_SUB) //unary operators
	{
		if (TKN.type == data->prev_token.type) // ++ / --
			return false;
		return expression_start(data);
	}
	else if (TKN.type == TOKEN_INT || TKN.type == TOKEN_STRING || TKN.type == TOKEN_FLOAT64) //constants
	{
		return expression(data);
	}
	else if (TKN.type == TOKEN_PAR_OPEN)
	{	
		*(int*)data->stack.top->data += 1;
		return expression_start(data);
	}
	else if (TKN.type == TOKEN_PAR_CLOSE && data->prev_token.type == TOKEN_PAR_OPEN && 
			*(int*)data->stack.top->data == 0 && data->stack.count > 0) //none argmunet in fnction call
	{
		return true;
	}

	return false;
}

static int expression(data *data)
{
	NEXT_TOKEN()
	switch (TKN.type)
	{
		//arithmetic operators
		case TOKEN_ADD: case TOKEN_SUB: case TOKEN_MUL: case TOKEN_DIV:
			return expression_start(data);
			break;
		//logic operators
		case TOKEN_GREATER_THAN: case TOKEN_GREATER_OR_EQUAL:
		case TOKEN_LESS_THAN: case TOKEN_LESS_OR_EQUAL: 
		case TOKEN_EQUAL: case TOKEN_NOT_EQUAL: 
			return expression_start(data);
			break;
		// )
		case TOKEN_PAR_CLOSE:
			*(int*)data->stack.top->data -= 1;
			if (*(int*)data->stack.top->data >= 0)
				return expression(data); 
			else if (*(int*)data->stack.top->data == -1 && data->stack.count > 0) //end of expression ... end of calling function
				return true;
			break;
		//function
		case TOKEN_PAR_OPEN:
			if (data->prev_token.type == TOKEN_IDENTIFIER) //function
			{
				APPLY_RULE(func_calling)
				return expression(data);	
			}
			else if (data->prev_token.type == TOKEN_KEYWORD && is_inter_func(data->prev_token)) //inter functions
			{
				APPLY_RULE(func_calling)
				return expression(data);
			}
			break;
		//end of expression
		case TOKEN_COMMA: case TOKEN_EOL: case TOKEN_CURLY_OPEN: case TOKEN_SEMICOLON:
			return true;
			break;

		default:
			return false;
	}
	return false;
}

static int func_calling(data *data)
{	
	int zero = 0;
	stack_push(&data->stack, &zero);
	APPLY_RULE(expression_start)
	if (TKN.type == TOKEN_COMMA)
		return func_calling(data);
	else if (TKN.type == TOKEN_PAR_CLOSE)
	{
		stack_pop(&data->stack);
		return true;
	}
	return false;
}

static int scope(data *data)
{
	NEXT_TOKEN();
	do
	{
		if (TKN.type == TOKEN_EOL)
		{
			NEXT_TOKEN()
			if (TKN.type == TOKEN_CURLY_CLOSE)
				return true;
		}
		
		if (TKN.type == TOKEN_IDENTIFIER)
		{
			NEXT_TOKEN()
			if (TKN.type == TOKEN_COMMA) //vars assignment
			{
				APPLY_RULE(list_of_vars)
				APPLY_RULE(assignment)
				if (data->prev_token.type != TOKEN_EOL)
					return false;
			}
			else if (TKN.type == TOKEN_PAR_OPEN) //function
			{
				APPLY_RULE(func_calling)
			}
			else if (TKN.type == TOKEN_ASSIGN || TKN.type == TOKEN_REASSIGN) //assignment
			{
				APPLY_RULE(assignment)
				if (data->prev_token.type != TOKEN_EOL)
					return false;
			}
			else
				return false;
		}
		else if (TKN.type == TOKEN_KEYWORD)
		{
			if (is_inter_func(TKN)) //calling inter functions
			{
				NEXT_TOKEN()
				if (TKN.type == TOKEN_PAR_OPEN)
				{
					APPLY_RULE(func_calling)
				}
				else
					return false;				
			}
			else if (TKN.attr.kw == KW_IF)
			{
				APPLY_RULE(statement)
			}
			else if (TKN.attr.kw == KW_FOR)
			{
				APPLY_RULE(cycle)
				APPLY_RULE(scope)
			}
			else
				return false;
		}
		else
			return false;
	} while (TKN.type != TOKEN_CURLY_CLOSE);

	return true;
}

static int assignment(data *data)
{
	APPLY_RULE(expression_start)
	if (data->prev_token.type == TOKEN_COMMA)
		return assignment(data);	
	else if (data->prev_token.type == TOKEN_EOL || 
			data->prev_token.type == TOKEN_SEMICOLON ||
			data->prev_token.type == TOKEN_CURLY_OPEN)
		return true;
	return false;
}

static int list_of_vars(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_IDENTIFIER || (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE))
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_ASSIGN || TKN.type == TOKEN_REASSIGN)
		{
			return true;
		}
		else if (TKN.type == TOKEN_COMMA) 
		{
			return list_of_vars(data);
		}
	}
	return false;
}

static int statement(data *data)
{
	APPLY_RULE(expression)
	if (data->prev_token.type == TOKEN_CURLY_OPEN)
	{
		NEXT_TOKEN()
		if (TKN.type != TOKEN_EOL)
			return false;

		APPLY_RULE(scope) //new scope if

		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_ELSE)
		{
			NEXT_TOKEN()
			if (TKN.type != TOKEN_CURLY_OPEN)
				return false;

			NEXT_TOKEN()
			if (TKN.type != TOKEN_EOL)
				return false;

			return scope(data); //new scope else
		}
	}
	return false;
}

static int cycle(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_SEMICOLON) //empty
	{
		APPLY_RULE(expression_start)
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
			return false;

		APPLY_RULE(assignment)
		if (TKN.type == TOKEN_SEMICOLON)
		{
			APPLY_RULE(expression_start)
			if (data->prev_token.type == TOKEN_SEMICOLON)
				return end_of_cycle(data);
		}
	}

	return false;
}

static int end_of_cycle(data *data)
{
	NEXT_TOKEN()
	if (TKN.type == TOKEN_CURLY_OPEN) //empty
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_EOL)
			return true;
	}
	else if (TKN.type == TOKEN_IDENTIFIER)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA)
		{
			APPLY_RULE(list_of_vars)
			if (data->prev_token.type != TOKEN_REASSIGN)
				return false;
		}
		else if (TKN.type != TOKEN_REASSIGN)
			return false;

		APPLY_RULE(assignment)
		if (TKN.type == TOKEN_CURLY_OPEN)
		{
			NEXT_TOKEN()
			if (TKN.type == TOKEN_EOL)
			{
				return true;
			}
		}
	}

	return false;
}

static int is_inter_func(token token)
{
	switch (token.attr.kw)
	{
		case KW_CHR: case KW_FLOAT2INT: case KW_INPUTF: case KW_PRINT: case KW_SUBSTR:
		case KW_INPUTI: case KW_INPUTS: case KW_INT2FLOAT: case KW_ORD: 
			return true;
		default:
			return false;
	}
}
