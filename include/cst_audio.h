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
/*               Date:  August 2000                                      */
/*************************************************************************/
/*                                                                       */
/*  Audio                                                                */
/*                                                                       */
/*************************************************************************/
#ifndef _CST_AUDIO_H__
#define _CST_AUDIO_H__

#include "cst_wave.h"
#include "cst_hrg.h"

#define CST_AUDIOBUFFSIZE 128
#define CST_AUDIO_DEFAULT_PORT 1746
#define CST_AUDIO_DEFAULT_SERVER "localhost"
#define CST_AUDIO_DEFAULT_ENCODING "short"

/* Generic audio functions */
int audio_open();
int audio_close(int afd);
int audio_set_sample_rate(int afd,int sample_rate);
int audio_write(int afd,void *buff,int num_bytes);
int audio_flush(int afd);

/* Generic high level audio functions */
int play_wave(cst_wave *w);
int play_wave_sync(cst_wave *w, cst_relation *rel,
		   int (*call_back)(cst_item *));

int play_wave_client(cst_wave *w,const char *servername,int port,
		     const char *encoding);

/* Be a server of given port */
int auserver(int port);

#endif
