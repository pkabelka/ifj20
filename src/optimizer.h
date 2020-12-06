/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Optimizer interface
 * 
 * @author Daniel Moudrý <xmoudr01 at stud.fit.vutbr.cz>
 */

#ifndef _OPTIMIZER_H
#define _OPTIMIZER_H

#include "expression.h"

int optimize(data_t *data, dll_t *list);

#endif
