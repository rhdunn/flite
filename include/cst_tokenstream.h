/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 1999                             */
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
/*               Date:  July 1999                                        */
/*************************************************************************/
/*                                                                       */
/*  Tokenizer for strings and files                                      */
/*                                                                       */
/*************************************************************************/
#ifndef _CST_TOKENSTREAM_H__
#define _CST_TOKENSTREAM_H__

#include <stdio.h>
#include <string.h>
#include "cst_alloc.h"
#include "cst_string.h"
#include "cst_file.h"

typedef struct  cst_tokenstream_struct {
    cst_file fd;
    int file_pos;
    int line_number;
    char *string_buffer;

    int current_char;

    int token_pos;
    int ws_max;
    char *whitespace;
    int prep_max;
    char *prepunctuation;
    int token_max;
    char *token;
    int postp_max;
    char *postpunctuation;

    const char *whitespacesymbols;
    const char *singlecharsymbols;
    const char *prepunctuationsymbols;
    const char *postpunctuationsymbols;
} cst_tokenstream;

extern const char *cst_ts_default_whitespace;
extern const char *cst_ts_default_prepunc;
extern const char *cst_ts_default_postpunc;
extern const char *cst_ts_default_singlecharsymbols;

/* Public functions for tokenstream manipulation */
cst_tokenstream *new_tokenstream();
void delete_tokenstream(cst_tokenstream *ts);

cst_tokenstream *ts_open(const char *filename);
cst_tokenstream *ts_open_string(const char *string);

void ts_close(cst_tokenstream *ts);

int ts_eof(cst_tokenstream *ts);
const char *ts_get(cst_tokenstream *ts);

const char *ts_get_quoted_token(cst_tokenstream *ts,
				char quote,
				char escape);

#endif
