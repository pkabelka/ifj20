
/* c402.c: ********************************************************************}
{* Téma: Nerekurzivní implementace operací nad BVS
**                                     Implementace: Petr Přikryl, prosinec 1994
**                                           Úpravy: Petr Přikryl, listopad 1997
**                                                     Petr Přikryl, květen 1998
**			  	                        Převod do jazyka C: Martin Tuček, srpen 2005
**                                         Úpravy: Bohuslav Křena, listopad 2009
**                                                 Karel Masařík, říjen 2013
**                                                 Radek Hranický 2014-2018
**/

#include "symtable.h"
#include "str.h"
#include "error.h"

int solved;

void BTWorkOut (STNodePtr Ptr)		{
/*   ---------
** Help function called while passing tree for processed Ptr
**/

	if (Ptr==NULL)
    printf("Chyba: Funkce BTWorkOut byla volána s NULL argumentem!\n");
  else
    printf("Výpis hodnoty daného uzlu> %d\n",Ptr->key);
}

/* -------------------------------------------------------------------------- */
/*
** stack for STNodePtr type
**/

void SInitP (tStackP *S)
/*   ------
** Stack initialization
**/
{
	S->top = 0;
}

void SPushP (tStackP *S, STNodePtr ptr)
/*   ------
** Inserts value on top of stack
**/
{
                 /* Při implementaci v poli může dojít k přetečení zásobníku. */
  if (S->top==MAXSTACK)
    printf("Chyba: Došlo k přetečení zásobníku s ukazateli!\n");
  else {
		S->top++;
		S->a[S->top]=ptr;
	}
}

STNodePtr STopPopP (tStackP *S)
/*         --------
** Removes value from top and return its value
**/
{
                            /* causes error if stack is empty. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
		return(NULL);
	}
	else {
		return (S->a[S->top--]);
	}
}

bool SEmptyP (tStackP *S)
/*   -------
** returns true if stack is empty
**/
{
  return(S->top==0);
}


/*--------------------------------------------------------------
** Binary stack
*/
void SInitB (tStackB *S) {
/*   ------
** Stack initialization
**/

	S->top = 0;
}

void SPushB (tStackB *S,bool val) {
/*   ------
** Inserts value on top of stack
**/
                 /* Overflow can happen */
	if (S->top==MAXSTACK)
		printf("Chyba: Došlo k přetečení zásobníku pro boolean!\n");
	else {
		S->top++;
		S->a[S->top]=val;
	}
}

bool STopPopB (tStackB *S) {
/*   --------
** Removes value from top and return its value
**/
                            
	if (S->top==0) {
		printf("Chyba: Došlo k podtečení zásobníku pro boolean!\n");
		return(NULL);
	}
	else {
		return(S->a[S->top--]);
	}
}

bool SEmptyB (tStackB *S) {
/*   -------
** Return true if stack is empty
**/
  return(S->top==0);
}


void symtable_init (STNodePtr *RootPtr)	{
/*   -------------
** Initializes new symtable.
**
**For symtable disposal there is function symtable_dispose
**/

    *RootPtr = NULL;
}

STNodePtr symtable_insert (STNodePtr *RootPtr, const char *key, bool *error) {
/*   ---------------
** Inserts in symtable new node with value of key.
**/
    STNodePtr output = malloc(sizeof(struct tSTNode));
    if (output == NULL)
        {
            *error = true;
            return NULL;
        }
    *error = false;
    char *s = (char *)malloc(strlen(key)+sizeof(char));
    if (s == NULL)
    {
        *error = true;
        return NULL;
    }
    strcpy(s, key);
    if (*RootPtr == NULL)
    {
        STNodePtr new = malloc(sizeof(struct tSTNode));
        if (new == NULL)
        {
            *error = true;
            return NULL;
        }
        new->LPtr = NULL;
        new->RPtr = NULL;
        new->key = s;
        if (new->key == NULL)
        {
            *error = true;
            return NULL;
        }
        *RootPtr = new;
        return new;
    }

    STNodePtr tmp = *RootPtr;
    while(tmp != NULL)
    {
        
        if (strcmp(tmp->key, s) == 0)
        {
            break;
        }

        if (strcmp(tmp->key, s) > 0) // New node will be inserted on the left side of current
        {
            if (tmp->LPtr != NULL)
            {
                tmp = tmp->LPtr; // No free space, we go further
            }
            else
            {
                STNodePtr new = malloc(sizeof(struct tSTNode));
                if (new == NULL)
                {
                    *error = true;
                    return NULL;
                }
                new->LPtr = NULL;
                new->RPtr = NULL;
                new->key = s;
                tmp->LPtr = new; // new node is on the left 
                output = new;
            }
        }
        else if (strcmp(tmp->key, s) < 0) // new node will be inserted on the left side of current
        {
            if (tmp->RPtr != NULL)
            {
                tmp = tmp->RPtr; // No free space, we go futher
            }
            else
            {
                STNodePtr new = malloc(sizeof(struct tSTNode));
                if (new == NULL)
                {
                    *error = true;
                    return NULL;
                }
                new->LPtr = NULL;
                new->RPtr = NULL;
                new->key = s;
                tmp->RPtr = new; // new node will be inserted on the right side of current
                output = new;
            }
        }
    }
    return output;
}

/*                                  PREORDER                                  */
STNodePtr symtable_search(STNodePtr *RootPtr, const char *key){
/*        ---------------
** Searches for occurance of given const char in tree
*/

    if (*RootPtr == NULL)
    {
        return NULL;
    }

    STNodePtr tmp = *RootPtr;
    while (tmp != NULL)
    {

        if(strcmp(key, tmp->key)==0)
            return tmp;
        
        else if(strcmp(key,tmp->key) < 0)
            tmp = tmp->LPtr;

        else if(strcmp(key, tmp->key) > 0)
            tmp = tmp->RPtr;
    }
    return NULL;
    
}
void Leftmost_Preorder (STNodePtr ptr, tStackP *Stack)	{
/*   -----------------
** Goes on left branch of subtree, untill it stops on its most left node.
**/

    while (ptr != NULL)
    {
        SPushP(Stack, ptr);
        BTWorkOut(ptr);
        ptr = ptr->LPtr;
    }
}

void symtable_preorder (STNodePtr RootPtr)	{
/*   -----------------
** Passage through tree using preorder style.
**/

    tStackP s;
    SInitP(&s);
    Leftmost_Preorder(RootPtr, &s);

    STNodePtr ptr = NULL;
    while (!SEmptyP(&s))
    {
        ptr = STopPopP(&s);
        Leftmost_Preorder(ptr->RPtr, &s);
    }
}


/*                                  INORDER                                   */

void Leftmost_Inorder(STNodePtr ptr, tStackP *Stack)		{
/*   ----------------
** Goes trhough left brand of subtree untill gets on the most left node.
**/

    while (ptr != NULL)
    {
        SPushP(Stack, ptr);
        ptr = ptr->LPtr;
    }
}

void symtable_inorder (STNodePtr RootPtr)	{
/*   ----------------
** Passage through tree using inorder style
**/

    tStackP s;
    SInitP(&s);

    STNodePtr ptr = RootPtr;
    Leftmost_Inorder(ptr, &s);
    while (!SEmptyP(&s))
    {
        ptr = STopPopP(&s);
        Leftmost_Inorder(ptr->RPtr, &s);
        BTWorkOut(ptr);
    }
}

/*                                 POSTORDER                                  */

void Leftmost_Postorder (STNodePtr ptr, tStackP *StackP, tStackB *StackB) {
/*   ------------------
** Goes trhough left brand of subtree untill gets on the most left node.
*/

    while (ptr != NULL)
    {
        SPushP(StackP, ptr);
        SPushB(StackB, true);
        ptr = ptr->LPtr;
    }
}

void symtable_postorder (STNodePtr RootPtr)	{
/*           -----------
** Passage through tree using Postorder style
**/

    tStackP sp;
    SInitP(&sp);
    tStackB sb;
    SInitB(&sb);
    
    Leftmost_Postorder(RootPtr, &sp, &sb);

    STNodePtr ptr = NULL;
    bool left;
    while (!SEmptyP(&sp))
    {
        ptr = STopPopP(&sp);
        SPushP(&sp, ptr);
        left = STopPopB(&sb);
        if (left)
        {
            SPushB(&sb, false);
            Leftmost_Postorder(ptr->RPtr, &sp, &sb);
        }
        else
        {
            STopPopP(&sp);
            BTWorkOut(ptr);
        }
    }
}


void symtable_dispose (STNodePtr *RootPtr)	{
/*   -------------
** Disposes all nodes in tree and frees memory
**/

    if (*RootPtr == NULL)
    {
        return;
    }

    // same passage as symtable_inorder
    tStackP s;
    SInitP(&s);

    STNodePtr ptr = *RootPtr;
    Leftmost_Inorder(ptr, &s);
    while (!SEmptyP(&s))
    {
        ptr = STopPopP(&s);
        Leftmost_Inorder(ptr->RPtr, &s);
        free(ptr);
        ptr = NULL;
    }
    *RootPtr = NULL;
}

/* konec c402.c */
