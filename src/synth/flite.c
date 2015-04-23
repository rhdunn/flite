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
/*  Basic user level functions                                           */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "cst_tokenstream.h"
#include "flite.h"

int flite_init()
{
    cst_regex_init();

    return 0;
}

static cst_utterance *flite_synth_foo(cst_utterance *u,
				      cst_voice *voice,
				      cst_uttfunc synth)
{		       
    utt_init(u, voice);
    if ((*synth)(u) == NULL)
    {
	delete_utterance(u);
	return NULL;
    }
    else
	return u;
}

cst_utterance *flite_synth_text(const char *text, cst_voice *voice)
{
    cst_utterance *u;

    u = new_utterance();
    utt_set_input_text(u,text);
    return flite_synth_foo(u, voice, utt_synth);
}

cst_utterance *flite_synth_phones(const char *text, cst_voice *voice)
{
    cst_utterance *u;

    u = new_utterance();
    utt_set_input_text(u,text);
    return flite_synth_foo(u, voice, utt_synth_phones);
}

cst_wave *flite_text_to_wave(const char *text, cst_voice *voice)
{
    cst_utterance *u;
    cst_wave *w;

    if ((u = flite_synth_text(text,voice)) == NULL)
	return NULL;

    w = copy_wave(utt_wave(u));
    delete_utterance(u);
    return w;
}

static int utt_break(cst_tokenstream *ts,const char *token,cst_relation *tokens)
{
    /* This is English (and some other latin based languages) */
    /* so it shouldn't be here                                */
    const char *postpunct = item_feat_string(relation_tail(tokens), "punc");
    const char *ltoken = item_name(relation_tail(tokens));

    if (strchr(ts->whitespace,'\n') != cst_strrchr(ts->whitespace,'\n'))
	 /* contains two new lines */
	 return TRUE;
    else if (strchr(postpunct,':') ||
	     strchr(postpunct,'?') ||
	     strchr(postpunct,'!'))
	return TRUE;
    else if (strchr(postpunct,'.') &&
	     (strlen(ts->whitespace) > 1) &&
	     strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",token[0]))
	return TRUE;
    else if (strchr(postpunct,'.') &&
	     /* next word starts with a capital */
	     strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",token[0]) &&
	     /* last word isn't an abbreviation */
	     !(strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",ltoken[strlen(ltoken)-1])||
	       ((strlen(ltoken) < 4) &&
		strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",ltoken[0]))))
	return TRUE;
    else
	return FALSE;
}

float flite_file_to_speech(const char *filename, 
			   cst_voice *voice,
			   const char *outtype)
{
    cst_utterance *utt;
    cst_tokenstream *ts;
    const char *token;
    cst_item *t;
    cst_relation *tokrel;
    float d, durs = 0;
    int num_tokens;

    if ((ts = ts_open(filename)) == NULL)
    {
	cst_errmsg("failed to open file \"%s\" for reading\n",
		   filename);
	return 1;
    }

    ts->whitespacesymbols = 
	get_param_string(voice->features,"text_whitespace",
			 ts->whitespacesymbols);
    ts->singlecharsymbols = 
	get_param_string(voice->features,"text_singlecharsymbols",
			 ts->singlecharsymbols);
    ts->prepunctuationsymbols = 
	get_param_string(voice->features,"text_prepunctuation",
			 ts->prepunctuationsymbols);
    ts->postpunctuationsymbols = 
	get_param_string(voice->features,"text_postpunctuation",
			 ts->postpunctuationsymbols);

    /* If its a file to write to delete it as we're going to */
    /* incrementally append to it                            */
    if (!cst_streq(outtype,"play") && !cst_streq(outtype,"none"))
    {
	cst_wave *w;
	w = new_wave();
	cst_wave_resize(w,0,1);
	cst_wave_set_sample_rate(w,16000);
	cst_wave_save_riff(w,outtype);  /* an empty wave */
	delete_wave(w);
    }

    num_tokens = 0;
    utt = new_utterance();
    tokrel = utt_relation_create(utt, "Token");
    while (!ts_eof(ts))
    {
	token = ts_get(ts);
	if ((strlen(token) == 0) ||
	    (num_tokens > 500) ||  /* need an upper bound */
	    (relation_head(tokrel) && utt_break(ts,token,tokrel)))
	{
	    /* An end of utt */
	    d = flite_tokens_to_speech(utt,voice,outtype);
	    utt = NULL;
	    if (d < 0)
		goto out;
	    durs += d;

	    utt = new_utterance();
	    tokrel = utt_relation_create(utt, "Token");
	    num_tokens = 0;
	}
	num_tokens++;

	t = relation_append(tokrel, NULL);
	item_set_string(t,"name",token);
	item_set_string(t,"whitespace",ts->whitespace);
	item_set_string(t,"prepunctuation",ts->prepunctuation);
	item_set_string(t,"punc",ts->postpunctuation);
	item_set_int(t,"file_pos",ts->file_pos);
	item_set_int(t,"line_number",ts->line_number);
    }

out:
    delete_utterance(utt);
    ts_close(ts);
    return durs;
}

float flite_text_to_speech(const char *text,
			   cst_voice *voice,
			   const char *outtype)
{
    cst_utterance *u;
    cst_wave *w;
    float durs;

    u = flite_synth_text(text,voice);
    if (u == NULL)
	return -1;

    w = utt_wave(u);

    durs = (float)w->num_samples/(float)w->sample_rate;
	     
    if (cst_streq(outtype,"play"))
	play_wave(w);
    else if (!cst_streq(outtype,"none"))
	cst_wave_save_riff(w,outtype);
    delete_utterance(u);

    return durs;
}

float flite_phones_to_speech(const char *text,
			     cst_voice *voice,
			     const char *outtype)
{
    cst_utterance *u;
    cst_wave *w;
    float durs;

    u = flite_synth_phones(text,voice);
    if (u == NULL)
	    return -1;
    w = utt_wave(u);

    durs = (float)w->num_samples/(float)w->sample_rate;
	     
    if (cst_streq(outtype,"play"))
	play_wave(w);
    else if (!cst_streq(outtype,"none"))
	cst_wave_save_riff(w,outtype);
    delete_utterance(u);

    return durs;
}

float flite_tokens_to_speech(cst_utterance *u,
			     cst_voice *voice,
			     const char *outtype)
{
    cst_wave *w;
    float durs;

    u = flite_synth_foo(u,voice,utt_synth_tokens);
    if (u == NULL)
	    return -1;
    w = utt_wave(u);

    durs = (float)w->num_samples/(float)w->sample_rate;
	     
    if (cst_streq(outtype,"play"))
	play_wave(w);
    else if (!cst_streq(outtype,"none"))
	cst_wave_append_riff(w,outtype);
    delete_utterance(u);

    return durs;
}
