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
/*               Date:  December 2000                                    */
/*************************************************************************/
/*  Generic models for US English                                        */
/*************************************************************************/

#ifndef _US_ENGLISH_H_
#define _US_ENGLISH_H_

#include "flite.h"

/* Used by festvox voices to define the voice */
void usenglish_init();
extern const cst_phoneset us_phoneset;

extern const char *us_english_punctuation;
extern const char *us_english_prepunctuation;
extern const char *us_english_singlecharsymbols;
extern const char *us_english_whitespace;

void us_text_init();
cst_utterance *us_textanalysis(cst_utterance *u);

extern const cst_cart us_phrasing_cart;

extern const cst_lexicon cmu_lex;

extern const cst_cart us_int_accent_cart;
extern const cst_cart us_int_tone_cart;
cst_utterance *us_f0_model(cst_utterance *u);

cst_utterance *us_postlex(cst_utterance *u);

extern const cst_cart us_durz_cart;
extern const dur_stats us_dur_stats;

/* Use internal to the usenglish directory */
void us_ff_register();
extern const cst_val * const * const us_gpos[];

#endif
