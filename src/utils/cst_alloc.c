/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 1999                             */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alan W Black (awb@cs.cmu.edu)                    */
/*               Date:  July 1999                                        */
/*************************************************************************/
/*                                                                       */
/*  Basic wraparounds for malloc and free                                */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cst_alloc.h"
#include "cst_error.h"

#ifdef UNDER_CE /* WinCE's standard library isn't */
void *calloc(size_t n, size_t s)
{
    void *ptr = malloc(n*s);
    if (ptr == NULL)
	return NULL;
    memset(ptr, 0, n*s);
    return ptr;
}
#endif /* UNDER_CE */

/* define this if you want to trace memory usage */
/* #define CST_DEBUG_MALLOC */
#ifdef CST_DEBUG_MALLOC
int cst_allocated = 0;
int cst_freed = 0;
int cst_alloc_max = 0;
int cst_alloc_imax = 0;
#endif

void *cst_safe_alloc(int size)
{
    /* returns pointer to memory all set 0 */
    void *p=0;
    if (size < 0)
    {
	cst_errmsg("alloc: asked for negative size %d\n", size);
	cst_error();
    }
    else if (size == 0)  /* some mallocs return NULL for this */
	size++;

#ifdef CST_DEBUG_MALLOC
    if (size > cst_alloc_imax)
	cst_alloc_imax = size;
    cst_allocated += size;
    size+=4;
#endif
    p = (void *)calloc(size,1);

#ifdef CST_DEBUG_MALLOC
    *(int *)p = size-4;
    if ((cst_allocated - cst_freed) > cst_alloc_max)
	cst_alloc_max = cst_allocated - cst_freed;
    p += 4;
#endif

    if (p == NULL)
    {
	cst_errmsg("alloc: can't alloc %d bytes\n", size);
	cst_error();
    }

    return p;
}

void *cst_safe_calloc(int size)
{
    return cst_safe_alloc(size);
}

void *cst_safe_realloc(void *p,int size)
{
    void *np=0;

#ifdef CST_DEBUG_MALLOC
    cst_free(p);
    return cst_safe_alloc(size);
#endif

    if (size == 0)
	size++;  /* as some mallocs do strange things with 0 */

    if (p == NULL)
	np = cst_safe_alloc(size);
    else
	np = realloc(p,size);

    if (np == NULL)
    {
	cst_errmsg("CST_REALLOC failed for %d bytes\n",size);
	cst_error();
    }

    return np;
}

void cst_free(void *p)
{
    if (p != NULL)
    {
#ifdef CST_DEBUG_MALLOC
	cst_freed += *(int *)(p-4);
	p -= 4;
#endif
	free(p);
    }
}





