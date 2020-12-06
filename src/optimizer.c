/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Optimizer implementation
 * 
 * @author Daniel Moudrý <xmoudr01 at stud.fit.vutbr.cz>
 */

#include "optimizer.h"
#include "error.h"
#include "dll.h"

static int copy_value(dll_node_t *dst, dll_node_t *src);
static dll_node_t* free_nodes(dll_node_t *operand_one, dll_node_t *operand_two, dll_node_t *current_node);

int optimize(data_t *data, dll_t *list) {
    dll_node_t *operand_one;
    dll_node_t *operand_two;
    symbol_t *symbol;
    o_type operator;

    symbol_type type;
    switch (data->current_type) {
        case 'i':
            type = SYM_INT;
            break;
        
        case 'f':
            type = SYM_FLOAT64;
            break;

        case 's':
            type = SYM_STRING;
            break;
    }


    dll_node_t *node = list->first;
    while (node != NULL) {
        symbol = (symbol_t*)node->data;
        if (symbol->sym_type == SYM_OPERATOR) {
            operator = *((o_type*)(symbol->data));
            if (operator == S_EQ || operator == S_NEQ || operator == S_GT || operator == S_GTE || operator == S_LT || operator == S_LTE) return 0;
            operand_two = node->prev;
            operand_one = operand_two->prev;

            // constant reduction
            if (((symbol_t*)operand_one->data)->sym_type == type && ((symbol_t*)operand_two->data)->sym_type == type) {
                if (type == SYM_INT || type == SYM_FLOAT64) {
                    if (type == SYM_INT) {
                        long *long_var_data = malloc(sizeof(long));
                        if (long_var_data == NULL) return ERR_INTERNAL;
                        switch (operator) {
                            case S_ADD:
                                *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) + *((long*)((symbol_t*)operand_two->data)->data);
                                break;
                            case S_SUB:
                                *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) - *((long*)((symbol_t*)operand_two->data)->data);
                                break;
                            case S_MUL:
                                *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) * *((long*)((symbol_t*)operand_two->data)->data);
                                break;
                            case S_DIV:
                                // zero division check
                                if (*((long*)((symbol_t*)operand_two->data)->data) == 0)
                                {
                                    free(long_var_data);
                                    return ERR_ZERO_DIVISION;
                                }
                                *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) / *((long*)((symbol_t*)operand_two->data)->data);
                                break;
                            default:
                                break;
                        }
                        ((symbol_t*)operand_one->data)->sym_type = SYM_INT;
                        free(((symbol_t*)operand_one->data)->data);
                        ((symbol_t*)operand_one->data)->data = long_var_data;
                    }
                    else if (type == SYM_FLOAT64) {
                        double *float_var_data = malloc(sizeof(double));
                        if (float_var_data == NULL) return ERR_INTERNAL;
                        switch (operator) {
                            case S_ADD:
                                *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) + *((double*)((symbol_t*)operand_two->data)->data);
                                break;
                            case S_SUB:
                                *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) - *((double*)((symbol_t*)operand_two->data)->data);
                                break;
                            case S_MUL:
                                *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) * *((double*)((symbol_t*)operand_two->data)->data);
                                break;
                            case S_DIV:
                                // zero division check
                                if (*((double*)((symbol_t*)operand_two->data)->data) == 0)
                                {
                                    free(float_var_data);
                                    return ERR_ZERO_DIVISION;
                                }
                                *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) / *((double*)((symbol_t*)operand_two->data)->data);
                                break;
                            default:
                                break;
                        }
                        ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                        free(((symbol_t*)operand_one->data)->data);
                        ((symbol_t*)operand_one->data)->data = float_var_data;
                    }

                    node = free_nodes(operand_one, operand_two, node);
                    continue;
                }
                else if (type == SYM_STRING) {
                    if (operator == S_ADD) {
                        string *string_var_data = malloc(sizeof(string));
                        if (string_var_data == NULL) return ERR_INTERNAL;
                        str_init(string_var_data);

                        str_add_const(string_var_data, ((string*)((symbol_t*)operand_one->data)->data)->str);
                        str_add_const(string_var_data, ((string*)((symbol_t*)operand_two->data)->data)->str);
                        ((symbol_t*)operand_one->data)->sym_type = SYM_STRING;
                        str_free((string*)(((symbol_t*)operand_one->data)->data));
                        free(((symbol_t*)operand_one->data)->data);
                        ((symbol_t*)operand_one->data)->data = string_var_data;

                        node = free_nodes(operand_one, operand_two, node);
                        continue;
                    }
                }
            }
            // optimize variables in certain situations
            else {
                if (((symbol_t*)operand_one->data)->sym_type == type && ((symbol_t*)operand_two->data)->sym_type == SYM_VAR) {
                    if (type == SYM_INT) {
                        if (*((long*)((symbol_t*)operand_one->data)->data) == 0) {
                            if (operator == S_MUL) { // 0 * x = 0
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                            else if (operator == S_ADD) { // 0 + x = x
                                if (copy_value(operand_one, operand_two) == ERR_INTERNAL) return ERR_INTERNAL;
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                        }
                        else if (*((long*)((symbol_t*)operand_one->data)->data) == 1) {
                            if (operator == S_MUL) { // 1 * x = x
                                if (copy_value(operand_one, operand_two) == ERR_INTERNAL) return ERR_INTERNAL;
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                        }
                    }
                    else if (type == SYM_FLOAT64) {
                        if (*((double*)((symbol_t*)operand_one->data)->data) == 0) {
                            if (operator == S_MUL) { // 0 * x = 0
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                            else if (operator == S_ADD) { // 0 + x = x
                                if (copy_value(operand_one, operand_two) == ERR_INTERNAL) return ERR_INTERNAL;
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                        }
                        else if (*((double*)((symbol_t*)operand_one->data)->data) == 1) {
                            if (operator == S_MUL) { // 1 * x = x
                                if (copy_value(operand_one, operand_two) == ERR_INTERNAL) return ERR_INTERNAL;
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                        }
                    }
                }
                else if (((symbol_t*)operand_two->data)->sym_type == type && ((symbol_t*)operand_one->data)->sym_type == SYM_VAR) {
                    if (type == SYM_INT) {
                        if (*((long*)((symbol_t*)operand_two->data)->data) == 0) {
                            if (operator == S_MUL) { // x * 0 = 0
                                if (copy_value(operand_one, operand_two) == ERR_INTERNAL) return ERR_INTERNAL;
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                            else if (operator == S_ADD || operator == S_SUB) { // x + 0 = x or x - 0 = x
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                            else if (operator == S_DIV){
                                return ERR_ZERO_DIVISION;
                            }
                        }
                        else if (*((long*)((symbol_t*)operand_two->data)->data) == 1) {
                            if (operator == S_MUL || operator == S_DIV) { // x * 1 = x or x / 1 = x
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                        }
                    }
                    else if (type == SYM_FLOAT64) {
                        if (*((double*)((symbol_t*)operand_two->data)->data) == 0) {
                            if (operator == S_MUL) {
                                if(copy_value(operand_one, operand_two) == ERR_INTERNAL) return ERR_INTERNAL;
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                            else if (operator == S_ADD || operator == S_SUB) {
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                            else if (operator == S_DIV) {
                                return ERR_ZERO_DIVISION;
                            }
                        }
                        else if (*((double*)((symbol_t*)operand_two->data)->data) == 1) {
                            if (operator == S_MUL || operator == S_DIV) { // x * 1 = x or x / 1 = x
                                node = free_nodes(operand_one, operand_two, node);
                                continue;
                            }
                        }
                    }
                }
            }
        }

        node = node->next;
    }

    return 0;
}

static int copy_value(dll_node_t *dst, dll_node_t *src) {
    switch (((symbol_t*)src->data)->sym_type) {
        case SYM_INT: ;
            long *long_var_data = malloc(sizeof(long));
            if (long_var_data == NULL) return ERR_INTERNAL;
            *long_var_data = *((long*)((symbol_t*)src->data)->data);

            ((symbol_t*)dst->data)->sym_type = SYM_INT;
            //free(((symbol_t*)dst->data)->data);
            ((symbol_t*)dst->data)->data = long_var_data;
            break;

        case SYM_FLOAT64: ;
            double *float_var_data = malloc(sizeof(double));
            if (float_var_data == NULL) return ERR_INTERNAL;
            *float_var_data = *((double*)((symbol_t*)src->data)->data);

            ((symbol_t*)dst->data)->sym_type = SYM_FLOAT64;
            //free(((symbol_t*)dst->data)->data);
            ((symbol_t*)dst->data)->data = float_var_data;
            break;

        case SYM_VAR: ;
            ((symbol_t*)dst->data)->sym_type = SYM_VAR;
            //free(((symbol_t*)dst->data)->data);
            ((symbol_t*)dst->data)->data = ((symbol_t*)src->data)->data;
            ((symbol_t*)src->data)->data = NULL;

        default:
            break;
    }

    return 0;
}

static dll_node_t* free_nodes(dll_node_t *operand_one, dll_node_t *operand_two, dll_node_t *current_node) {
    operand_one->next = current_node->next;
    if(current_node->next != NULL) current_node->next->prev = operand_one;
    free_symbol(((symbol_t*)operand_two->data));
    free_symbol(((symbol_t*)current_node->data));
    free(operand_two);
    free(current_node);
    return operand_one->next;
}