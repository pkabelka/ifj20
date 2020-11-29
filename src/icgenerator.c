#include "icgenerator.h"

bool generate_internal_code(dll_t *list) {
    symbol_type previous_type;
    symbol_type last_type;

    dll_node_t *tmp = list->first;

    for (int i = 0; i < list->size; i++) {
        switch (((symbol_t*)tmp->data)->sym_type) {
            case SYM_OPERATOR:
                if (last_type == previous_type) {
                    switch(*((o_type*)((symbol_t*)tmp->data)->data)) {
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
                                    /*string *string_var_data = malloc(sizeof(string));
                                    *string_var_data = *((char*)((symbol_t*)tmp->prev->prev->data)->data) + *((char*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_STRING;
                                    ((symbol_t*)tmp->data)->data = string_var_data;*/
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
                                    /*string *string_var_data = malloc(sizeof(string));
                                    *string_var_data = *((char*)((symbol_t*)tmp->prev->prev->data)->data) - *((char*)((symbol_t*)tmp->prev->data)->data);
                                    ((symbol_t*)tmp->data)->sym_type = SYM_STRING;
                                    ((symbol_t*)tmp->data)->data = string_var_data;*/
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
                }
                dll_delete(list, i - 2, free);
                dll_delete(list, i - 1, free);
                break;

            case SYM_VAR:
                previous_type = last_type;
                last_type = SYM_VAR;
                tmp = tmp->next;
                break;
            case SYM_INT:
                previous_type = last_type;
                last_type = SYM_INT;
                tmp = tmp->next;
                break;
            case SYM_FLOAT64:
                previous_type = last_type;
                last_type = SYM_FLOAT64;
                tmp = tmp->next;
                break;
            case SYM_STRING:
                previous_type = last_type;
                last_type = SYM_STRING;
                tmp = tmp->next;
                break;
        }
    }

    return true;
}
