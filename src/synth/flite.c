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

static char *tokens_to_text(cst_val *tokens);

int flite_init()
{
    /* I'd like there to be nothing here, so hopefully these can be deleted */

    cst_regex_init();

    return 0;
}

cst_utterance *flite_synth_text(const char *text, cst_voice *voice)
{
    cst_utterance *u;

    u = new_utterance();
    voice->utt_init(u,voice);

    utt_set_input_text(u,text);
    voice->utt_synth(u,voice);

    return u;
}

cst_utterance *flite_synth_phones(const char *text, cst_voice *voice)
{
    cst_utterance *u;

    u = new_utterance();
    voice->utt_init(u,voice);

    utt_set_input_text(u,text);
    /* For some reason this is done in utt_synth not utt_init, which
       is wrong for us since we want to use a different synth type. */
    feat_copy_into(voice->features,u->features);
    utt_synth_phones(u);

    return u;
}

cst_wave *flite_text_to_wave(const char *text, cst_voice *voice)
{
    cst_utterance *u;

    u = flite_synth_text(text,voice);
    return utt_wave(u);
}

static int utt_break(cst_tokenstream *ts,const char *token,cst_val *tokens)
{
    /* This is English (and some other latin based languages) */
    /* so it shouldn't be here                                */
    const char *postpunct = val_string(val_car(tokens));
    const char *ltoken = val_string(val_car(val_cdr(tokens)));

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
	     !strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",ltoken[strlen(ltoken)-1]))
	return TRUE;
    else
	return FALSE;
}

float flite_file_to_speech(const char *filename, 
			 cst_voice *voice,
			 const char *outtype)
{
    cst_tokenstream *ts;
    cst_val *tokens;
    const char *token;
    char *text;
    float durs = 0;
    int verbosity = 0;
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
	get_param_string(voice->features,"text_pospunctuation",
			 ts->postpunctuationsymbols);
    verbosity = get_param_int(voice->features,"verbosity",0);

    tokens = NULL;
    num_tokens = 0;
    while (!ts_eof(ts))
    {
	token = ts_get(ts);
	if ((strlen(token) == 0) ||
	    (num_tokens > 500) ||  /* need an upper bound */
	    (tokens && utt_break(ts,token,tokens)))
	{
	    /* An end of utt */
	    tokens = val_reverse(tokens);
	    text = tokens_to_text(tokens);
#ifndef UNDER_CE
	    if (verbosity)
	    {
		printf("%s",text);
		fflush(stdout);
	    }
#endif
	    durs += (float)flite_text_to_speech(text,voice,outtype);
	    cst_free(text);

	    delete_val(tokens);
	    tokens = NULL;
	    num_tokens = 0;
	}
	num_tokens++;
	tokens = cons_val(string_val(ts->postpunctuation),
  	         cons_val(string_val(token),
  	         cons_val(string_val(ts->prepunctuation),
		 cons_val(string_val(ts->whitespace),
                          tokens))));
    }
#ifndef UNDER_CE
    if (verbosity) printf("\n");

#endif

    delete_val(tokens);
    ts_close(ts);
    return durs;
}

static char *tokens_to_text(cst_val *tokens)
{
    /* shouldn't do it this way */
    int size,lsize;
    const cst_val *v;
    char *text;
    const char *t;

    for (v=tokens,size=0; v; v=val_cdr(v))
	size += strlen(val_string(val_car(v)));

    text = cst_alloc(char,size+1);

    for (v=tokens,size=0; v; v=val_cdr(v))
    {
	t = val_string(val_car(v));
	lsize = strlen(t);
	memmove(&text[size],
		t, lsize);
	size += lsize;
    }
    return text;
}

float flite_text_to_speech(const char *text,
			 cst_voice *voice,
			 const char *outtype)
{
    cst_utterance *u;
    cst_wave *w;
    float durs;

    u = flite_synth_text(text,voice);
    w = utt_wave(u);

    durs = (float)w->num_samples/(float)w->sample_rate;
	     
    if (cst_streq(outtype,"play"))
	play_wave(w);
    else if (!cst_streq(outtype,"none"))
	cst_wave_save_riff(w,outtype);
    delete_utterance(u);

    return durs;
}
