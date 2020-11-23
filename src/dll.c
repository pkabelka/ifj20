#include "dll.h"

dll_t* dll_init(dll_t *list) {
    dll_t *new_list = (dll_t*)malloc(sizeof(dll_t));
    if (new_list == NULL) {
        // error message
        return NULL;
    }

    new_list->first = NULL;
    new_list->last = NULL;
    new_list->size = 0;

    return new_list;
}

void join_lists(dll_t *main_list, dll_t *secondary_list) {
    if (main_list != NULL && secondary_list != NULL) {
        main_list->last->next = secondary_list->first;
        secondary_list->first->prev = main_list->last;
        main_list->last = secondary_list->last;
        free(secondary_list);
        secondary_list = NULL;
    }
    else {
        // error message
    }
}

void* dll_get(dll_t *list, int index) {
    if (index >= 0) {
        if (list != NULL) {
            if (list->size > index) {
                if(index == 0) return list->first->data;

                dll_node_t *tmp = list->first;
                for (int i = 0; i < index; i++) {
                    tmp = tmp->next;
                }
                return tmp->data;
            }
            else {
                //error message
            }
        }
        else {
            // error message
        }
    }
    else {
        // error message
    }
}

void* dll_get_first(dll_t *list) {
    if (list != NULL) {
        return list->first->data;
    }
}

void* dll_get_last(dll_t *list) {
    return dll_get(list, list->size - 1);
}

bool dll_insert(dll_t *list, int index, void *data) {
    if (index >= 0) {
        if (list != NULL) {
            if (list->size > index) {
                dll_node_t *new_node = (dll_node_t*)malloc(sizeof(dll_node_t));
                if (new_node == NULL) {
                    // error message
                    return false;
                }
                new_node->data = data;

                if (index == 0) {
                    return dll_insert_first(list, data);
                }
                else {
                    dll_node_t *tmp = list->first;
                    for (int i = 0; i < index; i++) {
                        tmp = tmp->next;
                    }
                    new_node->prev = tmp->prev
                    new_node->next = tmp;
                    tmp->prev = new_node;

                    if (list->size == index) {
                        list->last = new_node;
                    }
                }
                list->size++;

                return true;
            }
            else {
                // error message
            }
        }
        else {
            // error message
        }
    }
    else {
        // error message
    }
}

bool dll_insert_first(dll_t *list, void *data) {
    if (list != NULL) {
        dll_node_t *newNode = (dll_node_t*)malloc(sizeof(dll_node_t));
        if (new_node == NULL) {
            // error message
            return false;
        }
        new_node->data = data;
        new_node->prev = NULL;

        if (list->first == NULL) {
            new_node->next = NULL;
            list->first = new_node;
            list->last = new_node;
        }
        else {
            new_node->next = list->first;
            list->first->prev = new_node;
            list->first = new_node;
        }
        list->size++;

        return true;
    }
    else {
        // error message
        return false;
    }
}
bool dll_insert_last(dll_t*, void *);

bool dll_delete(dll_t*, int, void (delete_ptr*)(void*));
bool dll_delete_first(dll_t*, void (delete_ptr*)(void*));
bool dll_delete_last(dll_t*, void (delete_ptr*)(void*));

bool dll_dispose(dll_t*, void (delete_ptr*)(void*));