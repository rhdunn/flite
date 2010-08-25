/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                      Copyright (c) 2000-2009                          */
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
/*  Generic models for Pashto                                            */
/*************************************************************************/

#include "flite.h"
#include "cst_ffeatures.h"
#include "cmu_pashto_lang.h"

static const char * const cmu_pashto_punctuation = "\"'`.,:;!?(){}[]";
static const char * const cmu_pashto_prepunctuation = "\"'`({[";
static const char * const cmu_pashto_singlecharsymbols = "";
static const char * const cmu_pashto_whitespace = " \t\n\r";

void cmu_pashto_lang_init(cst_voice *v)
{

    /* utterance break function */
    feat_set(v->features,"utt_break",breakfunc_val(&default_utt_break));

    /* Phoneset */
    feat_set(v->features,"phoneset",phoneset_val(&cmu_pashto_phoneset));
    feat_set_string(v->features,"silence",cmu_pashto_phoneset.silence);

    /* Text analyser */
    feat_set_string(v->features,"text_whitespace",cmu_pashto_whitespace);
    feat_set_string(v->features,"text_postpunctuation",cmu_pashto_punctuation);
    feat_set_string(v->features,"text_prepunctuation",
		    cmu_pashto_prepunctuation);
    feat_set_string(v->features,"text_singlecharsymbols",
		    cmu_pashto_singlecharsymbols);

    feat_set(v->features,"tokentowords_func",itemfunc_val(&cmu_pashto_tokentowords));

    /* Phrasing */
    feat_set(v->features,"phrasing_cart",cart_val(&cmu_pashto_phrasing_cart));

    /* Intonation */
    feat_set_string(v->features,"no_intonation_accent_model","1");

#if 0
    /* very simple POS tagger */
    feat_set(v->features,"pos_tagger_cart",cart_val(&cmu_pashto_pos_cart));

#endif

    basic_ff_register(v->ffunctions);

}
