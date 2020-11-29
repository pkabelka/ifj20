/**
 * @brief Code generator implementation
 *
 * @author Petr Kabelka <xkabel09@stud.fit.vutbr.cz>
 */

#include "codegen.h"

string ifjcode20_output;
string for_assigns;
string func_declarations;
string func_body;

bool gen_output_header()
{
    CODE(".IFJcode20\n"\
"DEFVAR GF@%%res\n"\
"MOVE GF@%%res int@0\n"\
"DEFVAR GF@%%tmp0\n"\
"MOVE GF@%%tmp0 bool@false\n"\
"DEFVAR GF@%%tmp1\n"\
"MOVE GF@%%tmp1 int@0\n"\
"DEFVAR GF@%%tmp2\n"\
"MOVE GF@%%tmp2 int@0\n"\
"JUMP $$main\n");
    GEN_BOOL(gen_builtin_functions);
    return true;
    }

bool gen_output_eof()
{
    // CODE("LABEL $$runerr\n",
    //     "WRITE string@Runtime\\032error!\\010Encountered\\032a\\032function\\032without\\032a\\032return\\032statement\\010\n",
    //     "EXIT int@6\n");
    CODE("LABEL $$EOF\n");
    return true;
}

bool gen_codegen_init()
{
    GEN_BOOL(str_init, &ifjcode20_output);
    GEN_BOOL(gen_output_header);
    return true;
}

bool gen_codegen_output()
{
    GEN_BOOL(gen_output_eof);
    if(fprintf(stdout, ifjcode20_output.str)<0)return false;
    if(fflush(stdout)!=0)return false;
    return true;
}

bool gen_func_begin(const char *id)
{
    if (strcmp(id, "main") == 0)
    {
        CODE("\nLABEL $$main\n"\
            "CREATEFRAME\n"\
            "PUSHFRAME\n");
        return true;
    }

    CODE("\nLABEL $", id, "\n"\
        "PUSHFRAME\n");
    return true;
}

bool gen_func_def_retval(unsigned long idx, keyword kw)
{
    CODE("DEFVAR LF@%%retval"); CODE_NUM(idx); CODE("\n");
    switch (kw)
    {
    case KW_INT:
        CODE("MOVE LF@%%retval"); CODE_NUM(idx); CODE(" int@0\n");
        break;
    case KW_FLOAT64:
        CODE("MOVE LF@%%retval"); CODE_NUM(idx); CODE(" float@0x0p+0\n");
        break;
    case KW_STRING:
        CODE("MOVE LF@%%retval"); CODE_NUM(idx); CODE(" string@\n");
        break;
    default:
        break;
    }
    return true;
}

bool gen_func_set_retval(unsigned long idx)
{
    // CODE("MOVE LF@%%retval"); CODE_NUM(idx); CODE(" GF@%%res\n");
    CODE("POPS LF@%%retval"); CODE_NUM(idx); CODE("\n");
    return true;
}

bool gen_defvar(char *id)
{
    CODE("DEFVAR LF@", id, "\n");
    return true;
}

bool gen_defvar_str(char *id, unsigned long idx, string *s)
{
    str_swap(&ifjcode20_output, s);
    CODE("DEFVAR LF@", id, "%%"); CODE_NUM(idx); CODE("\n"); // DEFVAR LF@id%idx
    str_swap(&ifjcode20_output, s);
    return true;
}

bool gen_pop(char *id, char *frame)
{
    CODE("POPS ", frame, "@", id, "\n");
    return true;
}

bool gen_pop_idx(char *id, char *frame, unsigned long idx)
{
    CODE("POPS ", frame, "@", id, "%%"); CODE_NUM(idx); CODE("\n");
    return true;
}

bool gen_get_retval(char *id, char *frame, unsigned long idx)
{
    CODE("MOVE ", frame, "@", id, " TF@%%retval"); CODE_NUM(idx); CODE("\n");
    return true;
}

bool gen_func_arg(char *arg_id, unsigned long idx)
{
    CODE("DEFVAR LF@", arg_id, "\n"); // DEFVAR LF@id
    CODE("MOVE LF@", arg_id, " LF@%%"); CODE_NUM(idx); CODE("\n"); // MOVE LF@id LF@%idx
    return true;
}

bool gen_func_call(const char *id) { CODE("CALL $", id, "\n"); return true; }

bool gen_token_value(token *tok)
{
    string tmp;
    GEN_BOOL(str_init, &tmp);
    char str[20*sizeof(char)];
    char c;

    switch (tok->type)
    {
        case TOKEN_STRING:
            for (int i = 0; (c = tok->attr.str->str[i]) != '\0'; i++)
            {
                if (c == '\\' || isprint(c) == 0 || c == '#' || c <= 32)
                {
                    // c as ASCII value in \000 format
                    str_add(&tmp, '\\');
                    sprintf(str, "%03d", c);
                    str_add_const(&tmp, str);
                }
                else
                {
                    str_add(&tmp, c);
                }
            }
            CODE("string@"); CODE(tmp.str); // string@text
            break;
        case TOKEN_IDENTIFIER:
            CODE("LF@"); CODE(tok->attr.str->str); // LF@id
            break;
        case TOKEN_INT:
            CODE("int@"); CODE_NUM(tok->attr.int_val); // int@int_val
            break;
        case TOKEN_FLOAT64:
            CODE("float@"); sprintf(str, "%a", tok->attr.float64_val); CODE(str); // float@hex_float
            break;
        default:
            str_free(&tmp);
            return false;
    }
    str_free(&tmp);
    return true;
}

bool gen_func_call_arg(unsigned long idx, token *tok)
{
    CODE("DEFVAR TF@%%"); CODE_NUM(idx); CODE("\n"); // DEFVAR TF@idx
    CODE("MOVE TF@%%"); CODE_NUM(idx); CODE(" "); GEN(gen_token_value, tok); CODE("\n"); // MOVE TF@idx type@value
    return true;
}

bool gen_func_arg_push(token *tok, unsigned long idx)
{
    CODE("PUSHS "); GEN(gen_token_value, tok); // PUSHS type@value
    if (tok->type == TOKEN_IDENTIFIER)
    {
        CODE("%%"); CODE_NUM(idx);
    }
    CODE("\n");
    return true;
}

bool gen_func_return(const char *id) { CODE("JUMP $", id, "$return\n"); return true; }

bool gen_func_end(const char *id)
{
    if (strcmp(id, "main") == 0)
    {
        CODE("CLEARS\n"\
            "JUMP $$EOF\n");
        return true;
    }

    CODE("LABEL $", id, "$return\n", // LABEL $id$return
        "POPFRAME\n"\
        "RETURN\n");
    return true;
}

bool gen_create_frame() { CODE("CREATEFRAME\n"); return true; }

bool gen_label(const char *id, unsigned long idx, unsigned long depth)
{
    CODE("LABEL $", id, "$"); CODE_NUM(idx); CODE("$"); CODE_NUM(depth); CODE("\n"); // LABEL $id$idx$depth
    return true;
}

bool gen_if_start(const char *id, unsigned long idx)
{
    CODE("JUMPIFNEQ $", id, "$"); CODE_NUM(idx); CODE("$else GF@%%res bool@true\n"); // JUMPIFNEQ $id$idx$else GF@%%res bool@true
    return true;
}

bool gen_else(const char *id, unsigned long idx)
{
    CODE("JUMP $", id, "$"); CODE_NUM(idx); CODE("$endif\n"); // JUMP $id$endif
    CODE("LABEL $", id, "$"); CODE_NUM(idx); CODE("$else"); CODE("\n"); // LABEL $id$else
    return true;
}

bool gen_endif(const char *id, unsigned long idx)
{
    CODE("LABEL $", id, "$"); CODE_NUM(idx); CODE("$endif"); CODE("\n"); // LABEL $id$endif
    return true;
}

bool gen_for_start(const char *id, unsigned long idx)
{
    CODE("LABEL $", id, "$"); CODE_NUM(idx); CODE("$for"); CODE("\n"); // LABEL $id$for
    return true;
}

bool gen_for_cond(const char *id, unsigned long idx)
{
    CODE("JUMPIFNEQ $", id, "$"); CODE_NUM(idx); CODE("$endfor GF@%%res bool@true\n"); // JUMPIFNEQ $id$idx$endfor GF@%%res bool@true
    return true;
}

bool gen_endfor(const char *id, unsigned long idx)
{
    CODE("JUMP $", id, "$"); CODE_NUM(idx); CODE("$for"); CODE("\n"); // JUMP $id$idx$for
    CODE("LABEL $", id, "$"); CODE_NUM(idx); CODE("$endfor"); CODE("\n"); // LABEL $id$idx$endfor
    return true;
}

bool gen_builtin_functions()
{
    CODE("###################################################\n"\
"LABEL $len\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval\n"\
"STRLEN LF@%%retval LF@%%0\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $inputs\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval0\n"\
"DEFVAR LF@%%retval1\n"\
"READ LF@%%retval0 string\n"\
"MOVE LF@%%retval1 int@0\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $inputi\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval0\n"\
"DEFVAR LF@%%retval1\n"\
"MOVE LF@%%retval1 int@0\n"\
"DEFVAR LF@typeout\n"\
"READ LF@%%retval0 int\n"\
"TYPE LF@typeout LF@%%retval0\n"\
"JUMPIFEQ $inputi$istrue LF@typeout string@int\n"\
"MOVE LF@%%retval1 int@1\n"\
"POPFRAME\n"\
"RETURN\n"\
"LABEL $inputi$istrue\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $inputf\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval0\n"\
"DEFVAR LF@%%retval1\n"\
"MOVE LF@%%retval1 int@0\n"\
"DEFVAR LF@typeout\n"\
"READ LF@%%retval0 float\n"\
"TYPE LF@typeout LF@%%retval0\n"\
"JUMPIFEQ $inputf$istrue LF@typeout string@float\n"\
"MOVE LF@%%retval1 int@1\n"\
"POPFRAME\n"\
"RETURN\n"\
"LABEL $inputf$istrue\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $print\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%0\n"\
"POPS LF@%%0\n"\
"DEFVAR LF@tmp\n"\
"DEFVAR LF@index\n"\
"MOVE LF@index int@0\n"\
"LABEL $print$while\n"\
"POPS LF@tmp\n"\
"WRITE LF@tmp\n"\
"ADD LF@index LF@index int@1\n"\
"JUMPIFNEQ $print$while LF@index LF@%%0\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $int2float\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval\n"\
"INT2FLOAT LF@%%retval LF@%%0\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $float2int\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval\n"\
"FLOAT2INT LF@%%retval LF@%%0\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $substr\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval1\n"\
"MOVE LF@%%retval1 int@0\n"\
"DEFVAR LF@index\n"\
"MOVE LF@index int@1\n"\
"DEFVAR LF@tmp\n"\
"DEFVAR LF@%%retval0\n"\
"MOVE LF@%%retval0 string@\\000\n"\
"\n"\
"STRLEN LF@tmp LF@%%0\n"\
"LT LF@%%retval1 LF@%%1 int@0\n"\
"JUMPIFEQ $substr$error LF@%%retval1 bool@true\n"\
"GT LF@%%retval1 LF@%%1 LF@tmp\n"\
"JUMPIFEQ $substr$error LF@%%retval1 bool@true\n"\
"LT LF@%%retval1 LF@%%2 int@0\n"\
"JUMPIFEQ $substr$error LF@%%retval1 bool@true\n"\
"SUB LF@tmp LF@tmp LF@%%1\n"\
"GT LF@%%retval1 LF@%%2 LF@tmp\n"\
"JUMPIFEQ $substr$isgreater LF@%%retval1 bool@true\n"\
"LABEL $substr$continue\n"\
"JUMPIFEQ $substr$end LF@%%2 int@0\n"\
"GETCHAR LF@%%retval0 LF@%%0 LF@%%1\n"\
"JUMPIFEQ $substr$end LF@%%2 int@1\n"\
"\n"\
"LABEL $substr$cycle\n"\
"ADD LF@%%1 LF@%%1 int@1\n"\
"ADD LF@index LF@index int@1\n"\
"GETCHAR LF@tmp LF@%%0 LF@%%1\n"\
"CONCAT LF@%%retval0 LF@%%retval0 LF@tmp\n"\
"JUMPIFNEQ $substr$cycle LF@index LF@%%2\n"\
"LABEL $substr$end\n"\
"MOVE LF@%%retval1 int@0\n"\
"POPFRAME\n"\
"RETURN\n"\
"LABEL $substr$error\n"\
"MOVE LF@%%retval1 int@1\n"\
"POPFRAME\n"\
"RETURN\n"\
"\n"\
"LABEL $substr$isgreater\n"\
"MOVE LF@%%2 LF@tmp\n"\
"JUMP $substr$continue\n"\
"###################################################\n"\
"LABEL $ord\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval1\n"\
"DEFVAR LF@%%retval0 \n"\
"STRLEN LF@%%retval0 LF@%%0\n"\
"SUB LF@%%retval0 LF@%%retval0 int@1\n"\
"GT LF@%%retval1 LF@%%1 LF@%%retval0\n"\
"JUMPIFEQ $ord$error LF@%%retval1 bool@true\n"\
"LT LF@%%retval1 LF@%%1 int@0\n"\
"JUMPIFEQ $ord$error LF@%%retval1 bool@true\n"\
"STRI2INT LF@%%retval0 LF@%%0 LF@%%1\n"\
"MOVE LF@%%retval1 int@0\n"\
"POPFRAME\n"\
"RETURN\n"\
"LABEL $ord$error\n"\
"MOVE LF@%%retval0 int@-2\n"\
"MOVE LF@%%retval1 int@1\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n"\
"LABEL $chr\n"\
"PUSHFRAME\n"\
"DEFVAR LF@%%retval1\n"\
"DEFVAR LF@%%retval0\n"\
"GT LF@%%retval1 LF@%%0 int@255\n"\
"JUMPIFEQ $chr$error LF@%%retval1 bool@true\n"\
"LT LF@%%retval1 LF@%%0 int@0\n"\
"JUMPIFEQ $chr$error LF@%%retval1 bool@true\n"\
"INT2CHAR LF@%%retval0 LF@%%0\n"\
"MOVE LF@%%retval1 int@0\n"\
"POPFRAME\n"\
"RETURN\n"\
"LABEL $chr$error\n"\
"MOVE LF@%%retval0 string@\n"\
"MOVE LF@%%retval1 int@1\n"\
"POPFRAME\n"\
"RETURN\n"\
"###################################################\n");
    return true;
}
