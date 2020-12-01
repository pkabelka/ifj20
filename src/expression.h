/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Interface for parsing expression
 * 
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 */

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "parser.h"

/**
 * @brief Parse expression
 * 
 * @param data
 * @return int 
 */
int expression(data_t* data);

#endif
