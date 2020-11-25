#include "dll.h"

dll_t* dll_init() {
    dll_t *new_list = (dll_t*)malloc(sizeof(dll_t));
    if (new_list == NULL) return NULL;

    new_list->first = NULL;
    new_list->last = NULL;
    new_list->size = 0;

    return new_list;
}

bool dll_join_lists(dll_t *main_list, dll_t *secondary_list) {
    if (main_list != NULL && secondary_list != NULL) {
        if (main_list->first == NULL)
        {
            main_list->first = secondary_list->first;
        }
        else
        {
            main_list->last->next = secondary_list->first;
        }
        
        if (secondary_list->first != NULL)
        {
            secondary_list->first->prev = main_list->last;
        }

        
        main_list->last = secondary_list->last;
        free(secondary_list);
        secondary_list = NULL;

        return true;
    }

    return false;
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
        }
    }

    return NULL;
}

bool dll_insert(dll_t *list, int index, void *data) {
    if (index >= 0) {
        if (index == 0) return dll_insert_first(list, data);
        if (list->size == index) return dll_insert_last(list, data);

        if (list != NULL) {
            if (list->size >= index) {
                dll_node_t *new_node = (dll_node_t*)malloc(sizeof(dll_node_t));
                if (new_node == NULL) {
                    // error message
                    return false;
                }
                new_node->data = data;

                dll_node_t *tmp = list->first;
                for (int i = 0; i < index; i++) {
                    tmp = tmp->next;
                }
                new_node->prev = tmp->prev;
                new_node->next = tmp;
                tmp->prev = new_node;
                list->size++;

                return true;
            }
        }
    }

    return false;
}

bool dll_insert_first(dll_t *list, void *data) {
    if (list != NULL) {
        dll_node_t *new_node = (dll_node_t*)malloc(sizeof(dll_node_t));
        if (new_node == NULL) return false;
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

    return false;
}
bool dll_insert_last(dll_t *list, void *data) {
    if (list != NULL) {
        dll_node_t *new_node = (dll_node_t*)malloc(sizeof(dll_node_t));
        if (new_node == NULL) return false;

        new_node->data = data;
        new_node->next = NULL;

        if (list->last == NULL) {
            new_node->prev = NULL;
            list->last = new_node;
            list->first = new_node;
        }
        else {
            new_node->prev = list->last;
            list->last->next = new_node;
            list->last = new_node;
        }
        list->size++;

        return true;
    }

    return false;
}

bool dll_delete(dll_t *list, int index, void (*delete_ptr)(void*)) {
    if (index >= 0) {
        if (index == 0) return dll_delete_first(list, delete_ptr);
        if (index == list->size - 1) return dll_delete_last(list, delete_ptr);

        if (list != NULL) {
            if (list->size > index) {
                dll_node_t *tmp = list->first;
                for (int i = 0; i < index; i++) {
                    tmp = tmp->next;
                }
                tmp->prev->next = tmp->next;
                tmp->next->prev = tmp->prev;
                delete_ptr(tmp->data);
                free(tmp);
                list->size--;

                return true;
            }
        }
    }

    return false;
}

bool dll_delete_first(dll_t *list, void (*delete_ptr)(void*)) {
    if (list != NULL) {
        if (list->first != NULL) {
            if (list->last == list->first) {
                delete_ptr(list->first->data);
                free(list->first);
                list->first = NULL;
                list->last = NULL;
            }
            else {
                dll_node_t *tmp = list->first;

                list->first = list->first->next;
                list->first->prev = NULL;
                delete_ptr(tmp->data);
                free(tmp);
            }
            list->size--;

            return true;
        }
    }

    return false;
}

bool dll_delete_last(dll_t *list, void (*delete_ptr)(void*)) {
    if (list != NULL) {
        if (list->last != NULL) {
            if (list->last == list->first) {
                delete_ptr(list->last->data);
                free(list->last);
                list->first = NULL;
                list->last = NULL;
            }
            else {
                dll_node_t *tmp = list->last;

                list->last = list->last->prev;
                list->last->next = NULL;
                delete_ptr(tmp->data);
                free(tmp);
            }
            list->size--;

            return true;
        }
    }
    
    return false;
}

bool dll_clear(dll_t *list, void (*delete_ptr)(void*)) {
    if (list != NULL) {
        if (list->first != NULL) {
            dll_node_t *to_delete;
            dll_node_t *tmp = list->first;
            while (tmp != NULL) {
                to_delete = tmp;
                tmp = tmp->next;

                delete_ptr(to_delete->data);
                free(to_delete);
            }
            list->first = NULL;
            list->last = NULL;
            list->size = 0;
            
            return true;
        }
    }

    return false;
}

bool dll_dispose(dll_t *list, void (*delete_ptr)(void*)) {
    if (dll_clear(list, delete_ptr) == true) {
        free(list);
        list = NULL;

        return true;
    }

    return false;
}
