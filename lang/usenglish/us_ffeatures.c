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
/*  Feature functions used by various cart trees etc                     */
/*  These have been create as needed, and as some of the trees are       */
/*  from University of Edinburgh's Festival system their names and       */
/*  semantics follow them                                                */
/*************************************************************************/

#include "flite.h"
#include "usenglish.h"

static const cst_val *word_break(const cst_item *word);
static const cst_val *gpos(const cst_item *word);
static const cst_val *word_numsyls(const cst_item *word);
static const cst_val *ssyl_in(const cst_item *syl);
static const cst_val *syl_in(const cst_item *syl);
static const cst_val *syl_out(const cst_item *syl);
static const cst_val *syl_break(const cst_item *syl);
static const cst_val *syl_codasize(const cst_item *syl);
static const cst_val *syl_onsetsize(const cst_item *syl);
static const cst_val *ph_vc(const cst_item *p);
static const cst_val *ph_vlng(const cst_item *p);
static const cst_val *ph_vheight(const cst_item *p);
static const cst_val *ph_vrnd(const cst_item *p);
static const cst_val *ph_vfront(const cst_item *p);
static const cst_val *ph_ctype(const cst_item *p);
static const cst_val *ph_cplace(const cst_item *p);
static const cst_val *ph_cvox(const cst_item *p);
static const cst_val *accented(const cst_item *p);

DEF_STATIC_CONST_VAL_STRING(val_string_onset,"onset");
DEF_STATIC_CONST_VAL_STRING(val_string_coda,"coda");
DEF_STATIC_CONST_VAL_STRING(val_string_initial,"initial");
DEF_STATIC_CONST_VAL_STRING(val_string_single,"single");
DEF_STATIC_CONST_VAL_STRING(val_string_final,"final");
DEF_STATIC_CONST_VAL_STRING(val_string_mid,"mid");
DEF_STATIC_CONST_VAL_STRING(val_string_numeric,"numeric");
DEF_STATIC_CONST_VAL_STRING(val_string_number,"number");
DEF_STATIC_CONST_VAL_STRING(val_string_month,"month");
DEF_STATIC_CONST_VAL_STRING(val_string_day,"day");
DEF_STATIC_CONST_VAL_STRING(val_string_other,"_other_");
DEF_STATIC_CONST_VAL_STRING(val_string_a,"a");
DEF_STATIC_CONST_VAL_STRING(val_string_flight,"flight");
DEF_STATIC_CONST_VAL_STRING(val_string_to,"to");

static const cst_val *word_break(const cst_item *word)
{
    cst_item *ww,*pp;
    const char *pname;

    ww = item_as(word,"Phrase");

    if ((ww == NULL) || (item_next(ww) != 0))
	return VAL_STRING_1;
    else
    {
	pp = item_parent(ww);
	pname = val_string(item_feat(pp,"name"));
	if (cst_streq("BB",pname))
	    return VAL_STRING_4;
	else if (cst_streq("B",pname))
	    return VAL_STRING_3;
	else 
	    return VAL_STRING_1;
    }
}

DEF_STATIC_CONST_VAL_STRING(val_string_content,"content");

static const cst_val *gpos(const cst_item *word)
{
    /* Guess at part of speech (function/content) */
    const char *w;
    int s,t;

    w = item_feat_string(word,"name");

    for (s=0; us_gpos[s]; s++)
    {
	for (t=1; us_gpos[s][t]; t++)
	    if (cst_streq(w,val_string(us_gpos[s][t])))
		return us_gpos[s][0];
    }

    return (cst_val *)&val_string_content;
}

static const cst_val *word_numsyls(const cst_item *word)
{
    cst_item *d;
    int c;
    
    for (c=0,d=item_daughter(item_as(word,"SylStructure"));
	 d;
	 d=item_next(d),c++);

    return val_int_n(c);
}

static const cst_val *ssyl_in(const cst_item *syl)
{
    /* Number of stressed syllables since last major break */
    const cst_item *ss,*p,*fs;
    int c;

    ss = item_as(syl,"Syllable");

    fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughter.R:SylStructure.daughter");

    for (c=0, p=item_prev(ss); p; p=item_prev(p))
    {
	if (cst_streq("1",item_feat_string(p,"stress")))
	    c++;
	if (item_equal(p,fs))
	    break;
    }
    
    return val_string_n(c);  /* its used randomly as int and float */
}

static const cst_val *ssyl_out(const cst_item *syl)
{
    /* Number of stressed syllables until last major break */
    const cst_item *ss,*p,*fs;
    int c;

    ss = item_as(syl,"Syllable");

    fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughtern.R:SylStructure.daughter");

    for (c=0, p=item_next(ss); p; p=item_next(p))
    {
	if (cst_streq("1",item_feat_string(p,"stress")))
	    c++;
	if (item_equal(p,fs))
	    break;
    }
    
    return val_string_n(c);  /* its used randomly as int and float */
}

static const cst_val *syl_in(const cst_item *syl)
{
    /* Number of syllables since last major break */
    const cst_item *ss,*p,*fs;
    int c;

    ss = item_as(syl,"Syllable");

    fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughter.R:SylStructure.daughter");

    for (c=0, p=ss; p; p=item_prev(p),c++)
	if (item_equal(p,fs))
	    break;
    return val_string_n(c);
}

static const cst_val *syl_out(const cst_item *syl)
{
    /* Number of syllables until next major break */
    cst_item *ss,*p;
    const cst_item *fs;
    int c;

    ss = item_as(syl,"Syllable");

    fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughter.R:SylStructure.daughter");

    for (c=0, p=ss; p; p=item_prev(p),c++)
	if (item_equal(p,fs))
	    break;
    return val_string_n(c);
}

static const cst_val *syl_break(const cst_item *syl)
{
    /* Break level after this syllable */
    cst_item *ss;

    ss = item_as(syl,"SylStructure");

    if (ss == NULL)
	return VAL_STRING_1;  /* hmm, no sylstructure */
    else if (item_next(ss) != NULL)
	return VAL_STRING_0;  /* word internal */
    else if (item_parent(ss) == NULL)  /* no parent */
	return VAL_STRING_1;
    else
	return word_break(item_parent(ss));
}

static const cst_val *syl_codasize(const cst_item *syl)
{
    cst_item *d;
    int c;
    
    for (c=0,d=item_last_daughter(item_as(syl,"SylStructure"));
	 d;
	 d=item_prev(d),c++)
    {
	if (cst_streq("+",val_string(ph_vc(d))))
	    break;
    }

    return val_string_n(c);
}

static const cst_val *syl_onsetsize(const cst_item *syl)
{
    cst_item *d;
    int c;
    
    for (c=0,d=item_daughter(item_as(syl,"SylStructure"));
	 d;
	 d=item_next(d),c++)
    {
	if (cst_streq("+",val_string(ph_vc(d))))
	    break;
    }

    return val_string_n(c);
}

static const cst_val *ph_vc(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"vc");
}
static const cst_val *ph_vlng(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"vlng");
}
static const cst_val *ph_vheight(const cst_item *p)
{
   return phone_feature(item_phoneset(p),item_name(p),"vheight");
}
static const cst_val *ph_vrnd(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"vrnd");
}
static const cst_val *ph_vfront(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"vfront");
}
static const cst_val *ph_ctype(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"ctype");
}
static const cst_val *ph_cplace(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"cplace");
}
static const cst_val *ph_cvox(const cst_item *p)
{
    return phone_feature(item_phoneset(p),item_name(p),"cvox");
}

static const cst_val *accented(const cst_item *syl)
{
    if ((item_feat_present(syl,"accent")) ||
	(item_feat_present(syl,"endtone")))
	return VAL_STRING_1;
    else
	return VAL_STRING_0;
}

static const cst_val *asyl_in(const cst_item *syl)
{
    /* Number of accented syllables since last major break */
    const cst_item *ss,*p,*fs;
    int c;

    ss = item_as(syl,"Syllable");

    fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughter.R:SylStructure.daughter");

    for (c=0, p=ss; p; p=item_prev(p))
    {
	if (val_int(accented(p)) == 1)
	    c++;
	if (item_equal(p,fs))
	    break;
    }
    
    return val_string_n(c);
}

static const cst_val *seg_coda_fric(const cst_item *seg)
{
    const cst_item *s;
    const cst_phoneset *ps = item_phoneset(seg);
    
    for (s=item_last_daughter(item_parent(item_as(seg,"SylStructure")));
	 s;
	 s=item_prev(s))
    {
	if (cst_streq("+",phone_feature_string(ps,item_feat_string(s,"name"),
					       "vc")))
	    return VAL_STRING_0;
	if (cst_streq("f",phone_feature_string(ps,item_feat_string(s,"name"),
					       "ctype")))
	    return VAL_STRING_1;
    }

    return VAL_STRING_0;
}

static const cst_val *seg_onset_stop(const cst_item *seg)
{
    const cst_item *s;
    const cst_phoneset *ps = item_phoneset(seg);
    
    for (s=item_daughter(item_parent(item_as(seg,"SylStructure")));
	 s;
	 s=item_next(s))
    {
	if (cst_streq("+",phone_feature_string(ps,item_feat_string(s,"name"),
					       "vc")))
	    return VAL_STRING_0;
	if (cst_streq("s",phone_feature_string(ps,item_feat_string(s,"name"),
					       "ctype")))
	    return VAL_STRING_1;
    }

    return VAL_STRING_0;
}

static const cst_val *seg_coda_stop(const cst_item *seg)
{
    const cst_item *s;
    const cst_phoneset *ps = item_phoneset(seg);
    
    for (s=item_last_daughter(item_parent(item_as(seg,"SylStructure")));
	 s;
	 s=item_prev(s))
    {
	if (cst_streq("+",phone_feature_string(ps,item_feat_string(s,"name"),
					       "vc")))
	    return VAL_STRING_0;
	if (cst_streq("s",phone_feature_string(ps,item_feat_string(s,"name"),
					       "ctype")))
	    return VAL_STRING_1;
    }

    return VAL_STRING_0;
}

static const cst_val *seg_onset_nasal(const cst_item *seg)
{
    const cst_item *s;
    const cst_phoneset *ps = item_phoneset(seg);
    
    for (s=item_daughter(item_parent(item_as(seg,"SylStructure")));
	 s;
	 s=item_next(s))
    {
	if (cst_streq("+",phone_feature_string(ps,item_feat_string(s,"name"),
					       "vc")))
	    return VAL_STRING_0;
	if (cst_streq("n",phone_feature_string(ps,item_feat_string(s,"name"),
					       "ctype")))
	    return VAL_STRING_1;
    }

    return VAL_STRING_0;
}

static const cst_val *seg_onsetcoda(const cst_item *seg)
{
    const cst_item *s;
    const cst_phoneset *ps = item_phoneset(seg);
    
    for (s=item_next(item_as(seg,"SylStructure"));
	 s;
	 s=item_next(s))
    {
	if (cst_streq("+",phone_feature_string(ps,item_feat_string(s,"name"),
					       "vc")))
	    return (cst_val *)&val_string_onset;
    }
    return (cst_val *)&val_string_coda;
}

static const cst_val *pos_in_syl(const cst_item *seg)
{
    const cst_item *s;
    int c;
    
    for (c=-1,s=item_as(seg,"SylStructure");
	 s;
	 s=item_prev(s),c++);

    return val_string_n(c);
}

static const cst_val *position_type(const cst_item *syl)
{
    const cst_item *s = item_as(syl,"SylStructure");

    if (s == 0)
	return (cst_val *)&val_string_single;
    else if (item_next(s) == 0)
    {
	if (item_prev(s) == 0)
	    return (cst_val *)&val_string_single;
	else
	    return (cst_val *)&val_string_final;
    }
    else if (item_prev(s) == 0)
	return (cst_val *)&val_string_initial;
    else
	return (cst_val *)&val_string_mid;
}

static const cst_val *sub_phrases(const cst_item *syl)
{
    const cst_item *s;
    int c;
    
    for (c=0,s=path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.p");
	 s;
	 s=item_prev(s),c++);

    return val_string_n(c);
}

static const cst_val *last_accent(const cst_item *syl)
{
    const cst_item *s;
    int c;
    
    for (c=0,s=item_as(syl,"Syllable");
	 s;
	 s=item_prev(s),c++)
    {
	if (accented(s))
	    return val_string_n(c);
    }

    return val_string_n(c);
}

static const cst_val *syl_final(const cst_item *seg)
{   /* last segment in a syllable */
    const cst_item *s = item_as(seg,"SylStructure");

    if (!s || (item_next(s) == NULL))
	return VAL_STRING_1;
    else
	return VAL_STRING_0;
}

static const cst_val *num_digits(const cst_item *token)
{   
    const char *name = item_feat_string(token,"name");

    return val_int_n(strlen(name));
}

static const cst_val *month_range(const cst_item *token)
{   
    int v = item_feat_int(token,"name");

    if ((v > 0) && ( v < 32))
	return VAL_STRING_1;	
    else
	return VAL_STRING_0;	
}

static const cst_val *token_pos_guess(const cst_item *token)
{   
    const char *name = item_feat_string(token,"name");
    char *dc = cst_downcase(name);
    const cst_val *r;

    if (cst_regex_match(cst_rx_digits,dc))
	r = &val_string_numeric;
    else if ((cst_regex_match(cst_rx_double,dc)) ||
	(cst_regex_match(cst_rx_double,dc)))
	r = &val_string_number;
    else if (cst_streq(dc,"jan") ||
	cst_streq(dc,"january") ||
	cst_streq(dc,"feb") ||
	cst_streq(dc,"febrary") ||
	cst_streq(dc,"mar") ||
	cst_streq(dc,"march") ||
	cst_streq(dc,"apr") ||
	cst_streq(dc,"april") ||
	cst_streq(dc,"may") ||
	cst_streq(dc,"jun") ||
	cst_streq(dc,"june") ||
	cst_streq(dc,"jul") ||
	cst_streq(dc,"july") ||
	cst_streq(dc,"aug") ||
	cst_streq(dc,"august") ||
	cst_streq(dc,"sep") ||
	cst_streq(dc,"sept") ||
	cst_streq(dc,"september") ||
	cst_streq(dc,"oct") ||
	cst_streq(dc,"october") ||
	cst_streq(dc,"nov") ||
	cst_streq(dc,"november") ||
	cst_streq(dc,"dec") ||
	cst_streq(dc,"december"))
	r = &val_string_month;
    else if (cst_streq(dc,"sun") ||
	cst_streq(dc,"sunday") ||
	cst_streq(dc,"mon") ||
	cst_streq(dc,"monday") ||
	cst_streq(dc,"tue") ||
	cst_streq(dc,"tues") ||
	cst_streq(dc,"tuesday") ||
	cst_streq(dc,"wed") ||
	cst_streq(dc,"wednesday") ||
	cst_streq(dc,"thu") ||
	cst_streq(dc,"thurs") ||
	cst_streq(dc,"thursday") ||
	cst_streq(dc,"fri") ||
	cst_streq(dc,"friday") ||
	cst_streq(dc,"sat") ||
	cst_streq(dc,"saturday"))
	r = &val_string_day;
   /* ignoring the "token_most_common" condition, does get used */
    else if (cst_streq(dc,"a"))
	r =  &val_string_a;
    else if (cst_streq(dc,"flight"))
	r =  &val_string_flight;
    else if (cst_streq(dc,"to"))
	r =  &val_string_to;
    else
	r =  &val_string_other;
    cst_free(dc);
    return r;
}

void us_ff_register()
{
    ff_register("word_break",word_break);
    ff_register("gpos",gpos);
    ff_register("word_numsyls",word_numsyls);
    ff_register("ssyl_in",ssyl_in);
    ff_register("ssyl_out",ssyl_out);
    ff_register("syl_in",syl_in);
    ff_register("syl_out",syl_out);
    ff_register("syl_break",syl_break);
    ff_register("old_syl_break",syl_break);
    ff_register("syl_onsetsize",syl_onsetsize);
    ff_register("syl_codasize",syl_codasize);
    ff_register("ph_vc",ph_vc);
    ff_register("ph_vlng",ph_vlng);
    ff_register("ph_vheight",ph_vheight);
    ff_register("ph_vfront",ph_vfront);
    ff_register("ph_vrnd",ph_vrnd);
    ff_register("ph_ctype",ph_ctype);
    ff_register("ph_cplace",ph_cplace);
    ff_register("ph_cvox",ph_cvox);
    ff_register("accented",accented);
    ff_register("asyl_in",asyl_in);
    ff_register("seg_coda_fric",seg_coda_fric);
    ff_register("seg_onset_stop",seg_onset_stop);
    ff_register("seg_onsetcoda",seg_onsetcoda);
    ff_register("pos_in_syl",pos_in_syl);
    ff_register("position_type",position_type);
    ff_register("sub_phrases",sub_phrases);
    ff_register("last_accent",last_accent);
    ff_register("syl_final",syl_final);
    ff_register("seg_coda_stop",seg_coda_stop);
    ff_register("seg_onset_nasal",seg_onset_nasal);
    ff_register("num_digits",num_digits);
    ff_register("month_range",month_range);
    ff_register("token_pos_guess",token_pos_guess);
}
