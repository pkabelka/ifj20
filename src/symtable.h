/* Head file for symtable.h - nonrekursive implementation of operations over BT.Hlavičkový soubor pro c402.c - nerekurzívní implementace operací nad BVS
obsahuje jednak nutné knihovny a externí proměnné, ale rovněž definici datových
typů, se kterými se pracuje v příslušných úlohách. Nemodifikovat! */

/* ********************** SOUBOR S HLAVIČKOU ********************** */
/* ********************** ------------------ ********************** */

/*  Předmět: Algoritmy (IAL) - FIT (Fakulta Informačních Technologií)
    Hlavička pro soubor: c402.c
    Vytvořil: Martin Tuček, září 2005
    Upravil: Bohuslav Křena, listopad 2009
             Karel Masařík, říjen 2013
             Radek Hranický 2014-2018
*/
/* ***************************************************************** */

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<stdbool.h>
#include"str.h"

#define FALSE 0
#define TRUE 1
#define MAXSTACK 30                      /* maximální počet prvků v zásobníku */
#define SYMTABLE_SUCCESS 0



typedef struct tSTNode	{                                      /* uzel stromu */
    char *key;                                         /* užitečný obsah uzlu  */
    struct tSTNode * LPtr;                                  /* levý podstrom  */
    struct tSTNode * RPtr; 	  	                            /* pravý podstrom */
} *STNodePtr;                                     /* ukazatel na uzel stromu */

/* pomocné zásobníky */
typedef struct	{                          /* zásobník hodnot typu tBTNodePtr */
    STNodePtr a[MAXSTACK];
    int top;
} tStackP;

typedef struct {                                 /* zásobník hodnot typu bool */
    bool a[MAXSTACK];
    int top;
} tStackB;

/***prototypes***/
void symtable_init (STNodePtr * );
/**
 * @brief Initializes new symtable.
 * @param tSTNode tree to be initialized
**/
STNodePtr symtable_search(STNodePtr *, const char *);
/**
 * @brief searches const char in given tree, returns matching node
**/
STNodePtr symtable_insert (STNodePtr *, const char *, bool *);
/**
 * @brief Inserts in symtable new node with value of Content.
 * @param c const char to be inserted as string.
**/
void Leftmost_Preorder(STNodePtr, tStackP *);
/**
 * @brief Goes on left branch of subtree, untill it stops on its most left node.
 * @param tStack stack used in function
**/
void symtable_preorder(STNodePtr );
/**
 * @brief Passage through tree using preorder style.
 * @param STnode node to walk through
**/

void Leftmost_Inorder(STNodePtr, tStackP *);
/**
 * @brief Goes trhough left brand of subtree untill gets on the most left node.
 * @param STnode node to walk through
**/
void symtable_inorder(STNodePtr );
/**
 * @brief Passage through tree using inorder style
 * @param STnode node to walk through
**/

void Leftmost_Postorder(STNodePtr, tStackP *, tStackB *);
/**
 * @brief Goes trhough left brand of subtree untill gets on the most left node.
 * @param STnode node to walk through
**/
void symtable_postorder(STNodePtr );
/**
 * @brief Passage through tree using Postorder style
**/

void symtable_dispose(STNodePtr *);
/**
 * @brief Disposes all nodes in tree and frees memory.
 * @param tSTNode tree to be disposed.
**/
/********end of prototypes******/

/* konec c402.h */