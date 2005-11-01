/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2001                            */
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
/*               Date:  January 2001                                     */
/*************************************************************************/
/*                                                                       */
/*  Signal processing functions                                          */
/*                                                                       */
/*************************************************************************/

#include "cst_math.h"
#include "cst_hrg.h"
#include "cst_wave.h"
#include "cst_sigpr.h"
#include "cst_sts.h"

cst_wave *lpc_resynth(cst_lpcres *lpcres)
{
    cst_wave *w;
    int i,j,r,o,k;
    int ci,cr;
    float *outbuf, *lpccoefs;
    int pm_size_samps;
    float pp = 0;

    /* Get a new wave to build the signal into */
    w = new_wave();
    cst_wave_resize(w,lpcres->num_samples * lpcres->residual_fold,1);
    w->sample_rate = lpcres->sample_rate;
    /* outbuf is a circular buffer with past relevant samples in it */
    outbuf = cst_alloc(float,1+lpcres->num_channels);
    /* unpacked lpc coefficients */
    lpccoefs = cst_alloc(float,lpcres->num_channels);

    for (r=0,o=lpcres->num_channels,i=0; i < lpcres->num_frames; i++)
    {
	pm_size_samps = lpcres->sizes[i] * lpcres->residual_fold;

	/* Unpack the LPC coefficients */
	for (k=0; k<lpcres->num_channels; k++)
	{
	    lpccoefs[k] = (float)((((double)lpcres->frames[i][k])/65535.0)*
			   lpcres->lpc_range) + lpcres->lpc_min;
	}
	/* Note we don't zero the lead in from the previous part */
	/* seems like you should but it makes it worse if you do */
/*	memset(outbuf,0,sizeof(float)*(1+lpcres->num_channels)); */

	/* resynthesis the signal */
	for (j=0; j < pm_size_samps; j++,r++)
	{
	    outbuf[o] = (float)cst_ulaw_to_short(lpcres->residual[r/lpcres->residual_fold]);
	    cr = (o == 0 ? lpcres->num_channels : o-1);
	    for (ci=0; ci < lpcres->num_channels; ci++)
	    {
		outbuf[o] += lpccoefs[ci] * outbuf[cr];
		cr = (cr == 0 ? lpcres->num_channels : cr-1);
	    }
	    w->samples[r] = (short)(outbuf[o] + pp*lpcres->post_emphasis); 
	    pp = outbuf[o];
	    o = (o == lpcres->num_channels ? 0 : o+1);
	}
    }

    cst_free(outbuf);
    cst_free(lpccoefs);

    return w;

}

cst_wave *lpc_resynth_windows(cst_lpcres *lpcres)
{
    cst_wave *w;
    int i,j,r,o,k;
    int ci,cr;
    float *outbuf, *lpccoefs;
    int pm_size_samps;
    float pp = 0;

    /* Get a new wave to build the signal into */
    w = new_wave();
    cst_wave_resize(w,lpcres->num_samples * lpcres->residual_fold,1);
    w->sample_rate = lpcres->sample_rate;
    /* outbuf is a circular buffer with past relevant samples in it */
    outbuf = cst_alloc(float,1+lpcres->num_channels);
    /* unpacked lpc coefficients */
    lpccoefs = cst_alloc(float,lpcres->num_channels);

    for (r=0,o=lpcres->num_channels,i=0; i < lpcres->num_frames; i++)
    {
	pm_size_samps = lpcres->sizes[i] * lpcres->residual_fold;

	/* Unpack the LPC coefficients */
	for (k=0; k<lpcres->num_channels; k++)
	{
	    lpccoefs[k] = ((float)(((double)lpcres->frames[i][k])/65535.0)*
			   lpcres->lpc_range) + lpcres->lpc_min;
	}
	memset(outbuf,0,sizeof(float)*(1+lpcres->num_channels)); 

	/* resynthesis the signal */
	for (j=0; j < pm_size_samps; j++,r++)
	{
	    outbuf[o] = (float)cst_ulaw_to_short(lpcres->residual[r / lpcres->residual_fold]);
	    cr = (o == 0 ? lpcres->num_channels : o-1);
	    for (ci=0; ci < lpcres->num_channels; ci++)
	    {
		outbuf[o] += lpccoefs[ci] * outbuf[cr];
		cr = (cr == 0 ? lpcres->num_channels : cr-1);
	    }
	    w->samples[r] = (short)(outbuf[o] + pp*lpcres->post_emphasis); 
/*	    w->samples[r] = (short)outbuf[o] - pp*0.95; */
	    pp = outbuf[o];
	    o = (o == lpcres->num_channels ? 0 : o+1);
	}
    }

    cst_free(outbuf);
    cst_free(lpccoefs);

    return w;

}

