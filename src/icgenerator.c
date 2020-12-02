#include "icgenerator.h"

int optimize(dll_t *list, symbol_type type) {
    dll_node_t *operand_one;
    dll_node_t *operand_two;
    symbol_t *symbol;
    o_type operator;


    dll_node_t *node = list->first;
    while (node != NULL) {
        symbol = (symbol_t*)node->data;
        if (symbol->sym_type == SYM_OPERATOR) {
            operator = *((o_type*)(symbol->data));
            operand_two = node->prev;
            operand_one = operand_two->prev;

            // reduction of constants
            if (((symbol_t*)operand_one->data)->sym_type == type && ((symbol_t*)operand_two->data)->sym_type == type) {
                switch(operator) {
                    case S_ADD:
                        if (type == SYM_INT) {
                            long *long_var_data = malloc(sizeof(long));
                            *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) + *((long*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_INT;
                            ((symbol_t*)operand_one->data)->data = long_var_data;
                        }
                        else if (type == SYM_FLOAT64) {
                            double *float_var_data = malloc(sizeof(double));
                            *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) + *((double*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                            ((symbol_t*)operand_one->data)->data = float_var_data;
                        }
                        else if (type == SYM_STRING) {
                            string *string_var_data = malloc(sizeof(string));
                            str_init(string_var_data);
                            str_add_const(string_var_data, ((string*)((symbol_t*)operand_one->data)->data)->str);
                            str_add_const(string_var_data, ((string*)((symbol_t*)operand_two->data)->data)->str);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_STRING;
                            ((symbol_t*)operand_one->data)->data = string_var_data;
                        }
                        break;

                    case S_SUB:
                        if (type == SYM_INT) {
                            long *long_var_data = malloc(sizeof(long));
                            *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) - *((long*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_INT;
                            ((symbol_t*)operand_one->data)->data = long_var_data;
                        }
                        else if (type == SYM_FLOAT64) {
                            double *float_var_data = malloc(sizeof(double));
                            *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) - *((double*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                            ((symbol_t*)operand_one->data)->data = float_var_data;
                        }
                        break;

                    case S_MUL:
                        if (type == SYM_INT) {
                            long *long_var_data = malloc(sizeof(long));
                            *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) * *((long*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_INT;
                            ((symbol_t*)operand_one->data)->data = long_var_data;
                        }
                        else if (type == SYM_FLOAT64) {
                            double *float_var_data = malloc(sizeof(double));
                            *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) * *((double*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                            ((symbol_t*)operand_one->data)->data = float_var_data;
                        }
                        break;

                    case S_DIV:
                        if (type == SYM_INT) {
                            if (*((long*)((symbol_t*)operand_two->data)->data) == 0) {
                                // error
                                return FALSE;
                            }
                            long *long_var_data = malloc(sizeof(long));
                            *long_var_data = *((long*)((symbol_t*)operand_one->data)->data) / *((long*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_INT;
                            ((symbol_t*)operand_one->data)->data = long_var_data;
                        }
                        else if (type == SYM_FLOAT64) {
                            if (*((double*)((symbol_t*)operand_two->data)->data) == 0) {
                                // error
                                return FALSE;
                            }
                            double *float_var_data = malloc(sizeof(double));
                            *float_var_data = *((double*)((symbol_t*)operand_one->data)->data) / *((double*)((symbol_t*)operand_two->data)->data);
                            ((symbol_t*)operand_one->data)->sym_type = SYM_FLOAT64;
                            ((symbol_t*)operand_one->data)->data = float_var_data;
                        }
                        break;
                }
                if (type == SYM_STRING && operator != S_ADD) {
                    node = node->next;
                }
                else {
                    operand_one->next = node->next;
                    if(node->next != NULL) node->next->prev = operand_one;
                    free_symbol(((symbol_t*)operand_two->data)->data);
                    free_symbol(((symbol_t*)node->data)->data);
                    free(operand_two);
                    free(node);
                    node = operand_one->next;
                }
            }
            else {
                // check division by 0
                if (operator == S_DIV) {
                    if (type == SYM_INT) {
                        if (((symbol_t*)operand_two->data)->sym_type == type && *((long*)((symbol_t*)operand_two->data)->data) == 0) {
                            // error
                            return FALSE;
                        }
                    }
                    else if (type == SYM_FLOAT64) {
                        if (((symbol_t*)operand_two->data)->sym_type == type && *((double*)((symbol_t*)operand_two->data)->data) == 0) {
                            // error
                            return FALSE;
                        }
                    }
                }
                // simplification of given expression when variable is multiplied by 0
                else if (operator == S_MUL) {
                    if (type == SYM_INT) {
                        if ((((symbol_t*)operand_one->data)->sym_type == type && *((long*)((symbol_t*)operand_one->data)->data) == 0) || (((symbol_t*)operand_two->data)->sym_type == type && *((long*)((symbol_t*)operand_two->data)->data) == 0)) {
                            ((symbol_t*)operand_one->data)->data = 0;
                            operand_one->next = node->next;
                            if(node->next != NULL) node->next->prev = operand_one;
                            free_symbol(((symbol_t*)operand_two->data)->data);
                            free_symbol(((symbol_t*)node->data)->data);
                            free(operand_two);
                            free(node);
                            node = operand_one->next;
                        }
                    }
                    else if (type == SYM_FLOAT64) {
                        if ((((symbol_t*)operand_one->data)->sym_type == type && *((long*)((symbol_t*)operand_one->data)->data) == 0) || (((symbol_t*)operand_two->data)->sym_type == type && *((long*)((symbol_t*)operand_two->data)->data) == 0)) {
                            ((symbol_t*)operand_one->data)->data = 0;
                            operand_one->next = node->next;
                            if(node->next != NULL) node->next->prev = operand_one;
                            free_symbol(((symbol_t*)operand_two->data)->data);
                            free_symbol(((symbol_t*)node->data)->data);
                            free(operand_two);
                            free(node);
                            node = operand_one->next;
                        }
                    }
                }
            }
        }
        node = node->next;
    }
    
    return TRUE;
}






        /*switch (((symbol_t*)tmp->data)->sym_type) {
            case SYM_OPERATOR:
                if (last_type == previous_type) {
                    operator_type = *((o_type*)((symbol_t*)tmp->data)->data);
                    switch(operator_type) {
                        case S_ADD:
                            switch (last_type) {
                                case SYM_VAR:
                                    break;
                                case SYM_INT: ;
                                    long *long_var_data = malloc(sizeof(long));
                                    *long_var_data = *((long*)((symbol_t*)tmp->prev->prev->data)->data) + *((long*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_INT;
                                    ((symbol_t*)tmp->data)->data = long_var_data;
                                    break;
                                case SYM_FLOAT64: ;
                                    double *float_var_data = malloc(sizeof(double));
                                    *float_var_data = *((double*)((symbol_t*)tmp->prev->prev->data)->data) + *((double*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_FLOAT64;
                                    ((symbol_t*)tmp->data)->data = float_var_data;
                                    break;
                                case SYM_STRING: ;
                                    string *string_var_data = malloc(sizeof(string));
                                    str_init(string_var_data);
                                    str_add_const(string_var_data, ((string*)((symbol_t*)tmp->prev->prev->data)->data)->str);
                                    str_add_const(string_var_data, ((string*)((symbol_t*)tmp->prev->data)->data)->str);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_STRING;
                                    ((symbol_t*)tmp->data)->data = string_var_data;
                                    break;
                                default:
                                    break;
                            }
                            break;

                        case S_SUB:
                            switch (last_type) {
                                case SYM_VAR:
                                    break;
                                case SYM_INT: ;
                                    long *long_var_data = malloc(sizeof(long));
                                    *long_var_data = *((long*)((symbol_t*)tmp->prev->prev->data)->data) - *((long*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_INT;
                                    ((symbol_t*)tmp->data)->data = long_var_data;
                                    break;
                                case SYM_FLOAT64: ;
                                    double *float_var_data = malloc(sizeof(double));
                                    *float_var_data = *((double*)((symbol_t*)tmp->prev->prev->data)->data) - *((double*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_FLOAT64;
                                    ((symbol_t*)tmp->data)->data = float_var_data;
                                    break;
                                case SYM_STRING:
                                    break;
                                default:
                                    break;
                            }
                            break;

                        case S_MUL:
                            switch (last_type) {
                                case SYM_VAR: ;
                                    break;
                                case SYM_INT: ;
                                    long *long_var_data = malloc(sizeof(long));
                                    *long_var_data = *((long*)((symbol_t*)tmp->prev->prev->data)->data) * (*((long*)((symbol_t*)tmp->prev->data)->data));
                                    ((symbol_t*)tmp->data)->sym_type = SYM_INT;
                                    ((symbol_t*)tmp->data)->data = long_var_data;
                                    break;
                                case SYM_FLOAT64: ;
                                    double *float_var_data = malloc(sizeof(double));
                                    *float_var_data = *((double*)((symbol_t*)tmp->prev->prev->data)->data) * (*((double*)((symbol_t*)tmp->prev->data)->data));
                                    ((symbol_t*)tmp->data)->sym_type = SYM_FLOAT64;
                                    ((symbol_t*)tmp->data)->data = float_var_data;
                                    break;
                                case SYM_STRING:
                                    break;
                                default:
                                    break;
                            }
                            break;

                        case S_DIV:
                            switch (last_type) {
                                case SYM_VAR:
                                    break;
                                case SYM_INT:
                                    if (*((long*)((symbol_t*)tmp->prev->data)->data) == 0) return false;
                                    long *long_var_data = malloc(sizeof(long));
                                    *long_var_data = *((long*)((symbol_t*)tmp->prev->prev->data)->data) / *((long*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_INT;
                                    ((symbol_t*)tmp->data)->data = long_var_data;
                                    break;
                                case SYM_FLOAT64:
                                    if (*((double*)((symbol_t*)tmp->prev->data)->data) == 0) return false;
                                    double *float_var_data = malloc(sizeof(double));
                                    *float_var_data = *((double*)((symbol_t*)tmp->prev->prev->data)->data) / *((double*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_FLOAT64;
                                    ((symbol_t*)tmp->data)->data = float_var_data;
                                    break;
                                case SYM_STRING:
                                    break;
                                default:
                                    break;
                            }
                            break;
                    }
                    dll_delete(list, index - 1, free);
                    index--;
                    dll_delete(list, index - 1, free);
                    index--;
                }
                else {
                    previous_type = last_type;
                    last_type = SYM_OPERATOR;
                    index++;
                    tmp = tmp->next;
                }
                break;

            case SYM_VAR:
                previous_type = last_type;
                last_type = SYM_VAR;
                index++;
                tmp = tmp->next;
                break;
            case SYM_INT:
                previous_type = last_type;
                last_type = SYM_INT;
                index++;
                tmp = tmp->next;
                break;
            case SYM_FLOAT64:
                previous_type = last_type;
                last_type = SYM_FLOAT64;
                index++;
                tmp = tmp->next;
                break;
            case SYM_STRING:
                previous_type = last_type;
                last_type = SYM_STRING;
                index++;
                tmp = tmp->next;
                break;
        }
    }

    return TRUE;
}*/
