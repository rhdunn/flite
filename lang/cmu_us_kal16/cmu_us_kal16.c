/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                      Copyright (c) 1999-2000                          */
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
/*               Date:  December 2000                                    */
/*************************************************************************/
/*                                                                       */
/*  A simple voice defintion                                             */
/*                                                                       */
/*************************************************************************/

#include "flite.h"
#include "cst_diphone.h"
#include "usenglish.h"

static cst_utterance *cmu_us_kal_utt_init(cst_utterance *u, cst_voice *v);
static cst_utterance *cmu_us_kal_utt_synth(cst_utterance *u, cst_voice *v);
extern cst_diphone_db cmu_us_kal_db;

cst_voice *cmu_us_kal_diphone = NULL;

cst_voice *register_cmu_us_kal()
{
    cst_voice *v = new_voice();

    v->utt_init = cmu_us_kal_utt_init;
    v->utt_synth = cmu_us_kal_utt_synth;

    usenglish_init();

    /* Set up basic values for synthesizing with this voice */
    feat_set_string(v->features,"name","cmu_us_kal_diphone");

    /* Phoneset */
    feat_set(v->features,"phoneset",phoneset_val(&us_phoneset));
    feat_set_string(v->features,"silence",us_phoneset.silence);

    /* Text analyser */
    feat_set_string(v->features,"text_whitespace",us_english_whitespace);
    feat_set_string(v->features,"text_postpunctuation",us_english_punctuation);
    feat_set_string(v->features,"text_prepunctuation",
		    us_english_prepunctuation);
    feat_set_string(v->features,"text_singlecharsymbols",
		    us_english_singlecharsymbols);

    feat_set(v->features,"textanalysis_func",uttfunc_val(&us_textanalysis));

    /* Phrasing */
    feat_set(v->features,"phrasing_cart",cart_val(&us_phrasing_cart));

    /* Lexicon */
    feat_set(v->features,"lexicon",lexicon_val(&cmu_lex));

    /* Intonation */
    feat_set(v->features,"int_cart_accents",cart_val(&us_int_accent_cart));
    feat_set(v->features,"int_cart_tones",cart_val(&us_int_tone_cart));
    feat_set_float(v->features,"int_f0_target_mean",105.0);
    feat_set_float(v->features,"int_f0_target_stddev",14.0);
    feat_set(v->features,"f0_model_func",uttfunc_val(&us_f0_model));

    /* Post lexical rules */
    feat_set(v->features,"postlex_func",uttfunc_val(&us_postlex));

    /* Duration */
    feat_set(v->features,"dur_cart",cart_val(&us_durz_cart));
    feat_set(v->features,"dur_stats",dur_stats_val(&us_dur_stats));
    feat_set_float(v->features,"Duration_Stretch",1.1);

    /* Waveform synthesis: diphone_synth */
    feat_set(v->features,"wave_synth_func",uttfunc_val(&diphone_synth));
    feat_set(v->features,"diphone_db",diphone_db_val(&cmu_us_kal_db));
    feat_set_string(v->features,"resynth_type","fixed");
    feat_set_string(v->features,"join_type","modified_lpc");
/*    feat_set_string(v->features,"join_type","simple_join"); */

    cmu_us_kal_diphone = v;

    return cmu_us_kal_diphone;
}

static cst_utterance *cmu_us_kal_utt_init(cst_utterance *u, cst_voice *v)
{
    /* Set up voice specific paramerers for synthesis */

    return u;
}

static cst_utterance *cmu_us_kal_utt_synth(cst_utterance *u, cst_voice *v)
{
    feat_copy_into(v->features,u->features);
    /* Use standard synthesizer */
    return utt_synth(u);
}

