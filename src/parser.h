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
} data;

static int parse(data *data);
static int init_data(data *data);
static int dispose_data(data *data);
