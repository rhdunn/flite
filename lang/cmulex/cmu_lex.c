/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 2001                             */
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
/*               Date:  January 2001                                     */
/*************************************************************************/
/*                                                                       */
/*  CMU Lexicon definition                                               */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include "flite.h"

extern const unsigned char cmu_lex_phones[];
extern const lexicon_entry cmu_lex_entry[];
extern const int cmu_num_entries;
extern const char * const cmu_phone_table[54];
extern const cst_lts_rules cmu6_lts_rules;

static int cmu_is_vowel(const char *p);
static int cmu_is_silence(const char *p);
static int cmu_has_vowel_in_list(const cst_val *v);
static int cmu_has_vowel_in_syl(const cst_item *i);
static int cmu_sonority(const char *p);

static const char * const addenda0[] = { "p,", NULL };
static const char * const addenda1[] = { "p.", NULL };
static const char * const addenda2[] = { "p(", NULL };
static const char * const addenda3[] = { "p)", NULL };
static const char * const addenda4[] = { "p[", NULL };
static const char * const addenda5[] = { "p]", NULL };
static const char * const addenda6[] = { "p{", NULL };
static const char * const addenda7[] = { "p}", NULL };
static const char * const addenda8[] = { "p:", NULL };
static const char * const addenda9[] = { "p;", NULL };
static const char * const addenda10[] = { "p?", NULL};
static const char * const addenda11[] = { "p!", NULL };
static const char * const addenda12[] = { "n@", "ae1", "t", NULL };
static const char * const addenda13[] = { "n#", "hh", "ae1","sh", NULL };
static const char * const addenda14[] = { "n$", "d", "aa1", "l", "er", NULL };
static const char * const addenda15[] = { "n%", "p", "er", "s", "eh1", "n", "t", NULL };
static const char * const addenda16[] = { "n^", "k", "eh1", "r", "eh1", "t",  NULL };
static const char * const addenda17[] = { "n&","ae1","m","p","er","s","ae1","n","d", NULL };
static const char * const addenda18[] = { "n*","ae1","s","t","er","ih1","s","k",NULL };
static const char * const addenda19[] = { "n|","b","aa1","r",NULL };
static const char * const addenda20[] = { "n\\","b","ae1","k","s","l","ae1","sh",NULL };
static const char * const addenda21[] = { "n=","iy1","k","w","ax","l","z",NULL};
static const char * const addenda22[] = { "n+","p","l","ah1","s",NULL};
static const char * const addenda23[] = { "n~","t","ih1","l","d","ax",NULL};
static const char * const addenda24[] = { "p'",NULL};
static const char * const addenda25[] = { "p`",NULL};
static const char * const addenda26[] = { "p\"",NULL};
static const char * const addenda27[] = { "p-",NULL};
static const char * const addenda28[] = { "p<",NULL};
static const char * const addenda29[] = { "p>",NULL};
static const char * const addenda30[] = { "n_","ah1","n","d","er","s","k","ao1","r",NULL};
static const char * const addenda31[] = { "s's","z",NULL};
static const char * const addenda32[] = { "nim","ay1","m",NULL};
static const char * const addenda33[] = { "vdoesnt","d","ah1","z","n","t",NULL};
static const char * const addenda34[] = { "vyoull","y","uw1","l",NULL};
static const char * const addenda35[] = { "n/","s","l","ae1","sh",NULL};

static const char * const addenda36[] = { "nin","ih","n",NULL};
static const char * const addenda37[] = { "nto","t","ax",NULL};
static const char * const addenda38[] = { "n_a","ey",NULL};
static const char * const addenda39[] = { "ncepstral","k","eh1", "p", "s", "t", "r", "ax", "l", NULL};

static const char * const * const addenda[] = {
    addenda0,
    addenda1,
    addenda2,
    addenda3,
    addenda4,
    addenda5,
    addenda6,
    addenda7,
    addenda8,
    addenda9,
    addenda10,
    addenda11,
    addenda12,
    addenda13,
    addenda14,
    addenda15,
    addenda16,
    addenda17,
    addenda18,
    addenda19,
    addenda20,
    addenda21,
    addenda22,
    addenda23,
    addenda24,
    addenda25,
    addenda26,
    addenda27,
    addenda28,
    addenda29,
    addenda30,
    addenda31,
    addenda32,
    addenda33,
    addenda34,
    addenda35,

    addenda36,
    addenda37,
    addenda38,
    addenda39,
    NULL };

static int cmu_is_silence(const char *p)
{
    if (cst_streq(p,"pau"))
	return TRUE;
    else
	return FALSE;
}

static int cmu_has_vowel_in_list(const cst_val *v)
{
    const cst_val *t;

    for (t=v; t; t=val_cdr(t))
	if (cmu_is_vowel(val_string(val_car(v))))
	    return TRUE;
    return FALSE;
}

static int cmu_has_vowel_in_syl(const cst_item *i)
{
    const cst_item *n;

    for (n=i; n; n=item_prev(n))
	if (cmu_is_vowel(item_feat_string(n,"name")))
	    return TRUE;
    return FALSE;
}

static int cmu_is_vowel(const char *p)
{
    /* this happens to work for US English phoneset */
    if (strchr("aeiou",p[0]) == NULL)
	return FALSE;
    else
	return TRUE;
}

static int cmu_sonority(const char *p)
{
    /* A bunch of hacks for US English phoneset */
    if (cmu_is_vowel(p) || (cmu_is_silence(p)))
	return 5;
    else if (strchr("wylr",p[0]) != NULL)
	return 4;  /* glides/liquids */
    else if (strchr("nm",p[0]) != NULL)
	return 3;  /* nasals */
    else if (strchr("bdgjlmnnnrvwyz",p[0]) != NULL)
	return 2;  /* voiced obstruents */
    else
	return 1;
}

int cmu_syl_boundary(const cst_item *i,const cst_val *v)
{
    /* Returns TRUE if this should be a syllable boundary */
    /* This is of course phone set dependent              */
    int p, n, nn;
    
    if (v == NULL)
	return TRUE;
    else if (cmu_is_silence(val_string(val_car(v))))
	return TRUE;
    else if (!cmu_has_vowel_in_list(v)) /* no more vowels so rest *all* coda */
	return FALSE;
    else if (!cmu_has_vowel_in_syl(i))  /* need a vowel */
	return FALSE;
    else if (cmu_is_vowel(val_string(val_car(v))))
	return TRUE;
    else if (val_cdr(v) == NULL)
	return FALSE;
    else 
    {   /* so there is following vowel, and multiple phones left */
	p = cmu_sonority(item_feat_string(i,"name"));
	n = cmu_sonority(val_string(val_car(v)));
	nn = cmu_sonority(val_string(val_car(val_cdr(v))));

	if ((p <= n) && (n <= nn))
	    return TRUE;
	else
	    return FALSE;
    }
}

cst_lexicon cmu_lex;

void cmu_lex_init()
{
    /* I'd like to do this as a const but it needs everything in this */
    /* file and already the bits are too big for some compilers */
    cmu_lex.name = "cmu";
    cmu_lex.num_entries = cmu_num_entries;
    cmu_lex.entry_index = (lexicon_entry *) cmu_lex_entry;
    cmu_lex.phones = (unsigned char *) cmu_lex_phones;
    cmu_lex.phone_table = (char **) cmu_phone_table;
    cmu_lex.syl_boundary = cmu_syl_boundary;
    cmu_lex.addenda = (char ***) addenda;
    cmu_lex.lts_rule_set = (cst_lts_rules *) &cmu6_lts_rules;
}
