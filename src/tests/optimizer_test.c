#include <stdio.h>
#include "icgenerator.h"
#include "dll.h"

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

    symbol_t *symbol4 = (symbol_t*)malloc(sizeof(symbol_t));
    var_data_t *symbol4_data_ptr = malloc(sizeof(var_data_t));
    symbol4->sym_type = SYM_VAR;
    symbol4->data = symbol4_data_ptr;

    symbol_t *symbol5 = (symbol_t*)malloc(sizeof(symbol_t));
    double symbol5_data = 10;
    double *symbol5_data_ptr = &symbol5_data;
    symbol5->sym_type = SYM_FLOAT64;
    symbol5->data = symbol5_data_ptr;

    symbol_t *symbol6 = (symbol_t*)malloc(sizeof(symbol_t));
    double symbol6_data = 100;
    double *symbol6_data_ptr = &symbol6_data;
    symbol6->sym_type = SYM_FLOAT64;
    symbol6->data = symbol6_data_ptr;

    symbol_t *symbol7 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator2 = S_MUL;
    symbol7->sym_type = SYM_OPERATOR;
    symbol7->data = &operator2;

    symbol_t *symbol8 = (symbol_t*)malloc(sizeof(symbol_t));
    var_data_t *symbol8_data_ptr = malloc(sizeof(var_data_t));
    symbol8->sym_type = SYM_VAR;
    symbol8->data = symbol8_data_ptr;

    symbol_t *symbol9 = (symbol_t*)malloc(sizeof(symbol_t));
    var_data_t *symbol9_data_ptr = malloc(sizeof(var_data_t));
    symbol9->sym_type = SYM_VAR;
    symbol9->data = symbol9_data_ptr;

    symbol_t *symbol10 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol10_data = 300;
    long *symbol10_data_ptr = &symbol10_data;
    symbol10->sym_type = SYM_INT;
    symbol10->data = symbol10_data_ptr;

    symbol_t *symbol11 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol11_data = 0;
    long *symbol11_data_ptr = &symbol11_data;
    symbol11->sym_type = SYM_INT;
    symbol11->data = symbol11_data_ptr;

    symbol_t *symbol12 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator3 = S_ADD;
    symbol12->sym_type = SYM_OPERATOR;
    symbol12->data = &operator3;

    symbol_t *symbol13 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol13_data = 10;
    long *symbol13_data_ptr = &symbol13_data;
    symbol13->sym_type = SYM_INT;
    symbol13->data = symbol13_data_ptr;

    symbol_t *symbol14 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator4 = S_ADD;
    symbol14->sym_type = SYM_OPERATOR;
    symbol14->data = &operator4;

    symbol_t *symbol15 = (symbol_t*)malloc(sizeof(symbol_t));
    double symbol15_data = 10;
    double *symbol15_data_ptr = &symbol15_data;
    symbol15->sym_type = SYM_FLOAT64;
    symbol15->data = symbol15_data_ptr;

    symbol_t *symbol16 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator5 = S_ADD;
    symbol16->sym_type = SYM_OPERATOR;
    symbol16->data = &operator5;

    symbol_t *symbol17 = (symbol_t*)malloc(sizeof(symbol_t));
    string *symbol17_data = (string*)malloc(sizeof(string));
    str_init(symbol17_data);
    const char* string1 = "hello";
    str_add_const(symbol17_data, string1);
    symbol17->sym_type = SYM_STRING;
    symbol17->data = symbol17_data;

    symbol_t *symbol18 = (symbol_t*)malloc(sizeof(symbol_t));
    string *symbol18_data = (string*)malloc(sizeof(string));
    str_init(symbol18_data);
    const char* string2 = "world";
    str_add_const(symbol18_data, string2);
    symbol18->sym_type = SYM_STRING;
    symbol18->data = symbol18_data;

    symbol_t *symbol19 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator6 = S_ADD;
    symbol19->sym_type = SYM_OPERATOR;
    symbol19->data = &operator6;

    symbol_t *symbol20 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol20_data = 50;
    long *symbol20_data_ptr = &symbol20_data;
    symbol20->sym_type = SYM_INT;
    symbol20->data = symbol20_data_ptr;

    symbol_t *symbol21 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol21_data = 500;
    long *symbol21_data_ptr = &symbol21_data;
    symbol21->sym_type = SYM_INT;
    symbol21->data = symbol21_data_ptr;

    symbol_t *symbol22 = (symbol_t*)malloc(sizeof(symbol_t));
    long symbol22_data = 350;
    long *symbol22_data_ptr = &symbol22_data;
    symbol22->sym_type = SYM_INT;
    symbol22->data = symbol22_data_ptr;

    symbol_t *symbol23 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator7 = S_SUB;
    symbol23->sym_type = SYM_OPERATOR;
    symbol23->data = &operator7;

    symbol_t *symbol24 = (symbol_t*)malloc(sizeof(symbol_t));
    o_type operator8 = S_ADD;
    symbol24->sym_type = SYM_OPERATOR;
    symbol24->data = &operator8;

    dll_insert_first(list, symbol24);
    dll_insert_first(list, symbol23);
    dll_insert_first(list, symbol22);
    dll_insert_first(list, symbol21);
    dll_insert_first(list, symbol20);
    dll_insert_first(list, symbol19);
    dll_insert_first(list, symbol18);
    dll_insert_first(list, symbol17);
    dll_insert_first(list, symbol16);
    dll_insert_first(list, symbol15);
    dll_insert_first(list, symbol14);
    dll_insert_first(list, symbol13);
    dll_insert_first(list, symbol12);
    dll_insert_first(list, symbol11);
    dll_insert_first(list, symbol10);
    dll_insert_first(list, symbol9);
    dll_insert_first(list, symbol8);
    dll_insert_first(list, symbol7);
    dll_insert_first(list, symbol6);
    dll_insert_first(list, symbol5);
    dll_insert_first(list, symbol4);
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
                }
                break;
        }
        tmp = tmp->next;
    }

    if(!generate_internal_code(list)) return 1;

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
                }
                break;
        }
        tmp = tmp->next;
    }

    return 0;
}
