/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2000                            */
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
/*               Date:  September 2000                                   */
/*************************************************************************/
/*                                                                       */
/*  General synthesis control                                            */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "cst_hrg.h"
#include "cst_cart.h"
#include "cst_tokenstream.h"
#include "cst_utt_utils.h"
#include "cst_lexicon.h"
#include "cst_units.h"
#include "cst_synth.h"
#include "cst_phoneset.h"

static cst_utterance *tokenization(cst_utterance *u);
static cst_utterance *tokentowords(cst_utterance *u);
static cst_utterance *pos_tagger(cst_utterance *u);
static cst_utterance *phrasing(cst_utterance *u);
static cst_utterance *WordSylSeg(cst_utterance *u);
static cst_utterance *pauses(cst_utterance *u);
static cst_utterance *intonation(cst_utterance *u);
static cst_utterance *postlex(cst_utterance *u);
static cst_utterance *duration(cst_utterance *u);
static cst_utterance *int_target(cst_utterance *u);
static cst_utterance *tokentosegs(cst_utterance *u);
static cst_utterance *flat_prosody(cst_utterance *u);
static cst_utterance *wave_synth(cst_utterance *u);

cst_utterance *utt_synth(cst_utterance *u)
{
    /* The actual synthesis process */

    tokenization(u);
    tokentowords(u);
    pos_tagger(u);
    phrasing(u);
    WordSylSeg(u);
    pauses(u);
    intonation(u);
    postlex(u);
    duration(u);
    int_target(u);
/*    relation_save(utt_relation(u,"Segment"),"-");  */
    wave_synth(u);

    return u;
}

cst_utterance *utt_synth_phones(cst_utterance *u)
{
	tokenization(u);
	tokentosegs(u);
	duration(u);
	flat_prosody(u);
	wave_synth(u);

	return u;
}

static cst_utterance *tokenization(cst_utterance *u)
{
    const char *text,*token;
    cst_tokenstream *fd;
    cst_item *t;
    cst_relation *r;

    text = utt_input_text(u);
    r = utt_relation_create(u,"Token");
    fd = ts_open_string(text);
    fd->whitespacesymbols = 
	get_param_string(u->features,"text_whitespace",fd->whitespacesymbols);
    fd->singlecharsymbols = 
	get_param_string(u->features,"text_singlecharsymbols",
			 fd->singlecharsymbols);
    fd->prepunctuationsymbols = 
	get_param_string(u->features,"text_prepunctuation",
			 fd->prepunctuationsymbols);
    fd->postpunctuationsymbols = 
	get_param_string(u->features,"text_pospunctuation",
			 fd->postpunctuationsymbols);
    
    while(!ts_eof(fd))
    {
	token = ts_get(fd);
	if (strlen(token) > 0)
	{
	    t = relation_append(r,NULL);
	    item_set_string(t,"name",token);
	    item_set_string(t,"whitespace",fd->whitespace);
	    item_set_string(t,"prepunctuation",fd->prepunctuation);
	    item_set_string(t,"punc",fd->postpunctuation);
	    item_set_int(t,"file_pos",fd->file_pos);
	    item_set_int(t,"line_number",fd->line_number);
	}
    }

    ts_close(fd);
    
    return u;
}

static cst_utterance *tokentowords(cst_utterance *u)
{
    cst_uttfunc ta = val_uttfunc(feat_val(u->features,"textanalysis_func"));
    return (ta)(u);
}

static cst_utterance *pos_tagger(cst_utterance *u)
{
    return u;
}

static cst_utterance *phrasing(cst_utterance *u)
{
    cst_relation *r;
    cst_item *w, *p;
    const cst_val *v;
    cst_cart *phrasing_cart;

    r = utt_relation_create(u,"Phrase");
    phrasing_cart = val_cart(feat_val(u->features,"phrasing_cart"));

    for (p=NULL,w=relation_head(utt_relation(u,"Word")); w; w=item_next(w))
    {
	if (p == NULL)
	{
	    p = relation_append(r,NULL);
	    item_set_string(p,"name","BB");
	}
	item_add_daughter(p,w);
	v = cart_interpret(w,phrasing_cart);
	if (cst_streq(val_string(v),"BB"))
	    p = NULL;
    }
    
    return u;
}

static cst_utterance *pauses(cst_utterance *u)
{
    /* Add initial silences and silence at each phrase break */
    const char *silence;
    const cst_item *w;
    cst_item *p, *s;

    silence = val_string(feat_val(u->features,"silence"));

    /* Insert initial silence */
    s = relation_head(utt_relation(u,"Segment"));
    if (s == NULL)
	s = relation_append(utt_relation(u,"Segment"),NULL);
    else
	s = item_prepend(s,NULL);
    item_set_string(s,"name",silence);

    for (p=relation_head(utt_relation(u,"Phrase")); p; p=item_next(p))
    {
	for (w = item_last_daughter(p); w; w=item_prev(w))
	{
	    s = path_to_item(w,"R:SylStructure.daughtern.daughtern.R:Segment");
	    if (s &&
		!(cst_streq("",ffeature_string(w,"R:Token.parent.punc"))))
	    {
		s = item_append(s,NULL);
		item_set_string(s,"name",silence);
		break;
	    }
	}
    }

    return u;
}

static cst_utterance *intonation(cst_utterance *u)
{
    cst_cart *accents, *tones;
    cst_item *s;
    const cst_val *v;

    accents = val_cart(feat_val(u->features,"int_cart_accents"));
    tones = val_cart(feat_val(u->features,"int_cart_tones"));
    
    for (s=relation_head(utt_relation(u,"Syllable")); s; s=item_next(s))
    {
	v = cart_interpret(s,accents);
	if (!cst_streq("NONE",val_string(v)))
	    item_set_string(s,"accent",val_string(v));
	v = cart_interpret(s,tones);
	if (!cst_streq("NONE",val_string(v)))
	    item_set_string(s,"endtone",val_string(v));
    }
    return u;
}

static cst_utterance *postlex(cst_utterance *u)
{
    cst_uttfunc pl = val_uttfunc(feat_val(u->features,"postlex_func"));
    return (pl)(u);
}

CST_VAL_REGISTER_TYPE_NODEL(dur_stats,dur_stats)

const dur_stat *phone_dur_stat(const dur_stats ds,const char *ph)
{
    int i;
    for (i=0; ds[i]; i++)
	if (cst_streq(ph,ds[i]->phone))
	return ds[i];

    return ds[0];
}

static cst_utterance *duration(cst_utterance *u)
{
    cst_cart *dur_tree;
    cst_item *s;
    float zdur, dur_stretch, dur;
    float end;
    dur_stats ds;
    const dur_stat *dur_stat;

    end = 0;

    dur_tree = val_cart(feat_val(u->features,"dur_cart"));
    dur_stretch = get_param_float(u->features,"Duration_Stretch", 1.0);
    /* Why do I need this cast ? */
    ds = (dur_stats)val_dur_stats(feat_val(u->features,"dur_stats"));
    
    for (s=relation_head(utt_relation(u,"Segment")); s; s=item_next(s))
    {
	zdur = val_float(cart_interpret(s,dur_tree));
	dur_stat = phone_dur_stat(ds,item_name(s));
	dur = dur_stretch * ((zdur*dur_stat->stddev)+dur_stat->mean);
	end += dur;
	item_set_float(s,"end",end);
    }
    return u;
}

static cst_utterance *int_target(cst_utterance *u)
{
    cst_uttfunc ta = val_uttfunc(feat_val(u->features,"f0_model_func"));
    return (ta)(u);
}

static cst_utterance *WordSylSeg(cst_utterance *u)
{
    cst_item *word;
    cst_relation *sylstructure,*seg,*syl;
    cst_lexicon *lex, *ulex = NULL;
    
    const cst_val *p;
    char *phone_name;
    char *stress = "0";
    cst_val *phones;
    cst_item *ssword, *sssyl, *segitem, *sylitem, *seg_in_syl;
    
    lex = val_lexicon(feat_val(u->features,"lexicon"));
    if (feat_present(u->features, "user_lexicon"))
	ulex = val_lexicon(feat_val(u->features, "user_lexicon"));

    syl = utt_relation_create(u,"Syllable");
    sylstructure = utt_relation_create(u,"SylStructure");
    seg = utt_relation_create(u,"Segment");
    
    for (word=relation_head(utt_relation(u,"Word")); 
	 word; word=item_next(word))
    {
	ssword = relation_append(sylstructure,word);
	phones = NULL;
	if (ulex)
	    phones = lex_lookup(ulex,item_feat_string(word, "name"),0);
	if (phones == NULL)
	    phones = lex_lookup(lex,item_feat_string(word,"name"),0);
	for (sssyl=NULL,sylitem=NULL,p=phones; p; p=val_cdr(p))
	{
	    if (sylitem == NULL)
	    {
		sylitem = relation_append(syl,NULL);
		sssyl = item_add_daughter(ssword,sylitem);
		stress = "0";
	    }
	    segitem = relation_append(seg,NULL);
	    phone_name = cst_strdup(val_string(val_car(p)));
	    if (phone_name[strlen(phone_name)-1] == '1')
	    {
		stress = "1";
		phone_name[strlen(phone_name)-1] = '\0';
	    }
	    item_set_string(segitem,"name",phone_name);
	    seg_in_syl = item_add_daughter(sssyl,segitem);
	    if ((lex->syl_boundary)(seg_in_syl,val_cdr(p)))
	    {
		sylitem = NULL;
		if (sssyl)
		    item_set_string(sssyl,"stress",stress);
	    }
	    cst_free(phone_name);
	}
	delete_val(phones);
    }

    return u;
}

static cst_utterance *wave_synth(cst_utterance *u)
{

    cst_uttfunc ws = val_uttfunc(feat_val(u->features,"wave_synth_func"));
    return (ws)(u);

    return u;
}

/* Dummy F0 modelling for phones, copied directly from us_f0_model.c */
static cst_utterance *flat_prosody(cst_utterance *u)
{
    /* F0 target model */
    cst_item *s,*t;
    cst_relation *targ_rel;
    float mean, stddev;

    targ_rel = utt_relation_create(u,"Target");
    mean = get_param_float(u->features,"target_f0_mean", 100.0);
    stddev = get_param_float(u->features,"target_f0_stddev", 12.0);

    s=relation_head(utt_relation(u,"Segment"));
    t = relation_append(targ_rel,NULL);
    item_set_float(t,"pos",0.0);
    item_set_float(t,"f0",120.0);

    s=relation_tail(utt_relation(u,"Segment"));
    t = relation_append(targ_rel,NULL);

    item_set_float(t,"pos",item_feat_float(s,"end"));
    item_set_float(t,"f0",100.0);

    return u;
}

static cst_utterance *tokentosegs(cst_utterance *u)
{
    cst_item *t;
    cst_relation *seg;
    cst_phoneset *ps;

    ps = val_phoneset(utt_feat_val(u, "phoneset"));
    /* Just copy tokens into the Segment relation */
    seg = utt_relation_create(u, "Segment");
    for (t = relation_head(utt_relation(u, "Token")); t; t = item_next(t)) {
	    cst_item *segitem = relation_append(seg, NULL);
	    char const *pname = item_feat_string(t, "name");

	    if (phone_id(ps, pname) == -1) {
		    cst_errmsg("Phone `%s' not in phoneset\n", pname);
		    cst_error();
	    }
	    item_set_string(segitem, "name", pname);
    }
    return u;
}

