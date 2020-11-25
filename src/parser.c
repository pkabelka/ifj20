/**
 * @brief Parser implementation
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 */

#include "parser.h"
#include "scanner.h"
#include "str.h"
#include "error.h"
#include "expression.h"
#include "enum_str.h"

#define CHECK_RESULT() if (data->result != 0) return data->result;
#define NEXT_TOKEN() data->prev_token = data->token; if (get_next_token(&data->token) != SCANNER_SUCCESS) return ERR_LEX_STRUCTURE;
#define APPLY_RULE(func) data->result = func(data); if (data->result != 0) return data->result;
#define APPLY_NEXT_RULE(func) NEXT_TOKEN() APPLY_RULE(func)
#define TKN data->token
#define EXPECT_TOKEN(token) if (TKN.type != token) return ERR_SYNTAX;
#define EXPECT_NEXT_TOKEN(token) NEXT_TOKEN() if (TKN.type != token) return ERR_SYNTAX;

//rules
static int func_header(data_t *data);
static int func_args(data_t *data);
static int func_return_vals(data_t *data);
static int var_type(data_t *data);
static int call_func(data_t *data);
static int func_calling(data_t *data);
static int _scope_(data_t *data);
static int scope(data_t *data);
static int assignment(data_t *data);
static int reassignment(data_t *data);
static int list_of_vars(data_t *data);
static int statement(data_t *data);
static int func_or_list_of_vars(data_t *data);
static int cycle(data_t *data);
static int end_of_cycle(data_t *data);
static int condition(data_t *data);
static int returned_vals(data_t *data);
static int next_returned_val(data_t *data, unsigned int n);
static int new_scope(data_t *data);
static int close_scope(data_t *data);
static int end_of_assignment(data_t *data, dll_node_t *node);
static int check_ret_vals(data_t *data, char type, unsigned int n);
static int check_func_calls(data_t *data);

static char kw_to_char(keyword kw);
static bool add_inter_func_to_table(data_t *data);
static char tkn_to_char(token token);
static bool add_to_assign_list(data_t *data, token token);
static void set_return_types(data_t *data, dll_node_t *node);
static bool compare_list_of_types(string expected, string sent);
static var_data_t* create_aux_var(data_t *data);

bool init_func_data(void **ptr);
func_call_data_t* init_func_call_data();
void free_func_data(void *ptr);
void free_local_scope(void *ptr);
void free_func_call_data(void *ptr);
void free_var_data(void *ptr);

bool init_data(data_t *data)
{
	data->current_type = '0';
	data->result = 0;
	stack_init(&data->var_table);
	stack_init(&data->calls);
	stack_init(&data->aux);
	symtable_init(&data->func_table);
	data->assign_list = dll_init();

	add_inter_func_to_table(data);

	return true;
}

void dispose_data(data_t *data)
{
	symtable_dispose(&data->func_table, free_func_data);
	stack_dispose(&data->calls, free_func_call_data);
	stack_dispose(&data->var_table, free_local_scope);
	stack_dispose(&data->aux, free);
}

bool init_func_data(void **ptr)
{
	*ptr = malloc(sizeof(func_data_t));
	if (*ptr == NULL) return false;
	func_data_t **fd = (func_data_t**)ptr;
	bool res = true;
	res = res && str_init(&(*fd)->args_types);
	res = res && str_init(&(*fd)->ret_val_types);
	(*fd)->nargs = false;
	return res;
}

func_call_data_t* init_func_call_data()
{
	func_call_data_t *fcd = malloc(sizeof(func_call_data_t));
	if (fcd == NULL) return NULL;
	bool res = true;
	res = res && str_init(&fcd->args_types);
	res = res && str_init(&fcd->func_name);
	res = res && str_init(&fcd->expected_return);
	if (!res) 
		return NULL;
	return fcd;
}

void free_func_call_data(void *ptr)
{
	func_call_data_t *fcd = (func_call_data_t*)ptr;
	str_free(&fcd->args_types);
	str_free(&fcd->func_name);
	str_free(&fcd->expected_return);
	free(fcd);
}

void free_func_data(void *ptr)
{
	func_data_t *fd = (func_data_t*)ptr;
	str_free(&fd->args_types);
	str_free(&fd->ret_val_types);
	free(fd);
}

void free_local_scope(void *ptr)
{
	stnode_ptr *node = (stnode_ptr*)ptr;
	symtable_dispose(node, free_var_data);
}

void free_var_data(void *ptr)
{
	free(ptr);
}

int parse(data_t *data)
{
	//program starts with 'package main'
	NEXT_TOKEN()
	while (TKN.type == TOKEN_EOL)
	{
		NEXT_TOKEN()
	}

	if (TKN.type != TOKEN_KEYWORD)
		return ERR_SYNTAX;
	else if (TKN.attr.kw != KW_PACKAGE)
		return ERR_SYNTAX;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_IDENTIFIER && !str_cmp_const(TKN.attr.str, "main"))
		return ERR_SEMANTIC_OTHER;
	
	//parsing all functions
	while (TKN.type != TOKEN_EOF)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_FUNC)
		{
			APPLY_RULE(new_scope)
			APPLY_NEXT_RULE(func_header)
			APPLY_NEXT_RULE(_scope_);
			APPLY_RULE(close_scope)
		}
		else if (TKN.type != TOKEN_EOL && TKN.type != TOKEN_EOF)
			return ERR_SYNTAX;
	}

	return check_func_calls(data);
}

static int func_header(data_t *data)
{
	if (TKN.type != TOKEN_IDENTIFIER) //name of function
		return ERR_SYNTAX;
	
	char *name = TKN.attr.str->str;
	if (symtable_search(data->func_table, name) != NULL)
		return ERR_SEMANTIC_UNDEF_REDEF; //this funcion name already exist

	bool err;
	stnode_ptr ptr = symtable_insert(&data->func_table, name, &err);
	if (ptr == NULL || !init_func_data(&ptr->data))
		return ERR_INTERNAL;
	data->fdata = ptr->data;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_PAR_OPEN) // func name(
		return ERR_SYNTAX;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_PAR_CLOSE) //1+ args
	{
		APPLY_RULE(func_args)
	}
	
	NEXT_TOKEN()
	if (TKN.type == TOKEN_CURLY_OPEN) //start of body
		return 0;
	else if (TKN.type == TOKEN_PAR_OPEN) //return vals
	{
		APPLY_NEXT_RULE(func_return_vals)
		NEXT_TOKEN()
		if (TKN.type == TOKEN_CURLY_OPEN) //start of body
			return 0;
	}
	else if (TKN.type == TOKEN_KEYWORD)
	{
		if (TKN.attr.kw == KW_INT || TKN.attr.kw == KW_STRING || TKN.attr.kw == KW_FLOAT64) //one returned value without ()
		{
			str_add(&data->fdata->ret_val_types, kw_to_char(TKN.attr.kw));
			NEXT_TOKEN()
			if (TKN.type == TOKEN_CURLY_OPEN) //starts of body
				return 0;
		}
	}
	return ERR_SYNTAX;
}

static int func_args(data_t *data)
{
	if (TKN.type == TOKEN_IDENTIFIER) //one or more
	{
		APPLY_NEXT_RULE(var_type)
		//add arg type to header
		str_add(&data->fdata->args_types, kw_to_char(TKN.attr.kw));
		//create var
		var_data_t *vd = malloc(sizeof(var_data_t));
		if (vd == NULL)
			return ERR_INTERNAL;

		vd->type = kw_to_char(TKN.attr.kw);
		if (!str_init(&vd->name))
			return ERR_INTERNAL;

		char *name = data->prev_token.attr.str->str;
		str_add_const(&vd->name, name);
		
		//add var into local scope
		bool err;
		stnode_ptr ptr = symtable_insert((stnode_ptr*)data->var_table.top->data, name, &err);
		if (ptr == NULL)
			return ERR_INTERNAL;
		ptr->data = vd;

		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA)
		{
			APPLY_NEXT_RULE(func_args)
			return 0;
		}
		else if (TKN.type == TOKEN_PAR_CLOSE)
			return 0;
	}
	return ERR_SYNTAX;
}

static int func_return_vals(data_t *data)
{
	APPLY_RULE(var_type)
	str_add(&data->fdata->ret_val_types, kw_to_char(TKN.attr.kw));
	NEXT_TOKEN()
	if (TKN.type == TOKEN_PAR_CLOSE)
		return 0;
	else if (TKN.type == TOKEN_COMMA)
	{
		APPLY_NEXT_RULE(func_return_vals)
		return 0;
	}
	return ERR_SYNTAX;
}

static int var_type(data_t *data)
{
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
			default:
				return ERR_SYNTAX;
		}
	}
	return ERR_SYNTAX;
}

static int call_func(data_t *data)
{
	func_call_data_t *call = init_func_call_data();
	if (call == NULL)
		return ERR_INTERNAL;
	
	call->line = TKN.line;
	str_add(&call->expected_return, 'r');
	char *name;
	if (data->prev_token.type == TOKEN_KEYWORD)
		name = keyword_str(data->prev_token.attr.kw);
	else
		name = data->prev_token.attr.str->str;
	str_add_const(&call->func_name, name);
	if (!stack_push(&data->calls, call)) 
		return ERR_INTERNAL;

	APPLY_NEXT_RULE(func_calling)
	return 0;
}

static int func_calling(data_t *data)
{
	if (TKN.type == TOKEN_IDENTIFIER)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA || TKN.type == TOKEN_PAR_CLOSE) //variable as func parameter
		{
			var_data_t *vd = find_var(data, data->prev_token.attr.str->str, false);
			if (vd == NULL) //used undefined variable
				return ERR_SEMANTIC_UNDEF_REDEF;
			str_add(&((func_call_data_t*)data->calls.top->data)->args_types, vd->type);
			
			if (TKN.type == TOKEN_PAR_CLOSE)
				return 0;

			APPLY_NEXT_RULE(func_calling)
			return 0;
		}
	}
	else if (TKN.type == TOKEN_INT || TKN.type == TOKEN_STRING || TKN.type == TOKEN_FLOAT64)
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA || TKN.type == TOKEN_PAR_CLOSE) //constants as func parameter
		{
			str_add(&((func_call_data_t*)data->calls.top->data)->args_types, tkn_to_char(data->prev_token));
			
			if (TKN.type == TOKEN_PAR_CLOSE)
				return 0;

			APPLY_NEXT_RULE(func_calling)
			return 0;
		}
	}
	else if (TKN.type == TOKEN_PAR_CLOSE && data->prev_token.type != TOKEN_COMMA)
	{
		return 0;
	}
	return ERR_SYNTAX;
}

static int _scope_(data_t *data)
{
	if (data->prev_token.type == TOKEN_CURLY_OPEN && TKN.type == TOKEN_CURLY_CLOSE)
		return false;

	while (true)
	{
		if (TKN.type == TOKEN_EOL)
		{
			NEXT_TOKEN()
			continue;
		}
		if (TKN.type == TOKEN_CURLY_CLOSE)
			break;
		else if (TKN.type == TOKEN_IDENTIFIER || (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE))
		{
			APPLY_NEXT_RULE(func_or_list_of_vars)
			EXPECT_TOKEN(TOKEN_EOL)
		}
		else if (TKN.type == TOKEN_KEYWORD)
		{
			if (is_inter_func(TKN)) //calling inter functions
			{
				NEXT_TOKEN()
				if (TKN.type == TOKEN_PAR_OPEN)
				{
					APPLY_RULE(call_func)
					EXPECT_NEXT_TOKEN(TOKEN_EOL)
				}
				else
					return ERR_SYNTAX;
			}
			else if (TKN.attr.kw == KW_IF)
			{
				APPLY_NEXT_RULE(statement)
			}
			else if (TKN.attr.kw == KW_FOR)
			{
				APPLY_NEXT_RULE(cycle)
			}
			else if (TKN.attr.kw == KW_RETURN)
			{
				APPLY_NEXT_RULE(returned_vals)
			}
			else
				return ERR_SYNTAX;
		}
		else
			return ERR_SYNTAX;
	}
	return 0;
}

static int scope(data_t *data)
{
	APPLY_RULE(new_scope)
	APPLY_RULE(_scope_)
	APPLY_RULE(close_scope)
	return 0;
}

static int assignment(data_t *data)
{
	dll_node_t *node = data->assign_list->first;
	while (node != NULL)
	{
		if (node->data != NULL)
		{
			var_data_t *assign = (var_data_t*)node->data;
			var_data_t *vd = find_var(data, assign->name.str, true);
			if (vd != NULL)
				return ERR_SEMANTIC_UNDEF_REDEF; //variable alreay exist in current scope

			bool err;
			stnode_ptr ptr = symtable_insert(((stnode_ptr*)data->var_table.top->data), assign->name.str, &err);
			if (ptr == NULL)
				return ERR_INTERNAL;

			assign->type = 't';
			ptr->data = assign;
		}
		node = node->next;
	}

	data->nassigns = 0;
	data->allow_func = false;
	data->result = end_of_assignment(data, data->assign_list->first);
	CHECK_RESULT()

	dll_clear(data->assign_list, stack_nofree);
	return 0;
}

static int reassignment(data_t *data)
{
	dll_node_t *node = data->assign_list->first;
	while (node != NULL)
	{
		if (node->data != NULL)
		{
			var_data_t *assign = (var_data_t*)node->data;
			var_data_t *vd = find_var(data, assign->name.str, false);
			if (vd == NULL)
				return ERR_SEMANTIC_UNDEF_REDEF;
			assign->type = vd->type;
		}
		node = node->next;
	}

	data->nassigns = 0;
	data->allow_func = true;
	data->result = end_of_assignment(data, data->assign_list->first);
	CHECK_RESULT()

	dll_clear(data->assign_list, stack_nofree);
	return 0;
}

static int end_of_assignment(data_t *data, dll_node_t *node)
{	
	data->vdata = (var_data_t*)node->data;
	APPLY_RULE(expression)
	data->nassigns += 1;
	if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		if (node->next == NULL) //L < R
			return ERR_SEMANTIC_FUNC_PARAMS;

		NEXT_TOKEN()
		if (TKN.type == TOKEN_EOL)
			return 0;
		
		NEXT_TOKEN();
		return end_of_assignment(data, node->next); //assigning next value
	}
	else if (TKN.type == TOKEN_EOL ||
			TKN.type == TOKEN_SEMICOLON ||
			TKN.type == TOKEN_CURLY_OPEN)
		return 0;
	else if (TKN.type == TOKEN_PAR_OPEN)
	{
		if (data->nassigns == 2) //fix last call
		{
			func_call_data_t *call = (func_call_data_t*)data->calls.top->data;
			char t = call->expected_return.str[0];
			str_clear(&call->expected_return);
			str_add(&call->expected_return, t);
		}

		APPLY_RULE(call_func)
		if (data->nassigns == 1)
			set_return_types(data, NULL);
		else
			set_return_types(data, node);
		
		NEXT_TOKEN()
		if (TKN.type == TOKEN_EOL)
			return 0;
		
		NEXT_TOKEN();
		return end_of_assignment(data, node); //assigning still current value
	}
	return ERR_SYNTAX;
}

static int list_of_vars(data_t *data)
{
	if (TKN.type == TOKEN_IDENTIFIER || (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE))
	{
		if (!add_to_assign_list(data, TKN)) 
			return ERR_INTERNAL;

		NEXT_TOKEN()
		if (TKN.type == TOKEN_ASSIGN && data->allow_assign)
		{
			APPLY_NEXT_RULE(assignment)
			return 0;
		}
		else if (TKN.type == TOKEN_REASSIGN)
		{
			APPLY_NEXT_RULE(reassignment)
			return 0;
		}
		else if (TKN.type == TOKEN_COMMA)
		{
			APPLY_NEXT_RULE(list_of_vars)
			return 0;
		}
	}
	return ERR_SYNTAX;
}

static int statement(data_t *data)
{
	APPLY_RULE(condition)
	if (TKN.type == TOKEN_CURLY_OPEN)
	{
		EXPECT_NEXT_TOKEN(TOKEN_EOL)
		APPLY_NEXT_RULE(scope) //new scope if
		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_ELSE)
		{
			EXPECT_NEXT_TOKEN(TOKEN_CURLY_OPEN)
			EXPECT_NEXT_TOKEN(TOKEN_EOL)
			APPLY_NEXT_RULE(scope) //new scope else
			EXPECT_NEXT_TOKEN(TOKEN_EOL)
			return 0;
		}
	}
	return ERR_SYNTAX;
}

static int func_or_list_of_vars(data_t *data)
{
	if (TKN.type == TOKEN_PAR_OPEN && data->prev_token.type == TOKEN_IDENTIFIER) //func
	{
		APPLY_RULE(call_func)
		return 0;
	}
	else
	{
		if (!add_to_assign_list(data, data->prev_token)) 
			return ERR_INTERNAL;

		if (TKN.type == TOKEN_COMMA)
		{
			APPLY_NEXT_RULE(list_of_vars)
			return 0;
		}
		else if (TKN.type == TOKEN_ASSIGN && data->allow_assign)
		{
			APPLY_NEXT_RULE(assignment)
			return 0;
		}
		else if (TKN.type == TOKEN_REASSIGN)
		{
			APPLY_NEXT_RULE(reassignment)
			return 0;
		}
	}
	return ERR_SYNTAX;
}

static int cycle(data_t *data)
{
	APPLY_RULE(new_scope)

	if (TKN.type == TOKEN_SEMICOLON) //empty
	{
		APPLY_NEXT_RULE(condition)
		if (TKN.type == TOKEN_SEMICOLON)
		{
			APPLY_NEXT_RULE(end_of_cycle)
			APPLY_RULE(close_scope)
			return 0;
		}
	}
	else if (TKN.type == TOKEN_IDENTIFIER || (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE))
	{
		data->allow_assign = true;
		APPLY_RULE(list_of_vars)
		if (TKN.type == TOKEN_SEMICOLON)
		{
			APPLY_RULE(condition)
			if (TKN.type == TOKEN_SEMICOLON)
			{
				APPLY_NEXT_RULE(end_of_cycle)
				APPLY_RULE(close_scope)
				return 0;
			}
		}
	}
	return ERR_SYNTAX;
}

static int end_of_cycle(data_t *data)
{
	if (TKN.type == TOKEN_CURLY_OPEN) //empty
	{
		EXPECT_NEXT_TOKEN(TOKEN_EOL)
		APPLY_NEXT_RULE(scope)
		EXPECT_NEXT_TOKEN(TOKEN_EOL)
		return 0;
	}
	else if (TKN.type == TOKEN_IDENTIFIER)
	{
		data->allow_assign = false;
		APPLY_RULE(list_of_vars)
		if (TKN.type == TOKEN_CURLY_OPEN)
		{
			EXPECT_NEXT_TOKEN(TOKEN_EOL)
			APPLY_NEXT_RULE(scope)
			EXPECT_NEXT_TOKEN(TOKEN_EOL)
			return 0;
		}
	}
	return ERR_SYNTAX;
}

static int condition(data_t *data)
{
	var_data_t *aux1 = create_aux_var(data);
	data->vdata = aux1;	

	data->allow_func = false;
	APPLY_RULE(expression)
	switch (TKN.type)
	{
		case TOKEN_EQUAL: case TOKEN_NOT_EQUAL: 
		case TOKEN_LESS_OR_EQUAL: case TOKEN_LESS_THAN:
		case TOKEN_GREATER_OR_EQUAL: case TOKEN_GREATER_THAN:
			break;
		default:
			return ERR_SYNTAX;
	}

	var_data_t *aux2 = create_aux_var(data);
	data->vdata = aux2;

	data->allow_func = false;
	APPLY_NEXT_RULE(expression)

	if (compare_types(aux1->type, aux2->type) == '0')
		return ERR_SEMANTIC_TYPE_COMPAT;

	return 0;
}

static int returned_vals(data_t *data)
{
	if (TKN.type == TOKEN_EOL) //empty
	{
		if (data->fdata->ret_val_types.len != 0)
			return ERR_SEMANTIC_FUNC_PARAMS; //return too few variable
		return 0;
	}

	var_data_t *aux1 = create_aux_var(data);
	data->vdata = aux1;	

	data->allow_func = false;
	APPLY_RULE(expression)

	data->result = check_ret_vals(data, aux1->type, 0);
	CHECK_RESULT()

	if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		data->result = next_returned_val(data, 1);		
		CHECK_RESULT()
		return 0;
	}
	else if (TKN.type == TOKEN_EOL)
	{
		if (data->fdata->ret_val_types.len != 1)
			return ERR_SEMANTIC_FUNC_PARAMS; //return too few variable
		return 0;
	}
	return ERR_SYNTAX;
}

static int next_returned_val(data_t *data, unsigned int n)
{
	var_data_t *auxn = create_aux_var(data);
	data->vdata = auxn;	

	data->allow_func = false;
	APPLY_RULE(expression)

	data->result = check_ret_vals(data, auxn->type, n);
	CHECK_RESULT()

	if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		data->result = next_returned_val(data, n + 1);
		CHECK_RESULT()
		return 0;
	}
	else if (TKN.type == TOKEN_EOL)
	{
		if (data->fdata->ret_val_types.len != n)
			return ERR_SEMANTIC_FUNC_PARAMS; //return too few variable
		return 0;
	}
	return ERR_SYNTAX;
}

static int new_scope(data_t *data)
{
	stnode_ptr *local_scope = malloc(sizeof(stnode_ptr));
	if (local_scope == NULL)
		return ERR_INTERNAL;

	symtable_init(local_scope);
	if (!stack_push(&data->var_table, local_scope))
		return ERR_INTERNAL;

	data->allow_assign = true;

	int *zero = malloc(sizeof(int));
	*zero = 0;

	stack_push(&data->aux, zero);
	return 0;
}

static int close_scope(data_t *data)
{
	stack_pop(&data->aux, free);
	stack_pop(&data->var_table, free_local_scope);
	return 0;
}

bool is_inter_func(token token)
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

static char kw_to_char(keyword kw)
{
	if (kw == KW_INT)
		return 'i';
	else if (kw == KW_STRING)
		return 's';
	else if (kw == KW_FLOAT64)
		return 'f';
	else
		return '0';
}

static bool add_inter_func_to_table(data_t *data)
{
	bool err, res = true;
	stnode_ptr ptr;
	//inputs
	ptr = symtable_insert(&data->func_table, "inputs", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "si");
	if (!res) return false;
	//inputi
	ptr = symtable_insert(&data->func_table, "inputi", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "ii");
	if (!res) return false;
	//inputf
	ptr = symtable_insert(&data->func_table, "inputf", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "fi");
	if (!res) return false;
	//print
	ptr = symtable_insert(&data->func_table, "print", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "r");
	((func_data_t*)ptr->data)->nargs = true;
	if (!res) return false;
	//int2float
	ptr = symtable_insert(&data->func_table, "int2float", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "i");
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "f");
	if (!res) return false;
	//float2int
	ptr = symtable_insert(&data->func_table, "float2int", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "f");
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "i");
	if (!res) return false;
	//len
	ptr = symtable_insert(&data->func_table, "len", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "s");
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "i");
	if (!res) return false;
	//substr
	ptr = symtable_insert(&data->func_table, "substr", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "sii");
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "si");
	if (!res) return false;
	//ord
	ptr = symtable_insert(&data->func_table, "ord", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "si");
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "ii");
	if (!res) return false;
	//chr
	ptr = symtable_insert(&data->func_table, "chr", &err);
	if (ptr == NULL || !init_func_data(&ptr->data)) return false;
	res = res && str_add_const(&((func_data_t*)ptr->data)->args_types, "i");
	res = res && str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "si");
	if (!res) return false;
	return true;
}

static int check_func_calls(data_t *data)
{
	struct stack_el *elem = data->calls.top;
	while (elem != NULL)
	{
		func_call_data_t *fcd = (func_call_data_t*)elem->data;
		stnode_ptr ptr = symtable_search(data->func_table, fcd->func_name.str);
		if (ptr == NULL) //called funcion does not exist
			return ERR_SEMANTIC_UNDEF_REDEF;

		func_data_t *fdata = (func_data_t*)ptr->data;
		if (str_cmp_const(&fcd->func_name, "print") == 0)
		{
			if (str_cmp_const(&fcd->expected_return, "r") != 0)
				return ERR_SEMANTIC_FUNC_PARAMS;
		}
		else
		{
			if (!compare_list_of_types(fcd->args_types, fdata->args_types))
				return ERR_SEMANTIC_FUNC_PARAMS;

			if (!compare_list_of_types(fcd->expected_return, fdata->ret_val_types))
				return ERR_SEMANTIC_FUNC_PARAMS;
		}
		elem = elem->next;
	}
	return 0;
}

var_data_t* find_var(data_t *data, const char *name, bool local)
{
	struct stack_el *elem = data->var_table.top;
	while (elem != NULL)
	{
		stnode_ptr *bst = (stnode_ptr*)elem->data;
		stnode_ptr var_ptr = symtable_search(*bst, name);
		if (var_ptr != NULL) //var find in this scope
			return (var_data_t*)(var_ptr->data);
		else if (local)
			return NULL;
		elem = elem->next;
	}
	return NULL; //undefined variable
}

static char tkn_to_char(token token)
{
	if (token.type == TOKEN_INT)
		return 'i';
	else if (token.type == TOKEN_STRING)
		return 's';
	else if (token.type == TOKEN_FLOAT64)
		return 'f';
	else if (token.type == TOKEN_KEYWORD && token.attr.kw == KW_UNDERSCORE)
		return 't';
	return '0';
}

static bool add_to_assign_list(data_t *data, token token)
{	
	if (token.attr.kw == KW_UNDERSCORE)
	{
		dll_insert_last(data->assign_list, NULL);
	}
	else
	{
		var_data_t *vd = malloc(sizeof(var_data_t));
		if (vd == NULL)
			return false;

		str_init(&vd->name);
		str_add_const(&vd->name, token.attr.str->str);
		vd->type = tkn_to_char(token);
		dll_insert_last(data->assign_list, vd);	
	}
	return true;
}

static void set_return_types(data_t *data, dll_node_t *node)
{
	func_call_data_t *call = (func_call_data_t*)data->calls.top->data;
	if (node == NULL)
	{
		str_clear(&call->expected_return);
		dll_node_t *node = data->assign_list->first;
		while (node != NULL)
		{
			if (node->data == NULL)
			{
				str_add(&call->expected_return, 't');
			}
			else 
			{
				var_data_t *vd = (var_data_t*)node->data;
				var_data_t *fvd = find_var(data, vd->name.str, false);
				str_add(&call->expected_return, fvd->type);
			}
			node = node->next;
		}
	}
	else
	{
		if (node->data == NULL)
		{
			str_add(&call->expected_return, 't');
		}
		else 
		{
			var_data_t *vd = (var_data_t*)node->data;
			str_add(&call->expected_return, vd->type);
		}
	}
}

static var_data_t* create_aux_var(data_t *data)
{
	var_data_t *vd = malloc(sizeof(var_data_t));
	if (vd == NULL)
		return NULL;

	vd->type = 't';
	str_init(&vd->name);

	int n = *(int*)data->aux.top->data;
	int len = (6 + ((n - n % 2) / 10));
	char *name = malloc(sizeof(char) * len);
	sprintf(name, "@aux_%d", n);
	str_add_const(&vd->name, name);
	*(int*)data->aux.top->data += 1;

	//free(name); probably not needed ... causing seg fault
	return vd;
}

char compare_types(char a, char b)
{
	if (a == 't')
		return b;
	else if (b == 't')
		return a;
	else if (a == b)
		return b;
	return '0';	
}

static int check_ret_vals(data_t *data, char type, unsigned int n)
{	
	if (n > data->fdata->ret_val_types.len)
		return ERR_SEMANTIC_FUNC_PARAMS; //return too many vals

	char c = data->fdata->ret_val_types.str[n];
	char cmp = compare_types(type, c);
	if (cmp == c)
		return 0;
	
	return ERR_SEMANTIC_TYPE_COMPAT; //return bad data type
}

static bool compare_list_of_types(string expected, string sent)
{	
	if ((expected.len > 0 && expected.str[0] == 'r') //not assigning data from function
		/*(sent.len > 0 && sent.str[0] == 'r')*/)
		return true;

	if (expected.len != sent.len)
		return false; //bad argument count

	for (unsigned int i = 0; i < expected.len; i++)
	{
		if (compare_types(expected.str[i], sent.str[i]) == '0')
			return false;
	}

	return true;
}
