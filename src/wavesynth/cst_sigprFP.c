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
/*  Signal processing functions (Fixed Point)                            */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "cst_hrg.h"
#include "cst_wave.h"
#include "cst_sigpr.h"
#include "cst_sts.h"

cst_wave *lpc_resynth_fixedpoint(cst_lpcres *lpcres)
{
    /* The fixed point version, without floats */
    cst_wave *w;
    int i,j,r,o,k;
    int ci,cr;
    int *outbuf, *lpccoefs;
    int pm_size_samps, ilpc_min, ilpc_range;
    int pp = 0;

    /* Get a new wave to build the signal into */
    w = new_wave();
    cst_wave_resize(w,lpcres->num_samples * lpcres->residual_fold,1);
    w->sample_rate = lpcres->sample_rate;
    /* outbuf is a circular buffer with past relevant samples in it */
    outbuf = cst_alloc(int,1+lpcres->num_channels);
    /* unpacked lpc coefficients */
    lpccoefs = cst_alloc(int,lpcres->num_channels);
    ilpc_min = (int)(lpcres->lpc_min*32768.0);
    /* assume range is never > abs(16) */
    ilpc_range = (int)(lpcres->lpc_range*2048.0);

    for (r=0,o=lpcres->num_channels,i=0; i < lpcres->num_frames; i++)
    {
	pm_size_samps = lpcres->sizes[i] * lpcres->residual_fold;

	/* Unpack the LPC coefficients */
	for (k=0; k<lpcres->num_channels; k++)
	    lpccoefs[k]=((lpcres->frames[i][k]/2*ilpc_range)/2048+ilpc_min)/2;

	/* resynthesis the signal */
	for (j=0; j < pm_size_samps; j++,r++)
	{
	    outbuf[o] = (int)cst_ulaw_to_short(lpcres->residual[r / lpcres->residual_fold]);
	    outbuf[o] *= 16384;
	    cr = (o == 0 ? lpcres->num_channels : o-1);
	    for (ci=0; ci < lpcres->num_channels; ci++)
	    {
		outbuf[o] += lpccoefs[ci]*outbuf[cr];
		cr = (cr == 0 ? lpcres->num_channels : cr-1);
	    }
	    outbuf[o] /= 16384;
	    w->samples[r] = (short)outbuf[o]
		/* I'll have to re-think this for FP case */
		/* + (pp*lpcres->post_emphasis)/32768 */
		;
	    pp = outbuf[o];
	    o = (o == lpcres->num_channels ? 0 : o+1);
	}
    }

    cst_free(outbuf);
    cst_free(lpccoefs);

    return w;

}

