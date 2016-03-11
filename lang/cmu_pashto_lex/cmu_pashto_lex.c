/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 2009                             */
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
/*               Date:  August 2009                                      */
/*************************************************************************/
/*                                                                       */
/*  PASHTO Lexicon definition                                            */
/*                                                                       */
/*************************************************************************/

#include "flite.h"

#include "cmu_pashto_lex.h"

extern const int cmu_pashto_lex_entry[];
extern const unsigned char cmu_pashto_lex_data[];
extern const int cmu_pashto_lex_num_entries;
extern const int cmu_pashto_lex_num_bytes;
extern const char * const cmu_pashto_lex_phone_table[54];
extern const char * const cmu_pashto_lex_phones_huff_table[];
extern const char * const cmu_pashto_lex_entries_huff_table[];

static int cmu_pashto_is_vowel(const char *p);
static int cmu_pashto_is_silence(const char *p);
static int cmu_pashto_has_vowel_in_list(const cst_val *v);
static int cmu_pashto_has_vowel_in_syl(const cst_item *i);

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
static const char * const addenda24[] = { "p'",NULL};
static const char * const addenda25[] = { "p`",NULL};
static const char * const addenda26[] = { "p\"",NULL};
static const char * const addenda27[] = { "p-",NULL};
static const char * const addenda28[] = { "p<",NULL};
static const char * const addenda29[] = { "p>",NULL};

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
    addenda24,
    addenda25,
    addenda26,
    addenda27,
    addenda28,
    addenda29,
    NULL };

static int cmu_pashto_is_silence(const char *p)
{
    if (cst_streq(p,"pau"))
	return TRUE;
    else
	return FALSE;
}

static int cmu_pashto_has_vowel_in_list(const cst_val *v)
{
    const cst_val *t;

    for (t=v; t; t=val_cdr(t))
	if (cmu_pashto_is_vowel(val_string(val_car(t))))
	    return TRUE;
    return FALSE;
}

static int cmu_pashto_has_vowel_in_syl(const cst_item *i)
{
    const cst_item *n;

    for (n=i; n; n=item_prev(n))
	if (cmu_pashto_is_vowel(item_feat_string(n,"name")))
	    return TRUE;
    return FALSE;
}

static int cmu_pashto_is_vowel(const char *p)
{
    /* this happens to work for US English phoneset */
    if (strchr("aeiouAEIOU@",p[0]) == NULL)
	return FALSE;
    else
	return TRUE;
}

static int cmulex_dist_to_vowel(const cst_val *rest)
{
    if (rest == 0)
        return 0;  /* shouldn't get here */
    else if (cmu_pashto_is_vowel(val_string(val_car(rest))))
        return 0;
    else
        return 1+cmulex_dist_to_vowel(val_cdr(rest));
}

static const char * const cmulex_onset_trigrams[] = {
    "str", "spy", "spr", "spl", "sky", "skw", "skr", "skl", NULL
};
static const char * const cmulex_onset_bigrams[] = {
    "zw", "zl",
    "vy", "vr", "vl",
    "thw", "thr",
    "ty", "tw",
    "tr", /* "ts", */
    "shw", "shr", "shn", "shm", "shl",
    "sw", "sv", "st", "sr", "sp", "sn", "sm", "sl", "sk", "sf",
    "py", "pw", "pr", "pl",
    "ny",
    "my", "mr",
    "ly",
    "ky", "kw", "kr", "kl",
    "hhy", "hhw", "hhr", "hhl",
    "gy", "gw", "gr", "gl", 
    "fy", "fr", "fl", 
    "dy", "dw", "dr",
    "by", "bw", "br", "bl",
    NULL
};

static int cmulex_onset_bigram(const cst_val *rest)
{
    char x[10];
    int i;

    cst_sprintf(x,"%s%s",val_string(val_car(rest)),
           val_string(val_car(val_cdr(rest))));
    for (i=0; cmulex_onset_bigrams[i]; i++)
        if (cst_streq(x,cmulex_onset_bigrams[i]))
            return TRUE;
    return FALSE;
}

static int cmulex_onset_trigram(const cst_val *rest)
{
    char x[15];
    int i;

    cst_sprintf(x,"%s%s%s",val_string(val_car(rest)),
           val_string(val_car(val_cdr(rest))),
           val_string(val_car(val_cdr(val_cdr(rest)))));
    for (i=0; cmulex_onset_trigrams[i]; i++)
        if (cst_streq(x,cmulex_onset_trigrams[i]))
            return TRUE;
    return FALSE;
}

int cmu_pashto_syl_boundary_mo(const cst_item *i,const cst_val *rest)
{
    /* syl boundary maximal onset */
    int d2v;

    if (rest == NULL)
	return TRUE;
    else if (cmu_pashto_is_silence(val_string(val_car(rest))))
	return TRUE;
    else if (!cmu_pashto_has_vowel_in_list(rest)) 
        /* no more vowels so rest *all* coda */
	return FALSE;
    else if (!cmu_pashto_has_vowel_in_syl(i))  /* need a vowel */
        /* no vowel yet in syl so keep copying */
	return FALSE;
    else if (cmu_pashto_is_vowel(val_string(val_car(rest))))
        /* next is a vowel, syl has vowel, so this is a break */
	return TRUE;
    else 
    {
        /* want to know if from rest to the next vowel is a valid onset */
        d2v = cmulex_dist_to_vowel(rest);
        if (d2v < 2)
            return TRUE;
        else if (d2v > 3)
            return FALSE;
        else if (d2v == 2) 
            return cmulex_onset_bigram(rest);
        else /* if (d2v == 3) */
            return cmulex_onset_trigram(rest);
        return TRUE;
    }

}

cst_val *cmu_pashto_lex_lts_function(const struct lexicon_struct *l, 
                                   const char *word, const char *pos)
{
    cst_val *phones = 0;
    cst_val *p2 = 0;
    cst_val *utflets = 0;
    cst_val *x = 0;

    /* Need to explode word as a utf-string */
    utflets = cst_utf8_explode(word);

    /* Do lts */
    phones = lts_apply_val(utflets,
                           "",  /* more features if we had them */
                           l->lts_rule_set);

    for (x=phones; x; x=val_cdr(x))
    {
        if (cst_streq("@",val_string(val_car(x))))
            p2 = cons_val(string_val("@0"),p2);
        else
            p2 = cons_val(string_val(val_string(val_car(x))),p2);
    }
    delete_val(phones);
    phones = val_reverse(p2);
#if 0
    printf("awb_debug pashto OOV \"%s\" ",word);
    val_print(stdout,phones);
    printf("\n");
#endif

    delete_val(utflets);

    return phones;
}

cst_lexicon cmu_pashto_lex;
cst_lts_rules cmu_pashto_lts_rules;
extern const char * const cmu_pashto_lts_phone_table[];
extern const char * const cmu_pashto_lts_letter_table[];
extern const cst_lts_addr cmu_pashto_lts_letter_index[];
extern const cst_lts_model cmu_pashto_lts_model[];

cst_lexicon *cmu_pashto_lex_init()
{
    /* I'd like to do this as a const but it needs everything in this */
    /* file and already the bits are too big for some compilers */
    
    if (cmu_pashto_lts_rules.name)
        return &cmu_pashto_lex;  /* Already initialized */

    cmu_pashto_lts_rules.name = "cmu_pashto";
    cmu_pashto_lts_rules.letter_index = cmu_pashto_lts_letter_index;
#ifdef CST_NO_STATIC_LTS_MODEL
    /* cmu_pashto_lex_lts_rules.models will be set elsewhere */
#else
    cmu_pashto_lts_rules.models = cmu_pashto_lts_model;
#endif
    cmu_pashto_lts_rules.phone_table = cmu_pashto_lts_phone_table;
    cmu_pashto_lts_rules.context_window_size = 4;
    cmu_pashto_lts_rules.context_extra_feats = 1;
    cmu_pashto_lts_rules.letter_table = cmu_pashto_lts_letter_table;

    cmu_pashto_lex.name = "cmu_pashto_lex";
    cmu_pashto_lex.num_entries = cmu_pashto_lex_num_entries;
#ifdef CST_NO_STATIC_LEX
    /* cmu_pashto_lex.data will be set elsewhere */
#else
    cmu_pashto_lex.data = cmu_pashto_lex_data;
#endif
    cmu_pashto_lex.num_bytes = cmu_pashto_lex_num_bytes;
    cmu_pashto_lex.phone_table = (char **) cmu_pashto_lex_phone_table;
    cmu_pashto_lex.syl_boundary = cmu_pashto_syl_boundary_mo;
    cmu_pashto_lex.addenda = (char ***) addenda;
    cmu_pashto_lex.lts_rule_set = (cst_lts_rules *) &cmu_pashto_lts_rules;

    cmu_pashto_lex.lts_function = cmu_pashto_lex_lts_function;

    cmu_pashto_lex.phone_hufftable = cmu_pashto_lex_phones_huff_table;
    cmu_pashto_lex.entry_hufftable = cmu_pashto_lex_entries_huff_table;

    cmu_pashto_lex.postlex = cmu_pashto_postlex;

    return &cmu_pashto_lex;

}
