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
/*               Date:  October 2000                                     */
/*************************************************************************/
/*                                                                       */
/*  Access to audio devices                                   ,          */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cst_string.h"
#include "cst_wave.h"
#include "cst_audio.h"
#include "native_audio.h"

int audio_open()
{
    return AUDIO_OPEN_NATIVE();
}
int audio_close(int fd)
{
    return AUDIO_CLOSE_NATIVE(fd);
}
int audio_set_sample_rate(int afd, int sample_rate)
{
    return AUDIO_SET_SAMPLE_RATE_NATIVE(afd,sample_rate);
}
int audio_write(int afd,void *buff,int num_bytes)
{
    return AUDIO_WRITE_NATIVE(afd,buff,num_bytes);
}
int audio_flush(int afd)
{
    return AUDIO_FLUSH_NATIVE(afd);
}

int play_wave(cst_wave *w)
{
    int audiofd,i,n,r;

    if (!w)
	return CST_ERROR_FORMAT;
    
    if ((audiofd = audio_open()) == -1)
	return CST_ERROR_FORMAT;

    if (audio_set_sample_rate(audiofd,w->sample_rate) == -1)
    {
	cst_errmsg("can't set sample rate to %d\n",w->sample_rate);
	audio_close(audiofd);
	return CST_ERROR_FORMAT;
    }

    for (i=0; i < w->num_samples; i += r/2)
    {
	if (w->num_samples > i+CST_AUDIOBUFFSIZE)
	    n = CST_AUDIOBUFFSIZE;
	else
	    n = w->num_samples-i;
	r = audio_write(audiofd,&w->samples[i],n*2);
	if (r <= 0)
	{
	    if (n == 1) /* feature in the iPaq audio driver... */
		break;
	    else
		cst_errmsg("failed to write %d samples\n",n);
	}
    }

    audio_close(audiofd);

    return CST_OK_FORMAT;
}

int play_wave_sync(cst_wave *w, cst_relation *rel,
		   int (*call_back)(cst_item *))
{
    int audiofd,q,i,n,r;
    float r_pos;
    cst_item *item;

    if (!w)
	return CST_ERROR_FORMAT;
    
    if ((audiofd = audio_open()) == -1)
	return CST_ERROR_FORMAT;

    if (audio_set_sample_rate(audiofd,w->sample_rate) == -1)
    {
	cst_errmsg("can't set sample rate to %d\n",w->sample_rate);
	audio_close(audiofd);
	return CST_ERROR_FORMAT;
    }

    q=0;
    item = relation_head(rel);
    r_pos = (float) w->sample_rate * 0;
    for (i=0; i < w->num_samples; i += r/2)
    {
	if (i >= r_pos)
	{
	    audio_flush(audiofd);

	    if ((*call_back)(item) != CST_OK_FORMAT)
		break;
	    item = item_next(item);
	    if (item)
		r_pos = w->sample_rate * val_float(ffeature(item,"p.end"));
	    else
		r_pos = (float) w->num_samples;
	}
	if (w->num_samples > i+CST_AUDIOBUFFSIZE)
	    n = CST_AUDIOBUFFSIZE;
	else
	    n = w->num_samples-i;
	r = audio_write(audiofd,&w->samples[i],n*2);
	q +=r;
	if (r <= 0)
	{
	    if (n == 1) /* feature in the iPaq audio driver... */
		break;
	    else
		cst_errmsg("failed to write %d samples\n",n);
	}
    }

    audio_close(audiofd);

    return CST_OK_FORMAT;
}

