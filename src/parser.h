/**
 * @brief Parser interface - syntax and semantic analysis
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
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
} var_data_t;

/**
 * @brief Struct of some data needed for parsing
 * 
 */
typedef struct
{
	token prev_token, token;

	func_data_t *fdata; //current function data
	var_data_t *vdata; //current var data

	//for assignment
	dll_t *assign_list;
	int nassigns;

	stnode_ptr func_table; //BST of functins
	stack var_table;	   //symbol table for variables (stack of BSTs)
	stack calls;		   //stack of all function calls

	stack aux; //stack for counting auxiliary variables

	char current_type;
	bool allow_assign;
	bool allow_func;
	bool fix_call;
	int result;
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
