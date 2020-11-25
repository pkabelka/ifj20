#ifndef _DLL_H
#define _DLL_H
#include <stdlib.h>
#include <stdbool.h>
typedef struct dll_node_t {
    void *data;
    struct dll_node_t *prev;
    struct dll_node_t *next;
} dll_node_t;

typedef struct dll_t {
    int size;
    dll_node_t *first;
    dll_node_t *last;
} dll_t;

dll_t* dll_init();
bool dll_join_lists(dll_t*, dll_t*);

void* dll_get(dll_t*, int);

bool dll_insert(dll_t*, int, void*);
bool dll_insert_first(dll_t*, void*);
bool dll_insert_last(dll_t*, void*);

bool dll_delete(dll_t*, int, void (*delete_ptr)(void*));
bool dll_delete_first(dll_t*, void (*delete_ptr)(void*));
bool dll_delete_last(dll_t*, void (*delete_ptr)(void*));

bool dll_clear(dll_t*, void (*delete_ptr)(void*));
bool dll_dispose(dll_t*, void (*delete_ptr)(void*));
#endif
