/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2001                            */
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
/*  US English text analysis functions                                   */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "flite.h"
#include "us_text.h"

static cst_val *en_token_to_words(cst_item *token, const char *name);
static int text_splitable(const char *s,int i);

const char *us_english_punctuation = "\"'`.,:;!?(){}[]";
const char *us_english_prepunctuation = "\"'`({[";
const char *us_english_singlecharsymbols = "";
const char *us_english_whitespace = " \t\n\r";

static cst_regex *ordinal_number = 0;
static cst_regex *hasvowel = 0;

void us_text_init()
{
    /* text initialization function, primarily for setting up regexes */
    if (ordinal_number == 0)
    {
	ordinal_number = new_cst_regex("[0-9][0-9,]*\\(th\\|TH\\|st\\|ST\\|nd\\|ND\\|rd\\|RD\\)");
	hasvowel = new_cst_regex(".*[aeiouAEIOU].*");
    }
}

cst_utterance *us_textanalysis(cst_utterance *u)
{
    /* English tokenization */
    cst_item *t,*word;
    cst_relation *word_rel;
    cst_val *words;
    const cst_val *w;

    word_rel = utt_relation_create(u,"Word");

    for (t=relation_head(utt_relation(u,"Token")); t; t=item_next(t))
    {
	words = en_token_to_words(t,item_feat_string(t,"name"));
	for (w=words; w; w=val_cdr(w))
	{
	    word = item_add_daughter(t,NULL);
	    /* Need more clever setting for features */
	    item_set_string(word,"name",val_string(val_car(w)));
	    relation_append(word_rel,word);
	}
	delete_val(words);
    }

    return u;
}

static cst_val *en_token_to_words(cst_item *token, const char *name)
{
    /* Return list of words that expand token/name */
    char *p, *aaa;
    int i,j;
    cst_val *r;
    const char *nsw = "";

    if (item_feat_present(token,"nsw"))
	nsw = item_feat_string(token,"nsw");

    if (cst_regex_match(cst_rx_commaint,name))
    {
	aaa = cst_strdup(name);
	for (j=i=0; i < strlen(name); i++)
	    if (name[i] != ',')
	    {
		aaa[j] = name[i];
		j++;
	    }
	aaa[j] = '\0';
	r = en_token_to_words(token,aaa);
	cst_free(aaa);
    }
    else if (cst_regex_match(cst_rx_digits,name))
    {
	if (cst_streq("nide",nsw))
	    r = en_exp_id(name);
	else {
	    const cst_val *tv = cart_interpret(token,&us_nums_cart);
	    const char *ts = val_string(tv);
	    if (cst_streq(ts,"ordinal"))
		r = en_exp_ordinal(name);
	    else if (cst_streq(ts,"digits"))
		r = en_exp_digits(name);
	    else if (cst_streq(ts,"year"))
		r = en_exp_id(name);
	    else
		r = en_exp_number(name);
	}
    }
    else if (cst_regex_match(cst_rx_double,name))
    {   /* real numbers */
	if (name && (name[0] == '-'))
	    r = cons_val(string_val("minus"),
			 en_token_to_words(token,&name[1]));
	else if (((p=strchr(name,'e')) != 0) ||
		 ((p=strchr(name,'E')) != 0))
        {
	    aaa = cst_strdup(name);
	    aaa[strlen(name)-strlen(p)] = '\0';
	    r = val_append(en_token_to_words(token,aaa),
			   cons_val(string_val("e"),
				    en_token_to_words(token,aaa)));
	    cst_free(aaa);
        }
	else if ((p=strchr(name,'.')) != 0)
	{
	    aaa = cst_strdup(name);
	    aaa[strlen(name)-strlen(p)] = '\0';
	    r = val_append(en_exp_number(aaa),
			   cons_val(string_val("point"),
				    en_exp_digits(p+1)));
	    cst_free(aaa);
	}
	else
	    r = en_exp_number(name);  /* I don't think you can get here */
    }
    else if (cst_regex_match(ordinal_number,name))
    {   /* explicit ordinals */
	aaa = cst_strdup(name);
	aaa[strlen(name)-2] = '\0';
	r = en_exp_ordinal(aaa);
	cst_free(aaa);
    }
    else if (((p=(strstr(name,"'s"))) || (p=(strstr(name,"'S"))))
	     && (p[2] == '\0'))
    {   /* apostrophe s */
	aaa = cst_strdup(name);
	aaa[strlen(name)-strlen(p)] = '\0';
	r = val_append(en_token_to_words(token,aaa),
		       cons_val(string_val("'s"),0));
	cst_free(aaa);
    }
    else if ((p=(strchr(name,'\''))))
    {   /* internal single quote deleted */
	aaa = cst_strdup(name);
	strcpy(&aaa[strlen(name)-strlen(p)],p+1);
	r = en_token_to_words(token,aaa);
	cst_free(aaa);
    }
    else if ((strlen(name) > 1) && 
	     (cst_regex_match(cst_rx_alpha,name)) &&
	     (!cst_regex_match(hasvowel,name)) &&
	     (strchr(name,'y') == 0) &&
	     (strchr(name,'Y') == 0))
	/* unpronouncable list of alphas */
	r = en_exp_letters(name);
    else if ((p=(strchr(name,'-'))))
    {   /* aaa-bbb */
	aaa = cst_strdup(name);
	aaa[strlen(name)-strlen(p)] = '\0';
	r = val_append(en_token_to_words(token,aaa),
		       en_token_to_words(token,p+1));
	cst_free(aaa);
    }
    else if ((strlen(name) > 1) && (!cst_regex_match(cst_rx_alpha,name)))
    {   /* its not just alphas */
	for (i=0; name[i] != '\0'; i++)
	    if (text_splitable(name,i))
		break;
	aaa = cst_strdup(name);
	aaa[i+1] = '\0';
	item_set_string(token,"nsw","nide");
	r = val_append(en_token_to_words(token,aaa),
		       en_token_to_words(token,&name[i+1]));
	cst_free(aaa);
    }

    /* buckets of other stuff missing */

    else  /* just a word */
    {
	aaa = cst_downcase(name);
	r = cons_val(string_val(aaa),0);
	cst_free(aaa);
    }
    return r;
}

static int text_splitable(const char *s,int i)
{
    /* should token be split abter this */

    if (strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",s[i]) &&
	strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",s[i+1]))
	return FALSE;
    else if (strchr("0123456789",s[i]) &&
	     strchr("0123456789",s[i+1]))
	return FALSE;
    else
	return TRUE;
}


