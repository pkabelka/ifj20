/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Parser interface - syntax and semantic analysis
 * 
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "scanner.h"
#include "symtable.h"
#include "stack.h"
#include "dll.h"

typedef struct
{
	string args_types;
	string ret_val_types;
	string name;
	bool used_return;
} func_data_t;

typedef struct
{
	string args_types;
	string func_name;
	string expected_return;
	int line;
} func_call_data_t;

typedef struct
{
	char type;
	string name;
	unsigned long scope_idx;
} var_data_t;

/**
 * @brief Struct of some data needed for parsing
 * 
 */
typedef struct
{
	token prev_token, token;

	func_data_t *fdata; //current function data
	var_data_t *vdata, *vdata2; //current var data

	//for assignment
	dll_t *assign_list;
	int nassigns;

	stnode_ptr func_table; //BST of functins
	stack var_table;	   //symbol table for variables (stack of BSTs)
	stack defvar_table;	   //symbol table for variables declarations (stack of BSTs)
	stack calls;		   //stack of all function calls

	bool used_relations;
	char current_type;
	bool allow_assign;
	bool allow_reassign;
	bool allow_func;
	bool allow_relations;
	bool fix_call;
	int result;
	unsigned long arg_idx;
	unsigned long label_idx;
	dll_t *arg_list;
	bool assign_func;
	bool assign_for;
	bool assign_for_swap_output;
	stack for_assign;
	unsigned long scope_idx;
} data_t;

/**
 * @brief Parse
 * 
 * @param data
 * @return int
 */
int parse(data_t *data);

/**
 * @brief Initialization of parser's data
 * 
 * @param data data for initialization
 * @return true if initialization succeded
 * @return false if initialization failed
 */
bool init_data(data_t *data);

/**
 * @brief Frees alocated memory
 * 
 * @param data data for dispose
 */
void dispose_data(data_t *data);

/**
 * @brief Finds variable in symbol table
 * 
 * @param data parser's data
 * @param str name of variable
 * @param local true for searching only in local scope
 * @return var_data_t* 
 */
var_data_t* find_var(data_t *data, const char *str, bool local);

/**
 * @brief Check if token is internal function
 * 
 * @param token 
 * @return true if token is inetnal function
 * @return false if token is not inetnal function
 */
bool is_inter_func(token token);

char compare_types(char a, char b);

#endif
