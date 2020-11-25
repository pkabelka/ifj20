/**
 * @brief Enum to string conversion interface
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#ifndef _ENUM_STR_H
#define _ENUM_STR_H

#include "error.h"
#include "scanner.h"

char *keyword_str(int e);
char *token_str(int e);

#endif
