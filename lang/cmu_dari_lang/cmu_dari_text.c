/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                      Copyright (c) 2001-2009                          */
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
/*                                                                       */
/*  Dari generic text analysis functions                                 */
/*                                                                       */
/*************************************************************************/

#include <ctype.h>
#include "flite.h"
#include "cmu_dari_lang.h"
#include "cst_regex.h"

#include "fbmap.c"

cst_utterance *cmu_dari_textanalysis(cst_utterance *u)
{
    return default_textanalysis(u);
}

static cst_val *cmu_dari_tokentowords_one(cst_item *token, const char *name);
cst_val *cmu_dari_tokentowords(cst_item *token)
{
    return cmu_dari_tokentowords_one(token, item_feat_string(token, "name"));
}

static cst_val *add_break(cst_val *l)
{
    /* add feature (break 1) to last item in this list */
    const cst_val *i;
    cst_val *t;
    cst_features *f;

    for (i=l; val_cdr(i); i=val_cdr(i));

    if (i)  /* might be empty list */
    {
	f = new_features();
	feat_set_string(f,"break","1");
	t = cons_val(val_car(i),features_val(f));
	set_car((cst_val *)i,t);
    }

    return l;
}

static char *cmu_dari_remove_joinspace(cst_item *token, const char *name)
{
    char *x;
    int i, j, l;

    x = cst_strdup(name);
    l = cst_strlen(name);
    for (j=i=0; name[i]; i++)
    {
        if ((i+2 < l) &&
            (name[i] == 0xffffffe2) &&
            (name[i+1] == 0xffffff80) &&
            ((name[i+2] == 0xffffff8c) ||
             (name[i+2] == 0xffffff8e) ||
             (name[i+2] == 0xffffff8f) ||
             (name[i+2] == 0xffffffaa) ||
             (name[i+2] == 0xffffffac)))
        {
            i += 2;
        }
        else if ((i+1 < l) &&
            (name[i] == 0xffffffc2) &&
            (name[i+1] == 0xffffffa0))
        {
            i += 1;
        }
        else if ((i+1 < l) &&
            (name[i] == 0xffffffd8) &&
            (name[i+1] == 0xffffff8c))
        {
            i += 1;
        }
        else
        {
            x[j] = x[i];
            j++;
        }
    }
    x[j] = '\0';
    
    return x;
}

cst_val *cmu_dari_fbmap(cst_val *l)
{
    /* Map any formation base A&B letters to standard letters */
    /* probably will do more copying that it should */
    const cst_val *i;
    int j;
    cst_val *nl = NULL;

    for (i=l; i; i=val_cdr(i))
    {
        for (j=0; dari_fbmap[j].x; j++)
        {
            if (cst_streq(val_string(val_car(i)),dari_fbmap[j].x))
                break;
        }
        if (dari_fbmap[j].x)
        {
            if (dari_fbmap[j].y)
                nl = cons_val(string_val(dari_fbmap[j].y),nl);
        }
        else
            nl = cons_val(string_val(val_string(val_car(i))),nl);
    }
    delete_val(l);
    return val_reverse(nl);

}

static cst_val *cmu_dari_tokentowords_one(cst_item *token, const char *name)
{
    /* Return list of words that expand token/name */
    cst_val *r;
    char *xname, *yname;
    cst_val *u1, *u2;

    /* printf("token_name %s name %s\n",item_name(token),name); */

    if (item_feat_present(token,"phones"))
	return cons_val(string_val(name),NULL);

#if 0
    if (item_feat_present(token,"nsw"))
	nsw = item_feat_string(token,"nsw");

    utt = item_utt(token);
    lex = val_lexicon(feat_val(utt->features,"lexicon"));
#endif

    xname = cmu_dari_remove_joinspace(token,name);

    /* More normalization -- for presentation forms */
    u1 = cst_utf8_explode(xname);
    u2 = cmu_dari_fbmap(u1);
    yname = cst_implode(u2);

    if (cst_strlen(xname) > 0)
        r = cons_val(string_val(yname),0);
    else
        r = NULL;
    
    cst_free(xname);
    cst_free(yname);
    /* u1 is deleted by dari_fbmap */
    delete_val(u2); 

    return r;
}




