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
/*               Date:  December 1999                                    */
/*************************************************************************/
/*                                                                       */
/*  Lexicon related functions                                            */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "cst_features.h"
#include "cst_lexicon.h"

CST_VAL_REGISTER_TYPE_NODEL(lexicon,cst_lexicon)

static int no_syl_boundaries(const cst_item *i, const cst_val *p);
static cst_val *lex_lookup_addenda(const char *wp,const cst_lexicon *l,
                                   int *found);

static int lex_match_entry(const char *a, const char *b);
static int lex_lookup_bsearch(const lexicon_entry *entries,
			      int start, int end,
			      const char *word);
static int find_full_match(const lexicon_entry *entries, int i,const char *word);

cst_lexicon *new_lexicon()
{
    cst_lexicon *l = cst_alloc(cst_lexicon,1);
    l->syl_boundary = no_syl_boundaries;
    return l;
}

void delete_lexicon(cst_lexicon *lex)
{
	if (lex)
	{
		cst_free(lex->entry_index);
		cst_free(lex->phones);
		cst_free(lex);
	}
}

lexicon_entry * lex_add_entry(cst_lexicon *l,
			      const char *word,
			      const char *pos,
			      const unsigned char *phones)
{
    unsigned char *nextphone;
    char *wp;
    int i;

    wp = cst_alloc(char,strlen(word)+2);
    sprintf(wp,"%c%s",(pos ? pos[0] : '0'),word);

    /* Allow me to rationalize my laziness by saying that linear
       search, realloc(), and friends are okay here since (a)
       user-defined lexicons won't be very large and (b) inserting
       items won't happen very often. */

    /* Find the first entry "greater than" the new item. */
    for (i = 0; i < l->num_entries; ++i) {
	int d = lex_match_entry(l->entry_index[i].word_pos, wp);

	if (d == 0) {
	    /* Uh oh, we have a match already.  How "fatal" should
               this be? */
	    return NULL;
	} else if (d > 0) {
	    break;
	}
    }
    
    /* Find phone indices and glob them onto the end of the phone table */
    if (l->phones == NULL) {
	nextphone = l->phones = cst_alloc(unsigned char, strlen(phones) + 1);
    } else {
	size_t lnphones;

	nextphone = l->phones + l->entry_index[l->num_entries-1].phone_index;
	nextphone += strlen(nextphone) + 1; /* They are null terminated */
	lnphones = nextphone - l->phones;
	l->phones = cst_realloc(l->phones, unsigned char,
				lnphones + strlen(phones) + 1);
	nextphone = l->phones + lnphones;
    }
    strcpy(nextphone, phones);

    /* Now expand and insert */
    l->entry_index = cst_realloc(l->entry_index, lexicon_entry, l->num_entries + 1);
    if (i < l->num_entries)
	    memmove(l->entry_index + i + 1, l->entry_index + i, l->num_entries - i);
    l->entry_index[i].word_pos = wp;
    l->entry_index[i].phone_index = nextphone - l->phones;

    ++l->num_entries;
    return l->entry_index + i;
}

int
lex_delete_entry(cst_lexicon *l, const char *word, const char *pos)
{
    unsigned char *phones, *nextphone, *lastphone;
    char *wp;
    int i, nphones, j;

    /* See notes above in lex_add_entry() about the inefficency of
       these functions. */

    wp = cst_alloc(char,strlen(word)+2);
    sprintf(wp,"%c%s",(pos ? pos[0] : '0'),word);

    /* Find the entry */
    if ((i = lex_lookup_bsearch(l->entry_index,0,l->num_entries,wp)) < 0) {
	cst_free(wp);
	return -1;
    }

    /* Shrink the phone index */
    phones = l->phones + l->entry_index[i].phone_index;
    nextphone = phones + strlen(phones) + 1;
    nphones = nextphone - phones;

    for (j = i + 1; j < l->num_entries; ++j)
	l->entry_index[j].phone_index -= nphones;

    lastphone = l->phones + l->entry_index[l->num_entries-1].phone_index;
    lastphone += strlen(lastphone) + 1;
    memmove(phones, nextphone, lastphone - nextphone);

    /* Dispose of the entry itself ... this of course assumes that it
       was added with lex_add_entry() and is thus not constant. */
    cst_free(l->entry_index[i].word_pos);

    /* Shrink the entry index */
    memmove(l->entry_index + i, l->entry_index + i + 1,
	    l->num_entries - i - 1);
    l->entry_index = cst_realloc(l->entry_index, lexicon_entry,
				 l->num_entries - 1);
    --l->num_entries;

    cst_free(wp);
    return 0;
}

#if 0
void lexicon_register(cst_lexicon *lex)
{
    /* Add given lexicon to list of known lexicons */
    cst_lexicon **old_lexs;
    int i;
    
    old_lexs = flite_lexicons;
    flite_num_lexicons++;
    flite_lexicons = cst_alloc(cst_lexicon *,flite_num_lexicons);
    for (i=0; i<flite_num_lexicons-1; i++)
	flite_lexicons[i] = old_lexs[i];
    flite_lexicons[i] = lex;
    cst_free(old_lexs);
}

cst_lexicon *lexicon_select(const char *name)
{
    int i;

    for (i=0; i < flite_num_lexicons; i++)
	if (cst_streq(name,flite_lexicons[i]->name))
	    return flite_lexicons[i];
    return NULL;
}
#endif

static int no_syl_boundaries(const cst_item *i, const cst_val *p)
{
    /* This is a default function that will normally be replace */
    /* for each lexicon                                         */
    (void)i;
    (void)p;
    return FALSE;
}

int in_lex(const cst_lexicon *l, const char *word, const char *pos)
{
    /* return TRUE is its in the lexicon */
    int r = FALSE, i;
    char *wp;

    wp = cst_alloc(char,strlen(word)+2);
    sprintf(wp,"%c%s",(pos ? pos[0] : '0'),word);

    for (i=0; l->addenda[i]; i++)
    {
	if (((wp[0] == '0') || (wp[0] == l->addenda[i][0][0])) &&
	    (cst_streq(wp+1,l->addenda[i][0]+1)))
	{
	    r = TRUE;
	    break;
	}
    }

    if (!r && (lex_lookup_bsearch(l->entry_index,0,l->num_entries,wp) >= 0))
	r = TRUE;

    cst_free(wp);
    return r;
}

cst_val *lex_lookup(const cst_lexicon *l, const char *word, const char *pos)
{
    int index,p;
    char *wp;
    cst_val *phones = 0;
    int found = FALSE;

    wp = cst_alloc(char,strlen(word)+2);
    sprintf(wp,"%c%s",(pos ? pos[0] : '0'),word);

    if (l->addenda)
	phones = lex_lookup_addenda(wp,l,&found);

    if (!found)
    {
	index = lex_lookup_bsearch(l->entry_index,0,l->num_entries,wp);

	if (index >= 0)
	{
	    for (p=l->entry_index[index].phone_index; l->phones[p]; p++)
		phones = cons_val(string_val(l->phone_table[l->phones[p]]),
				  phones);
	    phones = val_reverse(phones);
	}
	else if (l->lts_rule_set)
	    phones = lts_apply(word,
			       "",  /* more features if we had them */
			       l->lts_rule_set);
	else if (l->lts_function)
	    phones = (l->lts_function)(l,word,"");
    }

    cst_free(wp);
    return phones;
}

static cst_val *lex_lookup_addenda(const char *wp,const cst_lexicon *l, 
				   int *found)
{
    /* For those other words */
    int i,j;
    cst_val *phones;
    
    phones = NULL;
	
    for (i=0; l->addenda[i]; i++)
    {
	if (((wp[0] == '0') || (wp[0] == l->addenda[i][0][0])) &&
	    (cst_streq(wp+1,l->addenda[i][0]+1)))
	{
	    for (j=1; l->addenda[i][j]; j++)
		phones = cons_val(string_val(l->addenda[i][j]),phones);
	    *found = TRUE;
	    return val_reverse(phones);
	}
    }
    
    return NULL;
}

static int lex_lookup_bsearch(const lexicon_entry *entries,
			      int start, int end,
			      const char *word)
{
    int mid,c;

    while (start < end) {
	    mid = (start + end)/2;
	    c = lex_match_entry(entries[mid].word_pos,word);

	    if (c == 0)
		    return find_full_match(entries,mid,word);
	    else if (c > 0)
		    end = mid;
	    else
		    start = mid + 1;
    }
    return -1;
}

static int find_full_match(const lexicon_entry *entries, int i,const char *word)
{
    /* found word, now look for actual match including pos */
    int w, match;

    if (word[0] == '0')
	return i;  /* don't care about pos value */
    for (w=i; w >=0; w--)
    {
	if (!cst_streq(word+1,entries[w].word_pos+1))
	    break;
	else if ((cst_streq(word,entries[w].word_pos)) ||
		 (entries[w].word_pos[0] == '0'))
	    return w;
    }
    match = w+1;  /* if we can't find an exact match we'll take this one */
    
    for (w=i; entries[w].word_pos; w++)
    {
	if (!cst_streq(word+1,entries[w].word_pos+1))
	    break;
	else if ((cst_streq(word,entries[w].word_pos)) ||
		 (entries[w].word_pos[0] == '0'))
	    return w;
    }

    return match;
}

static int lex_match_entry(const char *a, const char *b)
{
    int c;

    c = strcmp(a+1,b+1);

    return c;
}


