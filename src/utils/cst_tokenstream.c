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
#include <stdio.h>
#include "cst_tokenstream.h"

const char * const ts_default_whitespacesymbols = " \t\n\r";
const char * const ts_default_singlecharsymbols = "(){}[]";
const char * const ts_default_prepunctuationsymbols = "\"'`({[";
const char * const ts_default_postpunctuationsymbols = "\"'`.,:;!?(){}[]";

#define TS_BUFFER_SIZE 256

static const char ts_getc(cst_tokenstream *ts);

static void extend_buffer(char **buffer,int *buffer_max)
{
    int new_max;
    char *new_buffer;

    new_max = (*buffer_max)+(*buffer_max)/5;
    new_buffer = cst_alloc(char,new_max);
    memmove(new_buffer,*buffer,*buffer_max);
    cst_free(*buffer);
    *buffer = new_buffer;
    *buffer_max = new_max;
}			  

cst_tokenstream *new_tokenstream()
{   /* Constructor function */
    cst_tokenstream *ts = cst_alloc(cst_tokenstream,1);
    ts->fd = NULL;
    ts->file_pos = 0;
    ts->line_number = 0;
    ts->string_buffer = NULL;
    ts->token_pos = 0;
    ts->whitespace = cst_alloc(char,TS_BUFFER_SIZE);
    ts->ws_max = TS_BUFFER_SIZE;
    ts->prepunctuation = cst_alloc(char,TS_BUFFER_SIZE);
    ts->prep_max = TS_BUFFER_SIZE;
    ts->token = cst_alloc(char,TS_BUFFER_SIZE);
    ts->token_max = TS_BUFFER_SIZE;
    ts->postpunctuation = cst_alloc(char,TS_BUFFER_SIZE);
    ts->postp_max = TS_BUFFER_SIZE;

    ts->whitespacesymbols = ts_default_whitespacesymbols;
    ts->singlecharsymbols = ts_default_singlecharsymbols;
    ts->prepunctuationsymbols = ts_default_prepunctuationsymbols;
    ts->postpunctuationsymbols = ts_default_postpunctuationsymbols;
    ts->current_char = 0;

    return ts;
}

void delete_tokenstream(cst_tokenstream *ts)
{
    cst_free(ts->whitespace);
    cst_free(ts->prepunctuation);
    cst_free(ts->token);
    cst_free(ts->postpunctuation);
    cst_free(ts);
}

cst_tokenstream *ts_open(const char *filename)
{
    cst_tokenstream *ts = new_tokenstream();

#ifndef UNDER_CE
    if (cst_streq("-",filename))
	ts->fd = stdin;
    else
#endif
	ts->fd = cst_fopen(filename,CST_OPEN_READ|CST_OPEN_BINARY);
    ts_getc(ts);

    if (ts->fd == NULL)
    {
	delete_tokenstream(ts);
	return NULL;
    }
    else
	return ts;
}

cst_tokenstream *ts_open_string(const char *string)
{
    cst_tokenstream *ts = new_tokenstream();

    ts->string_buffer = cst_strdup(string);
    ts_getc(ts);

    return ts;
}

void ts_close(cst_tokenstream *ts)
{
    if (ts->fd != NULL)
    {
#ifndef UNDER_CE
	if (ts->fd != stdin)
#endif
	    cst_fclose(ts->fd);
	ts->fd = NULL; /* just in case close gets called twice */
    }
    if (ts->string_buffer != NULL)
    {
	cst_free(ts->string_buffer);
	ts->string_buffer = NULL;
    }
    delete_tokenstream(ts);
}

static void get_token_sub_part(cst_tokenstream *ts,
			       const char *charclass,
			       char **buffer,
			       int *buffer_max)
{
    int p;

    for (p=0; ((strchr(charclass,ts->current_char) != NULL) &&
	       (strchr(ts->singlecharsymbols,ts->current_char) == NULL) &&
	       (ts->current_char != EOF)); p++)
    {
	if (p >= *buffer_max) extend_buffer(buffer,buffer_max);
	(*buffer)[p] = ts->current_char;
	ts_getc(ts);
    }
    (*buffer)[p] = '\0';
}

/* Can't afford dynamically generate this char class so have separater func */
static void get_token_sub_part_2(cst_tokenstream *ts,
			       const char *endclass1,
			       char **buffer,
			       int *buffer_max)
{
    int p;

    for (p=0; ((strchr(endclass1,ts->current_char) == NULL) &&
	       (strchr(ts->singlecharsymbols,ts->current_char) == NULL) &&
	       (ts->current_char != EOF)); p++)
    {
	if (p >= *buffer_max) extend_buffer(buffer,buffer_max);
	(*buffer)[p] = ts->current_char;
	ts_getc(ts);
    }
    (*buffer)[p] = '\0';
}

static void get_token_postpunctuation(cst_tokenstream *ts)
{
    int p,t;

    t = strlen(ts->token);
    for (p=t;
	 (p > 0) && (strchr(ts->postpunctuationsymbols,ts->token[p]) != NULL);
	 p--);

    if (t != p)
    {
	if (t-p >= ts->postp_max) 
	    extend_buffer(&ts->postpunctuation,&ts->postp_max);
	/* Copy postpunctuation from token */
	memmove(ts->postpunctuation,&ts->token[p+1],(t-p));
	/* truncate token at postpunctuation */
	ts->token[p+1] = '\0';
    }
}

int ts_eof(cst_tokenstream *ts)
{
    if (ts->current_char == EOF)
	return TRUE;
    else
	return FALSE;
}

static const char ts_getc(cst_tokenstream *ts)
{
    if (ts->fd)
    {
	ts->current_char = cst_fgetc(ts->fd);
    }
    else if (ts->string_buffer)
    {
	if (ts->string_buffer[ts->file_pos] == '\0')
	    ts->current_char = EOF;
	else
	    ts->current_char = ts->string_buffer[ts->file_pos];
    }
    
    if (ts->current_char != EOF)
	ts->file_pos++;
    if (ts->current_char == '\n')
	ts->line_number++;
    return ts->current_char;
}

const char *ts_get_quoted_token(cst_tokenstream *ts,
				char quote,
				char escape)
{
    /* for reading the next quoted token that starts with quote and
       ends with quote, quote may appear only if preceded by escape */
    int l;
    char e[3];

    e[0] = quote;
    e[1] = escape;
    e[2] = '\0';

    /* skipping whitespace */
    get_token_sub_part(ts,ts->whitespacesymbols,
		       &ts->whitespace,
		       &ts->ws_max);
    ts->token_pos = ts->file_pos - 1;

    if (ts->current_char == quote)
    {   /* go until quote */
	ts_getc(ts);
	l=0;
	while (!ts_eof(ts))
	{
	    get_token_sub_part_2(ts,e,&ts->token,&ts->token_max);
	    if (ts->current_char == escape)
	    {
		ts_getc(ts);
		l = strlen(ts->token);
		if (l+1 >= ts->token_max) 
		    extend_buffer(&ts->token,&ts->token_max);
		ts->token[l] = ts->current_char;
		ts->token[l+1] = '\0';
		ts_getc(ts);
	    }
	    else
		break;
	}
	ts_getc(ts);
    }
    else /* its not quotes, like to be careful dont you */
    {    /* treat is as standard token                  */
	/* Get prepunctuation */
	get_token_sub_part(ts,ts->prepunctuationsymbols,
			   &ts->prepunctuation,
			   &ts->prep_max);
	/* Get the symbol itself */
	if (strchr(ts->singlecharsymbols,ts->current_char) != NULL)
	{
	    if (2 >= ts->token_max) extend_buffer(&ts->token,&ts->token_max);
	    ts->token[0] = ts->current_char;
	    ts->token[1] = '\0';
	    ts_getc(ts);
	}
	else
	    get_token_sub_part_2(ts,
				 ts->whitespacesymbols,       /* end class1 */
				 &ts->token,
				 &ts->token_max);
	/* This'll have token *plus* post punctuation in ts->token */
	/* Get postpunctuation */
	get_token_postpunctuation(ts);
    }

    return ts->token;
}


const char *ts_get(cst_tokenstream *ts)
{
    /* Get next token */

    /* Skip whitespace */
    get_token_sub_part(ts,ts->whitespacesymbols,
		       &ts->whitespace,
		       &ts->ws_max);

    /* quoted strings currently ignored */
    ts->token_pos = ts->file_pos - 1;
	
    /* Get prepunctuation */
    get_token_sub_part(ts,ts->prepunctuationsymbols,
		       &ts->prepunctuation,
		       &ts->prep_max);
    /* Get the symbol itself */
    if (strchr(ts->singlecharsymbols,ts->current_char) != NULL)
    {
	if (2 >= ts->token_max) extend_buffer(&ts->token,&ts->token_max);
	ts->token[0] = ts->current_char;
	ts->token[1] = '\0';
	ts_getc(ts);
    }
    else
	get_token_sub_part_2(ts,
			     ts->whitespacesymbols,       /* end class1 */
			     &ts->token,
			     &ts->token_max);
    /* This'll have token *plus* post punctuation in ts->token */
    /* Get postpunctuation */
    get_token_postpunctuation(ts);

    return ts->token;
}
