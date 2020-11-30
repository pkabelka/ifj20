/**
 * @brief Parser implementation
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "parser.h"
#include "scanner.h"
#include "str.h"
#include "error.h"
#include "expression.h"
#include "enum_str.h"
#include "codegen.h"

#define NEXT_TOKEN() data->prev_token = data->token; if (get_next_token(&data->token) != SCANNER_SUCCESS) return ERR_LEX_STRUCTURE;
#define RET() return data->result;
#define APPLY_RULE_ERR(func) data->result = func(data); if (data->result != 0)
#define APPLY_NEXT_RULE_ERR(func) NEXT_TOKEN() APPLY_RULE_ERR(func)
#define CHECK_RESULT_ERR() if (data->result != 0)
#define CHECK_RESULT() CHECK_RESULT_ERR() RET()
#define APPLY_RULE(func) APPLY_RULE_ERR(func) RET()
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
func_call_data_t* create_func_call_data();
void free_func_data(void *ptr);
void free_local_scope(void *ptr);
void free_func_call_data(void *ptr);
void free_var_data(void *ptr);

bool init_data(data_t *data)
{
	data->current_type = '0';
	data->result = 0;
	data->arg_idx = 0;
	data->label_idx = 0;
	data->assign_func = false;
	data->assign_for = false;
	data->assign_for_swap_output = false;
	data->scope_idx = 0;

	stack_init(&data->for_assign);
	stack_init(&data->var_table);
	stack_init(&data->calls);
	stack_init(&data->aux);
	symtable_init(&data->func_table);

	data->assign_list = dll_init();
	data->arg_list = dll_init();
	if (data->assign_list == NULL || data->arg_list == NULL)
	{
		symtable_dispose(&data->func_table, free_func_data);
		stack_dispose(&data->calls, free_func_call_data);
		stack_dispose(&data->var_table, free_local_scope);
		stack_dispose(&data->aux, free);
		return false;
	}

	if (!add_inter_func_to_table(data))
	{
		dispose_data(data);
		return false;
	}

	return true;
}

void dispose_data(data_t *data)
{
	symtable_dispose(&data->func_table, free_func_data);
	stack_dispose(&data->calls, free_func_call_data);
	stack_dispose(&data->var_table, free_local_scope);
	stack_dispose(&data->aux, free);
	stack_dispose(&data->for_assign, free);
	dll_dispose(data->assign_list, stack_nofree);
	dll_dispose(data->arg_list, stack_nofree);
}

bool init_func_data(void **ptr)
{
	*ptr = malloc(sizeof(func_data_t));
	if (*ptr == NULL) return false;
	func_data_t **fd = (func_data_t**)ptr;

	if (!str_init(&(*fd)->args_types))
	{
		free(ptr);
		return false;
	}

	if (!str_init(&(*fd)->ret_val_types))
	{
		str_free(&(*fd)->args_types);
		free(ptr);
		return false;
	}

	if (!str_init(&(*fd)->name))
	{
		str_free(&(*fd)->args_types);
		str_free(&(*fd)->ret_val_types);
		return false;
	}

	(*fd)->used_return = false;
	return true;
}

func_call_data_t* create_func_call_data()
{
	func_call_data_t *fcd = malloc(sizeof(func_call_data_t));
	if (fcd == NULL) return NULL;

	if (!str_init(&fcd->args_types))
	{
		free(fcd);
		return NULL;
	}

	if (!str_init(&fcd->func_name))
	{
		str_free(&fcd->args_types);
		free(fcd);
		return NULL;
	}

	if (!str_init(&fcd->expected_return))
	{
		str_free(&fcd->args_types);
		str_free(&fcd->func_name);
		free(fcd);
		return NULL;
	}
	return fcd;
}

void free_func_call_data(void *ptr)
{
	func_call_data_t *fcd = (func_call_data_t*)ptr;
	str_free(&fcd->args_types);
	str_free(&fcd->func_name);
	str_free(&fcd->expected_return);
	free(ptr);
}

void free_func_data(void *ptr)
{
	func_data_t *fd = (func_data_t*)ptr;
	str_free(&fd->args_types);
	str_free(&fd->ret_val_types);
	str_free(&fd->name);
	free(ptr);
}

void free_local_scope(void *ptr)
{
	stnode_ptr *node = (stnode_ptr*)ptr;
	symtable_dispose(node, free_var_data);
	free(ptr);
}

void free_var_data(void *ptr)
{
	var_data_t *vd = (var_data_t*)ptr;
	str_free(&vd->name);
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

			GEN(gen_func_end, data->fdata->name.str);
			str_swap(&ifjcode20_output, &func_body);
			GEN(str_add_str, &ifjcode20_output, &func_declarations); // append function declarations
			GEN(str_add_str, &ifjcode20_output, &func_body); // append function body
			str_clear(&func_declarations);
			str_clear(&func_body);
			data->arg_idx = 0;
			data->label_idx = 0;
			data->scope_idx = 0;
			stack_dispose(&data->var_table, free_local_scope); // dispose all scopes at the end of a function

			if (!data->fdata->used_return)
				return ERR_SEMANTIC_OTHER;
		}
		else if (TKN.type != TOKEN_EOL && TKN.type != TOKEN_EOF)
			return ERR_SYNTAX;
	}

	if (symtable_search(data->func_table, "main") == NULL)
		return ERR_SEMANTIC_UNDEF_REDEF;

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
	if (ptr == NULL)
		return ERR_INTERNAL;

	if (!init_func_data(&ptr->data))
	{
		symtable_delete_node(&data->func_table, name, stack_nofree);
		return ERR_INTERNAL;
	}

	data->fdata = ptr->data;
	if (!str_add_const(&data->fdata->name, name))
		return ERR_INTERNAL;

	NEXT_TOKEN()
	if (TKN.type != TOKEN_PAR_OPEN) // func name(
		return ERR_SYNTAX;

	GEN(gen_func_begin, data->fdata->name.str);
	str_swap(&ifjcode20_output, &func_body);

	NEXT_TOKEN()
	if (TKN.type != TOKEN_PAR_CLOSE) //1+ args
	{
		APPLY_RULE(func_args)
	}
	
	NEXT_TOKEN()
	if (TKN.type == TOKEN_CURLY_OPEN) //start of body - empty return list
	{
		data->fdata->used_return = true; // no need for using return
		return 0;
	}
	else if (TKN.type == TOKEN_PAR_OPEN) //return vals
	{
		NEXT_TOKEN()
		if (TKN.type == TOKEN_PAR_CLOSE) //empty return list
			data->fdata->used_return = true; // no need for using return
		else
		{
			APPLY_RULE(func_return_vals)
		}
		NEXT_TOKEN()
		if (TKN.type == TOKEN_CURLY_OPEN) //start of body
			return 0;
	}
	else if (TKN.type == TOKEN_KEYWORD)
	{
		if (TKN.attr.kw == KW_INT || TKN.attr.kw == KW_STRING || TKN.attr.kw == KW_FLOAT64) //one returned value without ()
		{
			if (!str_add(&data->fdata->ret_val_types, kw_to_char(TKN.attr.kw)))
				return ERR_INTERNAL;
			GEN(gen_func_def_retval, data->fdata->ret_val_types.len-1, TKN.attr.kw);
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
		if (!str_add(&data->fdata->args_types, kw_to_char(TKN.attr.kw)))
			return ERR_INTERNAL;
		//create var
		var_data_t *vd = malloc(sizeof(var_data_t));
		if (vd == NULL)
			return ERR_INTERNAL;

		vd->type = kw_to_char(TKN.attr.kw);
		if (!str_init(&vd->name))
		{
			free(vd);
			return ERR_INTERNAL;
		}

		char *name = data->prev_token.attr.str->str;
		if (!str_add_const(&vd->name, name))
		{
			free_var_data(vd);
			return ERR_INTERNAL;
		}
		
		//add var into local scope
		bool err;
		stnode_ptr ptr = symtable_insert((stnode_ptr*)data->var_table.top->data, name, &err);
		if (ptr == NULL)
		{
			free_var_data(vd);
			return ERR_INTERNAL;
		}
		ptr->data = vd;

		GEN(gen_func_arg, vd->name.str, data->arg_idx);
		data->arg_idx++;

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
	GEN(gen_func_def_retval, data->fdata->ret_val_types.len-1, TKN.attr.kw);
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
	func_call_data_t *call = create_func_call_data();
	if (call == NULL)
		return ERR_INTERNAL;
	
	call->line = TKN.line;
	/*if (!str_add(&call->expected_return, 'r'))
	{
		free_func_call_data(call);
		return ERR_INTERNAL;
	}*/

	char *name;
	if (data->prev_token.type == TOKEN_KEYWORD)
		name = keyword_str(data->prev_token.attr.kw);
	else
		name = data->prev_token.attr.str->str;

	//checking if name of the function is used as variable name
	if (find_var(data, name, false) != NULL)
	{
		free_func_call_data(call);
		return ERR_SEMANTIC_UNDEF_REDEF;
	}

	if (!str_add_const(&call->func_name, name))
	{
		free_func_call_data(call);
		return ERR_INTERNAL;
	}

	if (!stack_push(&data->calls, call))
	{
		free_func_call_data(call); 
		return ERR_INTERNAL;
	}

	GEN(gen_create_frame);
	data->arg_idx = 0;
	APPLY_NEXT_RULE(func_calling)
	GEN(gen_func_call, call->func_name.str);
	return 0;
}

static int func_calling(data_t *data)
{
	if (TKN.type == TOKEN_IDENTIFIER)
	{
		if (strcmp(((func_call_data_t*)data->calls.top->data)->func_name.str, "print") == 0)
		{
			token *tmp_token = malloc(sizeof(token));
			tmp_token->type = TKN.type;
			string tmp_string;
			str_init(&tmp_string);
			tmp_token->attr.str = &tmp_string;
			str_copy(TKN.attr.str, tmp_token->attr.str);
			dll_insert_first(data->arg_list, tmp_token);
		}
		else
		{
			GEN(gen_func_call_arg, data->arg_idx++, &TKN);
		}

		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA || TKN.type == TOKEN_PAR_CLOSE) //variable as func parameter
		{
			var_data_t *vd = find_var(data, data->prev_token.attr.str->str, false);
			if (vd == NULL) //used undefined variable
				return ERR_SEMANTIC_UNDEF_REDEF;
			str_add(&((func_call_data_t*)data->calls.top->data)->args_types, vd->type);
			
			if (TKN.type == TOKEN_PAR_CLOSE)
			{
				NEXT_TOKEN();
				if (strcmp(((func_call_data_t*)data->calls.top->data)->func_name.str, "print") == 0)
				{
					dll_node_t *tmp = data->arg_list->first;
					while (tmp != NULL)
					{
						GEN(gen_func_arg_push, (token*)tmp->data, data->scope_idx);
						if (((token*)tmp->data)->type == TOKEN_STRING || ((token*)tmp->data)->type == TOKEN_IDENTIFIER)
						{
							str_free(((token*)tmp->data)->attr.str);
						}
						tmp = tmp->next;
					}
					token tmp_token;
					tmp_token.type = TOKEN_INT;
					tmp_token.attr.int_val = data->arg_list->size;
					GEN(gen_func_arg_push, &tmp_token, data->scope_idx);
					dll_dispose(data->arg_list, free);
					data->arg_list = dll_init();
				}
				return 0;
			}

			APPLY_NEXT_RULE(func_calling)
			return 0;
		}
	}
	else if (TKN.type == TOKEN_INT || TKN.type == TOKEN_STRING || TKN.type == TOKEN_FLOAT64)
	{
		if (strcmp(((func_call_data_t*)data->calls.top->data)->func_name.str, "print") == 0)
		{
			token *tmp_token = malloc(sizeof(token));
			tmp_token->type = TKN.type;
			string tmp_string;
			str_init(&tmp_string);
			tmp_token->attr.str = &tmp_string;
			switch (tmp_token->type)
			{
				case TOKEN_INT:
					tmp_token->attr.int_val = TKN.attr.int_val;
					str_free(&tmp_string);
					break;
				case TOKEN_FLOAT64:
					tmp_token->attr.float64_val = TKN.attr.float64_val;
					str_free(&tmp_string);
					break;
				case TOKEN_STRING:
					str_copy(TKN.attr.str, tmp_token->attr.str);
					break;
				default:
					break;
			}
			dll_insert_first(data->arg_list, tmp_token);
		}
		else
		{
			GEN(gen_func_call_arg, data->arg_idx++, &TKN);
		}

		NEXT_TOKEN()
		if (TKN.type == TOKEN_COMMA || TKN.type == TOKEN_PAR_CLOSE) //constants as func parameter
		{
			str_add(&((func_call_data_t*)data->calls.top->data)->args_types, tkn_to_char(data->prev_token));
			
			if (TKN.type == TOKEN_PAR_CLOSE)
			{
				if (strcmp(((func_call_data_t*)data->calls.top->data)->func_name.str, "print") == 0)
				{
					dll_node_t *tmp = data->arg_list->first;
					while (tmp != NULL)
					{
						if (((token*)tmp->data)->type == TOKEN_IDENTIFIER)
						{
							var_data_t *vd = find_var(data, ((token*)tmp->data)->attr.str->str, false);
							GEN(gen_func_arg_push, (token*)tmp->data, vd->scope_idx);
						}
						else
						{
							GEN(gen_func_arg_push, (token*)tmp->data, data->scope_idx);
						}
						// GEN(gen_func_arg_push, (token*)tmp->data, data->scope_idx);
						if (((token*)tmp->data)->type == TOKEN_STRING || ((token*)tmp->data)->type == TOKEN_IDENTIFIER)
						{
							str_free(((token*)tmp->data)->attr.str);
						}
						tmp = tmp->next;
					}
					token tmp_token;
					tmp_token.type = TOKEN_INT;
					tmp_token.attr.int_val = data->arg_list->size;
					GEN(gen_func_arg_push, &tmp_token, data->scope_idx);
					dll_dispose(data->arg_list, free);
					data->arg_list = dll_init();
				}
				NEXT_TOKEN();
				return 0;
			}

			APPLY_NEXT_RULE(func_calling)
			return 0;
		}
	}
	else if (TKN.type == TOKEN_PAR_CLOSE && data->prev_token.type != TOKEN_COMMA)
	{
		NEXT_TOKEN()
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
					EXPECT_TOKEN(TOKEN_EOL)
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
				data->fdata->used_return = true;
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

			// Check if the variable is declared in the same scope_idx but different scope
			bool declared = false;
			struct stack_el *elem = data->var_table.top;
			while (elem != NULL)
			{
				stnode_ptr bst = *(stnode_ptr*)elem->data;
				stnode_ptr var_ptr = symtable_search(bst, assign->name.str);
				if (var_ptr != NULL && var_ptr->data != NULL)
				{
					if (((var_data_t*)var_ptr->data)->scope_idx == assign->scope_idx)
					{
						declared = true;
						break;
					}
				}
				elem = elem->next;
			}

			if (!declared)
			{
				GEN(gen_defvar_str, assign->name.str, assign->scope_idx, &func_declarations);
			}

			assign->type = 't';
			ptr->data = assign;
		}
		node = node->next;
	}

	data->nassigns = 0;
	data->allow_func = false;
	data->fix_call = false;
	data->result = end_of_assignment(data, data->assign_list->first);
	CHECK_RESULT()

	dll_node_t *tmp = data->assign_list->first;
	unsigned long i = 0;
	while (tmp != NULL)
	{
		if (data->assign_func)
		{
			CODE_INT("PUSHS TF@%%retval"); CODE_NUM(i++); CODE_INT("\n");
		}
		if (data->vdata == NULL)
		{
			GEN(gen_pop, "%%void", "GF");
		}
		else
		{
			GEN(gen_pop_idx, data->vdata->name.str, "LF", data->vdata->scope_idx);
		}
		
		tmp = tmp->next;
	}

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

			free_var_data(assign); //var exists => free alocated var
			node->data = vd; //assign existed var to list
		}
		node = node->next;
	}

	data->nassigns = 0;
	data->allow_func = true;
	data->fix_call = false;
	data->result = end_of_assignment(data, data->assign_list->first);
	CHECK_RESULT()
	dll_node_t *tmp = data->assign_list->first;
	unsigned long i = 0;
	while (tmp != NULL)
	{
		if (data->assign_func)
		{
			CODE_INT("PUSHS TF@%%retval"); CODE_NUM(i++); CODE_INT("\n");
		}
		if (*((var_data_t*)tmp->data)->name.str == '_')
		{
			GEN(gen_pop, "%%void", "GF");
		}
		else
		{
			GEN(gen_pop_idx, ((var_data_t*)tmp->data)->name.str, "LF", ((var_data_t*)tmp->data)->scope_idx);
		}
		
		tmp = tmp->next;
	}
	if (data->assign_func)
	{
		data->assign_func = false;
	}
	if (data->assign_for)
	{
		str_swap(&ifjcode20_output, &for_assigns);
		data->assign_for = false;
		data->assign_for_swap_output = false;
		string *tmp_push = malloc(sizeof(string));
		str_init(tmp_push);
		str_copy(&for_assigns, tmp_push);
		stack_push(&data->for_assign, tmp_push);
		str_clear(&for_assigns);
	}

	dll_clear(data->assign_list, stack_nofree);
	return 0;
}

static int end_of_assignment(data_t *data, dll_node_t *node)
{	
	data->vdata = (var_data_t*)node->data;
	APPLY_RULE(expression)
	data->nassigns += 1;

	if (data->fix_call && data->nassigns == 2) //fix last call
	{
		data->fix_call = false;
		func_call_data_t *call = (func_call_data_t*)data->calls.top->data;
		char t = call->expected_return.str[0];
		str_clear(&call->expected_return);
		if (!str_add(&call->expected_return, t))
			return ERR_INTERNAL;
	}

	if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		if (node->next == NULL) //L < R
			return ERR_SEMANTIC_FUNC_PARAMS;

		if (TKN.type == TOKEN_EOL)
			return ERR_SYNTAX;

		return end_of_assignment(data, node->next); //assigning next value
	}
	else if (TKN.type == TOKEN_EOL ||
			TKN.type == TOKEN_SEMICOLON ||
			TKN.type == TOKEN_CURLY_OPEN)
	{
		if (data->nassigns != data->assign_list->size && !data->fix_call)
			return ERR_SEMANTIC_OTHER; //bad counts of parameter
		return 0;
	}
	else if (TKN.type == TOKEN_PAR_OPEN)
	{
		data->assign_func = true;
		APPLY_RULE(call_func)
		if (data->nassigns == 1)
		{
			set_return_types(data, NULL);
			data->fix_call = true;
		}
		else
			set_return_types(data, node);
		
		if (TKN.type == TOKEN_EOL)
		{
			if (data->nassigns != data->assign_list->size && !data->fix_call)
				return ERR_SEMANTIC_OTHER; //bad counts of parameter
			return 0;
		}
		
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
		unsigned long curr_idx = data->label_idx;
		data->label_idx++;
		GEN(gen_if_start, data->fdata->name.str, curr_idx);
		APPLY_NEXT_RULE(scope) //new scope if
		NEXT_TOKEN()
		if (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_ELSE)
		{
			EXPECT_NEXT_TOKEN(TOKEN_CURLY_OPEN)
			EXPECT_NEXT_TOKEN(TOKEN_EOL)
			GEN(gen_else, data->fdata->name.str, curr_idx);
			APPLY_NEXT_RULE(scope) //new scope else
			EXPECT_NEXT_TOKEN(TOKEN_EOL)
			GEN(gen_endif, data->fdata->name.str, curr_idx);
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
	unsigned long curr_idx = data->label_idx;
	data->label_idx++;
	if (TKN.type == TOKEN_SEMICOLON) //empty
	{
		GEN(gen_for_start, data->fdata->name.str, curr_idx);
		APPLY_NEXT_RULE(condition)
		GEN(gen_for_cond, data->fdata->name.str, curr_idx);
		if (TKN.type == TOKEN_SEMICOLON)
		{
			data->assign_for = true;
			data->assign_for_swap_output = true;
			APPLY_NEXT_RULE(end_of_cycle)
			APPLY_RULE(close_scope)

			if (data->for_assign.top != NULL)
			{
				str_add_const(&ifjcode20_output, ((string*)data->for_assign.top->data)->str);
				str_free((string*)data->for_assign.top->data);
				stack_pop(&data->for_assign, free);
			}
			GEN(gen_endfor, data->fdata->name.str, curr_idx);
			return 0;
		}
	}
	else if (TKN.type == TOKEN_IDENTIFIER || (TKN.type == TOKEN_KEYWORD && TKN.attr.kw == KW_UNDERSCORE))
	{
		data->allow_assign = true;
		APPLY_RULE(list_of_vars)
		GEN(gen_for_start, data->fdata->name.str, curr_idx);

		if (TKN.type == TOKEN_SEMICOLON)
		{
			APPLY_NEXT_RULE(condition)
			GEN(gen_for_cond, data->fdata->name.str, curr_idx);
			if (TKN.type == TOKEN_SEMICOLON)
			{
				data->assign_for = true;
				data->assign_for_swap_output = true;
				APPLY_NEXT_RULE(end_of_cycle)
				APPLY_RULE(close_scope)

				if (data->for_assign.top != NULL)
				{
					str_add_const(&ifjcode20_output, ((string*)data->for_assign.top->data)->str);
					str_free((string*)data->for_assign.top->data);
					stack_pop(&data->for_assign, free);
				}
				GEN(gen_endfor, data->fdata->name.str, curr_idx);
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
		data->assign_for = false;
		data->assign_for_swap_output = false;
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
	if (aux1 == NULL)
		return ERR_INTERNAL;
	data->vdata = aux1;	

	data->allow_func = false;
	APPLY_RULE_ERR(expression)
	{
		free_var_data(aux1);
		RET()
	}
	switch (TKN.type)
	{
		case TOKEN_EQUAL: case TOKEN_NOT_EQUAL: 
		case TOKEN_LESS_OR_EQUAL: case TOKEN_LESS_THAN:
		case TOKEN_GREATER_OR_EQUAL: case TOKEN_GREATER_THAN:
			break;
		default:
			return ERR_SYNTAX;
	}
	token_type rel_op = TKN.type;

	var_data_t *aux2 = create_aux_var(data);
	if (aux2 == NULL)
	{
		free_var_data(aux1);
		return ERR_INTERNAL;
	}
	data->vdata = aux2;

	data->allow_func = false;
	APPLY_NEXT_RULE_ERR(expression)
	{
		free_var_data(aux1);
		free_var_data(aux2);
		RET()
	}

	if (compare_types(aux1->type, aux2->type) == '0')
		return ERR_SEMANTIC_TYPE_COMPAT;

	free_var_data(aux1);
	free_var_data(aux2);

	switch (rel_op)
	{
		case TOKEN_EQUAL:
			CODE_INT("EQS\n");
			break;
		case TOKEN_NOT_EQUAL:
			CODE_INT("EQS\nNOTS\n");
			break;
		case TOKEN_LESS_THAN:
			CODE_INT("LTS\n");
			break;
		case TOKEN_LESS_OR_EQUAL:
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
		case TOKEN_GREATER_THAN:
			CODE_INT("GTS\n");
			break;
		case TOKEN_GREATER_OR_EQUAL:
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
	GEN(gen_pop, "%%res", "GF");

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
	if (aux1 == NULL)
		return ERR_INTERNAL;
	data->vdata = aux1;

	data->allow_func = false;
	APPLY_RULE_ERR(expression)
	{
		free_var_data(aux1);
		RET()
	}

	data->result = check_ret_vals(data, aux1->type, 0);
	CHECK_RESULT_ERR()
	{
		free_var_data(aux1);
		RET()
	}

	data->arg_idx = 0;
	GEN(gen_func_set_retval, data->arg_idx);
	data->arg_idx++;

	free_var_data(aux1);

	if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		data->result = next_returned_val(data, 1);		
		CHECK_RESULT()
		GEN(gen_func_return, data->fdata->name.str);
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
	if (auxn == NULL)
		return ERR_INTERNAL;
	data->vdata = auxn;	

	data->allow_func = false;
	APPLY_RULE_ERR(expression) 
	{
		free_var_data(auxn);
		RET()
	}
	GEN(gen_func_set_retval, data->arg_idx);
	data->arg_idx++;

	data->result = check_ret_vals(data, auxn->type, n);
	CHECK_RESULT_ERR()
	{
		free_var_data(auxn);
		RET()
	}

	free_var_data(auxn);

	if (TKN.type == TOKEN_COMMA)
	{
		NEXT_TOKEN()
		data->result = next_returned_val(data, n + 1);
		CHECK_RESULT()
		return 0;
	}
	else if (TKN.type == TOKEN_EOL)
	{
		if (data->fdata->ret_val_types.len != n + 1)
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

	var_data_t *assign = malloc(sizeof(var_data_t));
	str_init(&assign->name);
	str_add(&assign->name, '_');
	assign->type = 't';
	assign->scope_idx = data->scope_idx;
	bool err;
	stnode_ptr ptr = symtable_insert(local_scope, "_", &err);
	if (ptr == NULL || err)
		return ERR_INTERNAL;
	ptr->data = assign;

	if (!stack_push(&data->var_table, local_scope))
	{
		free(local_scope);
		return ERR_INTERNAL;
	}

	data->allow_assign = true;

	int *zero = malloc(sizeof(int));
	if (zero == NULL)
		return ERR_INTERNAL;
	*zero = 0;

	stack_push(&data->aux, zero);
	data->scope_idx++;
	return 0;
}

static int close_scope(data_t *data)
{
	stack_pop(&data->aux, free);
	// The next statement is commented because we don't want to free the
	// scope so we can check for variable declarations at the same scope index
	// stack_pop(&data->var_table, free_local_scope);
	data->scope_idx--;
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
	bool err;
	stnode_ptr ptr;
	//inputs
	ptr = symtable_insert(&data->func_table, "inputs", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "inputs", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "si")) return false;
	//inputi
	ptr = symtable_insert(&data->func_table, "inputi", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "inputi", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "ii")) return false;
	//inputf
	ptr = symtable_insert(&data->func_table, "inputf", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "inputf", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "fi")) return false;
	//print
	ptr = symtable_insert(&data->func_table, "print", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "print", stack_nofree);
		return false;
	}
	//int2float
	ptr = symtable_insert(&data->func_table, "int2float", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "int2float", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->args_types, "i")) return false;
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "f")) return false;
	//float2int
	ptr = symtable_insert(&data->func_table, "float2int", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "float2int", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->args_types, "f")) return false;
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "i")) return false;
	//len
	ptr = symtable_insert(&data->func_table, "len", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "len", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->args_types, "s")) return false;
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "i")) return false;
	//substr
	ptr = symtable_insert(&data->func_table, "substr", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "substr", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->args_types, "sii")) return false;
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "si")) return false;
	//ord
	ptr = symtable_insert(&data->func_table, "ord", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "ord", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->args_types, "si")) return false;
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "ii")) return false;
	//chr
	ptr = symtable_insert(&data->func_table, "chr", &err);
	if (ptr == NULL) return false;
	if (!init_func_data(&ptr->data)) 
	{
		symtable_delete_node(&data->func_table, "chr", stack_nofree);
		return false;
	}
	if (!str_add_const(&((func_data_t*)ptr->data)->args_types, "i")) return false;
	if (!str_add_const(&((func_data_t*)ptr->data)->ret_val_types, "si")) return false;

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
			if (fcd->expected_return.len != 0)
				return ERR_SEMANTIC_FUNC_PARAMS;
		}
		else
		{
			if (!compare_list_of_types(fcd->args_types, fdata->args_types)) //checking arguments
				return ERR_SEMANTIC_FUNC_PARAMS;

			if (!compare_list_of_types(fcd->expected_return, fdata->ret_val_types)) //checking returned types
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
	var_data_t *vd = malloc(sizeof(var_data_t));
	if (vd == NULL)
		return false;

	if (!str_init(&vd->name))
	{
		str_free(token.attr.str);
		free(vd);
		return false;
	}

	if (token.attr.kw == KW_UNDERSCORE)
	{
		if (!str_add(&vd->name, '_'))
		{
			str_free(token.attr.str);
			free_var_data(vd);
			return false;
		}
	}
	else
	{
		if (!str_add_const(&vd->name, token.attr.str->str))
		{
			str_free(token.attr.str);
			free_var_data(vd);
			return false;
		}
	}


	vd->type = tkn_to_char(token);
	vd->scope_idx = data->scope_idx;

	if (!dll_insert_last(data->assign_list, vd))
	{
		free_var_data(vd);
		return false;
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
	if (!str_init(&vd->name))
	{
		free(vd);
		return NULL;
	}

	int n = *(int*)data->aux.top->data;
	int len = (7 + ((n - n % 2) / 10));
	char *name = malloc(sizeof(char) * len);
	if (name == NULL)
	{
		free_var_data(vd);
		return NULL;
	}

	sprintf(name, "@aux_%d", n);
	if (!str_add_const(&vd->name, name))
	{
		free_var_data(vd);
		free(name);
		return NULL;
	}

	free(name);
	*(int*)data->aux.top->data += 1;
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
	
	return ERR_SEMANTIC_FUNC_PARAMS; //return bad data type
}

static bool compare_list_of_types(string expected, string sent)
{	
	/* if (expected.len == 0) //not assigning data from function
		return true;*/

	if (expected.len != sent.len)
		return false; //bad argument count

	for (unsigned int i = 0; i < expected.len; i++)
	{
		if (compare_types(expected.str[i], sent.str[i]) == '0')
			return false;
	}

	return true;
}
