/**
 * @brief Code generator interface
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "str.h"
#include "stdbool.h"
#include "error.h"
#include "scanner.h"

/**
 * @brief Adds string to the output code string
 * @return false if there was an error
 */
#define CODE(...) if(str_add_var(&ifjcode20_output, __VA_ARGS__, NULL)!=true)return false

/**
 * @brief Adds string to the output code string
 * @return ERR_INTERNAL if there was an error
 */
#define CODE_INT(...) if(str_add_var(&ifjcode20_output, __VA_ARGS__, NULL)!=true)return ERR_INTERNAL

/**
 * @brief Adds int to the output code string
 * @return false if there was an error
 */
#define CODE_NUM(_VAL) do{char str[20*sizeof(char)]; sprintf(str, "%ld", _VAL); CODE(str);}while(0)

/**
 * @brief Calls _FUNC with arguments passed
 * @return ERR_INTERNAL if there was an error
 */
#define GEN(_FUNC, ...) if(_FUNC(__VA_ARGS__)==false)return ERR_INTERNAL

/**
 * @brief Calls _FUNC with arguments passed
 * @return false if there was an error
 */
#define GEN_BOOL(_FUNC, ...) if(_FUNC(__VA_ARGS__)==false)return false

extern string ifjcode20_output;
extern string for_assigns;
extern string func_declarations;
extern string func_body;

bool gen_codegen_init();
bool gen_codegen_output();
bool gen_output_header();
bool gen_output_eof();
bool gen_main_begin();
bool gen_main_end();
bool gen_generate_function_return(char *function_id);
bool gen_func_begin(const char *id);
bool gen_func_def_retval(unsigned long idx, keyword kw);
bool gen_func_set_retval(unsigned long idx);
bool gen_defvar(char *id);
bool gen_defvar_str(char *id, unsigned long idx, string *str);
bool gen_pop(char *arg_id, char *frame);
bool gen_pop_idx(char *id, char *frame, unsigned long idx);
bool gen_get_retval(char *id, char *frame, unsigned long idx);
bool gen_func_arg(char *arg_id, unsigned long idx);
bool gen_func_call(const char *id);
bool gen_token_value(token *tok);
bool gen_func_call_arg(unsigned long idx, token *tok);
bool gen_func_arg_push(token *tok, unsigned long idx);
bool gen_func_return(const char *id);
bool gen_func_end(const char *id);
bool gen_create_frame();
bool gen_label(const char *id, unsigned long idx, unsigned long depth);
bool gen_if_start(const char *id, unsigned long idx);
bool gen_else(const char *id, unsigned long idx);
bool gen_endif(const char *id, unsigned long idx);
bool gen_for_start(const char *id, unsigned long idx);
bool gen_for_cond(const char *id, unsigned long idx);
bool gen_endfor(const char *id, unsigned long idx);
bool gen_builtin_functions();

#endif