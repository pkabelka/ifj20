/**
 * @brief Header of syntax analysis
 * @author Michael Škrášek <xskras01 at stud.fit.vutbr.cz>
 */

#include "scanner.h"
#include "symtable.h"
#include "stack.h"

/**
 * @brief Struct of some data needed for parsing
 * 
 */
typedef struct
{
	token prev_token, token;
	stack stack; // stack of brackets nested in function argument

	int result;
} data;

int parse(data *data);
int init_data(data *data);
int dispose_data(data *data);
