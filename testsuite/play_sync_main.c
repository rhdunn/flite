/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 2000                             */
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
/*  Play waveform displaying text as we go (testing callback events)     */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include "cst_hrg.h"
#include "cst_wave.h"
#include "cst_audio.h"

static int my_call_back(cst_item *i)
{
    const char *name;

    name = item_feat_string(i,"name");
    if (cst_streq(name,"__silence__"))
	printf("\n");
    else
    {
/*	printf("%s %f\n",name,item_feat_float(i,"end")); */
	printf("%s ",name);
	fflush(stdout);
    }

    return CST_OK_FORMAT;
}

int main(int argc, char **argv)
{
    cst_wave *w;
    cst_relation *r;

    if (argc != 3)
    {
	fprintf(stderr,"usage: play_wave_sync WAVEFILE LABELFILE\n");
	return 1;
    }

    w = new_wave();
    if (cst_wave_load_riff(w,argv[1]) != CST_OK_FORMAT)
	return -1;
    r = new_relation("FOO");
    if (relation_load(r,argv[2]) != CST_OK_FORMAT)
	return -1;
    
    play_wave_sync(w,r,my_call_back);

    return 0;
}
