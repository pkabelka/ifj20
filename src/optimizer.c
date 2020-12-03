#include "optimizer.h"
#include "error.h"
#include "dll.h"

int optimize(dll_t *list, data_t *data) {
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
                                if (*((long*)((symbol_t*)operand_two->data)->data) == 0) return ERR_ZERO_DIVISION;
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
                                if (*((double*)((symbol_t*)operand_two->data)->data) == 0) return ERR_ZERO_DIVISION;
                                *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) / *((double*)((symbol_t*)operand_two->data)->data);
                                break;
                            default:
                                break;
                        }
                        ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                        free(((symbol_t*)operand_one->data)->data);
                        ((symbol_t*)operand_one->data)->data = float_var_data;
                    }

                    //free nodes
                    operand_one->next = node->next;
                    if(node->next != NULL) node->next->prev = operand_one;
                    free_symbol(((symbol_t*)operand_two->data));
                    free_symbol(((symbol_t*)node->data));
                    free(operand_two);
                    free(node);
                    node = operand_one->next;
                }
                else if (type == SYM_STRING) {
                    if (operator == S_ADD) {
                        string *string_var_data = malloc(sizeof(string));
                        if (string_var_data == NULL) return ERR_INTERNAL;
                        str_init(string_var_data);

                        str_add_const(string_var_data, ((string*)((symbol_t*)operand_one->data)->data)->str);
                        str_add_const(string_var_data, ((string*)((symbol_t*)operand_two->data)->data)->str);
                        ((symbol_t*)operand_one->data)->sym_type = SYM_STRING;
                        free(((symbol_t*)operand_one->data)->data);
                        ((symbol_t*)operand_one->data)->data = string_var_data;

                        //free nodes
                        operand_one->next = node->next;
                        if(node->next != NULL) node->next->prev = operand_one;
                        free_symbol(((symbol_t*)operand_two->data));
                        free_symbol(((symbol_t*)node->data));
                        free(operand_two);
                        free(node);
                        node = operand_one->next;
                    }
                    else {
                        node = node->next;
                    }
                }
            }
            // optimize multiplying variable by 0
            else {
                if (((symbol_t*)operand_one->data)->sym_type == type && ((symbol_t*)operand_two->data)->sym_type == SYM_VAR) {
                    if (type == SYM_INT) {
                        if (*((long*)((symbol_t*)operand_one->data)->data) == 0) {
                            if (operator == S_MUL) {
                                long *long_var_data = malloc(sizeof(long));
                                if (long_var_data == NULL) return ERR_INTERNAL;

                                *long_var_data = 0;
                                ((symbol_t*)operand_one->data)->sym_type = SYM_INT;
                                free(((symbol_t*)operand_one->data)->data);
                                ((symbol_t*)operand_one->data)->data = long_var_data;

                                //free nodes
                                operand_one->next = node->next;
                                if(node->next != NULL) node->next->prev = operand_one;
                                free_symbol(((symbol_t*)operand_two->data));
                                free_symbol(((symbol_t*)node->data));
                                free(operand_two);
                                free(node);
                                node = operand_one->next;
                            }
                            else {
                                node = node->next;
                            }
                        }
                        else {
                            node = node->next;
                        }
                    }
                    else if (type == SYM_FLOAT64) {
                        if (*((double*)((symbol_t*)operand_one->data)->data) == 0) {
                            if (operator == S_MUL) {
                                double *float_var_data = malloc(sizeof(double));
                                if (float_var_data == NULL) return ERR_INTERNAL;

                                *float_var_data = 0;
                                ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                                free(((symbol_t*)operand_one->data)->data);
                                ((symbol_t*)operand_one->data)->data = float_var_data;

                                //free nodes
                                operand_one->next = node->next;
                                if(node->next != NULL) node->next->prev = operand_one;
                                free_symbol(((symbol_t*)operand_two->data));
                                free_symbol(((symbol_t*)node->data));
                                free(operand_two);
                                free(node);
                                node = operand_one->next;
                            }
                            else {
                                node = node->next;
                            }
                        }
                        else {
                            node = node->next;
                        }
                    }
                    else {
                        node = node->next;
                    }
                }
                else if (((symbol_t*)operand_two->data)->sym_type == type && ((symbol_t*)operand_one->data)->sym_type == SYM_VAR) {
                    if (type == SYM_INT) {
                        if (*((long*)((symbol_t*)operand_two->data)->data) == 0) {
                            if (operator == S_MUL) {
                                long *long_var_data = malloc(sizeof(long));
                                if (long_var_data == NULL) return ERR_INTERNAL;

                                *long_var_data = 0;
                                ((symbol_t*)operand_two->data)->sym_type = SYM_INT;
                                free(((symbol_t*)operand_one->data)->data);
                                ((symbol_t*)operand_two->data)->data = long_var_data;

                                //free nodes
                                operand_two->next = node->next;
                                if(node->next != NULL) node->next->prev = operand_two;
                                free_symbol(((symbol_t*)operand_two->data));
                                free_symbol(((symbol_t*)node->data));
                                free(operand_two);
                                free(node);
                                node = operand_one->next;
                            }
                            else if (operator == S_DIV){
                                return ERR_ZERO_DIVISION;
                            }
                            else {
                                node = node->next;
                            }
                        }
                        else {
                            node = node->next;
                        }
                    }
                    else if (type == SYM_FLOAT64) {
                        if (*((double*)((symbol_t*)operand_two->data)->data) == 0) {
                            if (operator == S_MUL) {
                                double *float_var_data = malloc(sizeof(double));
                                if (float_var_data == NULL) return ERR_INTERNAL;

                                *float_var_data = 0;
                                ((symbol_t*)operand_two->data)->sym_type = SYM_FLOAT64;
                                free(((symbol_t*)operand_one->data)->data);
                                ((symbol_t*)operand_two->data)->data = float_var_data;

                                //free nodes
                                operand_two->next = node->next;
                                if(node->next != NULL) node->next->prev = operand_two;
                                free_symbol(((symbol_t*)operand_two->data));
                                free_symbol(((symbol_t*)node->data));
                                free(operand_two);
                                free(node);
                                node = operand_one->next;
                            }
                            else if (operator == S_DIV) {
                                return ERR_ZERO_DIVISION;
                            }
                            else {
                                node = node->next;
                            }
                        }
                        else {
                            node = node->next;
                        }
                    }
                    else {
                        node = node->next;
                    }
                }
                else {
                    node = node->next;
                }
            }
        }
        else {
            node = node->next;
        }
    }

    return 0;
}