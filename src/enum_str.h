/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Enum to string conversion interface
 *
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#ifndef _ENUM_STR_H
#define _ENUM_STR_H

#include "error.h"
#include "scanner.h"

char *keyword_str(int e);
char *token_str(int e);

#endif
