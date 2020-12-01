/**
 * Project name: Imperative language IFJ20 compiler implementation
 * Název projektu: Implementace překladače imperativního jazyka IFJ20
 * 
 * @brief Dynamic string interface
 *
 * @author Petr Kabelka <xkabel09 at stud.fit.vutbr.cz>
 */

#ifndef _STR_H
#define _STR_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define STR_ALLOC_CONST 8 // Constant of 8 bytes to prevent reallocating with each character
// #define str_add_var(...) str_add_var(__VA_ARGS__, NULL)

/**
 * @struct Dynamic string
 */
typedef struct
{
    char *str;
    unsigned int len; // String length
    unsigned int mem_size; // Allocated memory size
} string;

/**
 * @brief Initializes the dynamic string
 * @param s Pointer to the string structure
 * @return True upon successful initialization
 */
bool str_init(string *s);

/**
 * @brief Frees the memory allocated by the dynamic string structure
 */
void str_free(string *s);

/**
 * @brief Clears the string content
 */
void str_clear(string *s);

/**
 * @brief Appends a single character to the dynamic string
 * @param s Pointer to the string structure
 * @param c Appended character
 * @return True upon successful append
 */
bool str_add(string *s, char c);

/**
 * @brief Appends a string literal to the dynamic string
 * @param s Pointer to the string structure
 * @param c Appended string literal
 * @return True upon successful append
 */
bool str_add_const(string *s, const char *cstr);

/**
 * @brief Appends a multiple string literals to the dynamic string
 * @param s Pointer to the string structure
 * @param ... unlimited parameters of type char *, last parameter MUST be NULL
 * @return True upon successful append
 */
bool str_add_var(string *s, ...);

/**
 * @brief Appends s2 to the end of s1
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @return True upon successful append
 */
bool str_add_str(string *s1, string *s2);

/**
 * @brief Copies the dynamic string from src to dst
 * @param src Pointer to the string structure
 * @param dst Appended string literal
 * @return True upon successful copy
 */
bool str_copy(string *src, string *dst);

/**
 * @brief Compares two dynamic strings
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @return An integer greater than, equal to, or less than 0, if the string pointed to by s1 is greater than, equal to, or less than the string pointed to by s2, respectively.
 */
int str_cmp(string *s1, string *s2);

/**
 * @brief Compares a dynamic string and a string literal
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @return An integer greater than, equal to, or less than 0, if the string pointed to by s1 is greater than, equal to, or less than the string pointed to by s2, respectively.
 */
int str_cmp_const(string *s1, const char *s2);

/**
 * @brief Swaps the two strings
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 */
void str_swap(string *s1, string *s2);

#endif