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

dll_t* dllInit(dll_t*);
void joinLists(dll_t*, dll_t*);

void* dllGet(dll_t*, int);
void* dllGetFirst(dll_t*);
void* dllGetLast(dll_t*);

void dllInsert(dll_t*, int, int);
void dllInsertFirst(dll_t*, int);
void dllInsertLast(dll_t*, int);

void dllDelete(dll_t*, int, void (deletePtr*)(dll_node_t*));
void dllDeleteFirst(dll_t*, void (deletePtr*)(dll_node_t*));
void dllDeleteLast(dll_t*, void (deletePtr*)(dll_node_t*));

void dllDispose(dll_t*, void (deletePtr*)(dll_node_t*));
