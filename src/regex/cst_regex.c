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
/*               Date:  January 2000                                     */
/*************************************************************************/
/*                                                                       */
/*  Regexes, this is just a front end to Henry Spencer's regex code      */
/*  Includes a mapping of fsf format regex's to hs format (escaping)     */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cst_alloc.h"
#include "regexp.h"
#include "cst_regex.h"

cst_regex *cst_rx_white = 0;
cst_regex *cst_rx_alpha = 0;
cst_regex *cst_rx_uppercase = 0;
cst_regex *cst_rx_lowercase = 0;
cst_regex *cst_rx_alphanum = 0;
cst_regex *cst_rx_identifier = 0;
cst_regex *cst_rx_int = 0;
cst_regex *cst_rx_double = 0;
cst_regex *cst_rx_commaint = 0;
cst_regex *cst_rx_digits = 0;
cst_regex *cst_rx_dotted_abbrev = 0;

/* For acces by const models */
cst_regex *cst_regex_table[1];

static int regex_init = 0;

static char *regularize(const char *unregex,int match);

void cst_regex_init()
{
    if (!regex_init)
    {
	cst_rx_white = new_cst_regex("[ \n\t\r]+");
	cst_rx_alpha = new_cst_regex("[A-Za-z]+");
	cst_rx_uppercase = new_cst_regex("[A-Z]+");
	cst_rx_lowercase = new_cst_regex("[a-z]+");
	cst_rx_alphanum = new_cst_regex("[0-9A-Za-z]+");
	cst_rx_identifier = new_cst_regex("[A-Za-z_][0-9A-Za-z_]+");
	cst_rx_int = new_cst_regex("-?[0-9]+");
	cst_rx_double = new_cst_regex("-?\\(\\([0-9]+\\.[0-9]*\\)\\|\\([0-9]+\\)\\|\\(\\.[0-9]+\\)\\)\\([eE][---+]?[0-9]+\\)?");
	cst_rx_commaint = new_cst_regex("[0-9][0-9]?[0-9]?,\\([0-9][0-9][0-9],\\)*[0-9][0-9][0-9]\\(\\.[0-9]+\\)?");
	cst_rx_digits = new_cst_regex("[0-9][0-9]*");
	cst_rx_dotted_abbrev = new_cst_regex("\\([A-Za-z]\\.\\)*[A-Za-z]");
	cst_regex_table[CST_RX_dotted_abbrev_NUM] = cst_rx_dotted_abbrev;
    }
    regex_init = 1;
}

int cst_regex_match(const cst_regex *r, const char *str)
{
    if (!regex_init) cst_regex_init();
    if (r)
	return hs_regexec((hs_regexp *)r->hs_reg,str);
    else
	return 0;
}

cst_regex *new_cst_regex(const char *str)
{
    cst_regex *r = cst_alloc(cst_regex,1);
    char *reg_str = regularize(str,1);

    r->hs_reg = (void *)hs_regcomp(reg_str);

    return r;
}

void delete_cst_regex(cst_regex *r)
{
    if (r)
    {
	cst_free(r->hs_reg);
	cst_free(r);
    }
}

/* These define the different escape conventions for the FSF's */
/* regexp code and Henry Spencer's */

static const char *fsf_magic="^$*+?[].\\";
static const char *fsf_magic_backslashed="()|<>";
static const char *spencer_magic="^$*+?[].()|\\\n";
static const char *spencer_magic_backslashed="<>";

/* Adaptation of rjc's mapping of fsf format to henry spencer's format */
/* of escape sequences, as taken from EST_Regex.cc in EST              */
static char *regularize(const char *unregex,int match)
{
    char *reg = cst_alloc(char, strlen(unregex)*2+3);
    char *r=reg;
    const char *e;
    int magic=0,last_was_bs=0;
    const char * in_brackets=NULL;
    const char *ex = (unregex?unregex:"");

    if (match && *ex != '^')
	*(r++) = '^';

    for(e=ex; *e ; e++)
    {
	if (*e == '\\' && !last_was_bs)
	{
	    last_was_bs=1;
	    continue;
	}

	magic=strchr((last_was_bs?fsf_magic_backslashed:fsf_magic), *e)!=NULL;

	if (in_brackets)
	{
	    *(r++) = *e;
	    if (*e  == ']' && (e-in_brackets)>1)
		in_brackets=0;
	}
	else if (magic)
	{
	    if (strchr(spencer_magic_backslashed, *e))
		*(r++) = '\\';

	    *(r++) = *e;
	    if (*e  == '[')
		in_brackets=e;
	}
	else 
	{
	    if (strchr(spencer_magic, *e))
		*(r++) = '\\';

	    *(r++) = *e;
	}
	last_was_bs=0;
    }
  
    if (match && (e==ex || *(e-1) != '$'))
    {
	if (last_was_bs)
	    *(r++) = '\\';
	*(r++) = '$';
    }

    *r='\0';

    return reg;
}
