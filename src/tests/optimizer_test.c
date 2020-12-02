#include <stdio.h>
#include "optimizer.h"
#include "dll.h"
#include "expression.h"
#include <stdlib.h>
#include "dll.c"
#include "optimizer.c"
#include "str.c"

int main() {
    dll_t *list = dll_init();

    symbol_t *symbol1 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol1_data = 25;
    long *symbol1_data_ptr = &symbol1_data;
    symbol1->sym_type = SYM_INT;
    symbol1->data = symbol1_data_ptr;

    symbol_t *symbol2 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol2_data = 5;
    long *symbol2_data_ptr = &symbol2_data;
    symbol2->sym_type = SYM_INT;
    symbol2->data = symbol2_data_ptr;

    symbol_t *symbol3 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator1 = S_SUB;
    symbol3->sym_type = SYM_OPERATOR;
    symbol3->data = &operator1;

    dll_insert_first(list, symbol3);
    dll_insert_first(list, symbol2);
    dll_insert_first(list, symbol1);

    dll_node_t *tmp = list->first;
    printf("\n");
    while (tmp != NULL) {
        switch (((symbol_t*)tmp->data)->sym_type) {
            case SYM_INT:
                printf("%ld ", *((long*)((symbol_t*)tmp->data)->data));
                break;

            case SYM_VAR:
                printf("var ");
                break;

            case SYM_FLOAT64:
                printf("%f ", *((double*)((symbol_t*)tmp->data)->data));
                break;

            case SYM_STRING:
                printf("%s ", ((string*)((symbol_t*)tmp->data)->data)->str);
                break;
            
            case SYM_OPERATOR:
                switch (*((o_type*)((symbol_t*)tmp->data)->data)) {
                    case S_ADD:
                        printf("+ ");
                        break;

                    case S_SUB:
                        printf("- ");
                        break;

                    case S_DIV:
                        printf("/ ");
                        break;

                    case S_MUL:
                        printf("* ");
                        break;
                        
                    default:
                        break;
                }
                break;
            
            default:
                break;
        }
        tmp = tmp->next;
    }

    if(optimize(list, SYM_INT) != 0) return 1;

    tmp = list->first;
    printf("\n\n");
    while (tmp != NULL) {
        switch (((symbol_t*)tmp->data)->sym_type) {
            case SYM_INT:
                printf("%ld ", *((long*)((symbol_t*)tmp->data)->data));
                break;

            case SYM_VAR:
                printf("var ");
                break;

            case SYM_FLOAT64:
                printf("%f ", *((double*)((symbol_t*)tmp->data)->data));
                break;

            case SYM_STRING:
                printf("%s ", ((string*)((symbol_t*)tmp->data)->data)->str);
                break;

            case SYM_OPERATOR:
                switch (*((o_type*)((symbol_t*)tmp->data)->data)) {
                    case S_ADD:
                        printf("+ ");
                        break;

                    case S_SUB:
                        printf("- ");
                        break;

                    case S_DIV:
                        printf("/ ");
                        break;

                    case S_MUL:
                        printf("* ");
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
        tmp = tmp->next;
    }

    return 0;
}
