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
/*  General unit functions (diphones or clunit)                          */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cst_hrg.h"
#include "cst_utt_utils.h"
#include "cst_wave.h"
#include "cst_track.h"
#include "cst_units.h"
#include "cst_sigpr.h"

static int nearest_pm(cst_sts_list *sts_list,int start,int end,float u_index);
void add_residual_windowed(int targ_size, 
			   unsigned char *targ_residual,
			   int unit_size, 
			   const unsigned char *unit_residual);

cst_utterance *join_units(cst_utterance *utt)
{
    /* Make a waveform form the units */
    const char *join_type;

    join_type = get_param_string(utt->features,"join_type", "modified_lpc");

    if (cst_streq(join_type,"none"))
	return utt;
#if 0
    else if (cst_streq(join_type,"windowed_join"))
	join_units_windowed(utt);
#endif
    else if (cst_streq(join_type,"simple_join"))
	join_units_simple(utt);
    else if (cst_streq(join_type,"modified_lpc"))
	join_units_modified_lpc(utt);
    
    return utt;
}

cst_utterance *join_units_simple(cst_utterance *utt)
{
    cst_wave *w;
    cst_lpcres *lpcres;
    const char *resynth_type;
    resynth_type = get_param_string(utt->features,"resynth_type", "float");
    
    asis_to_pm(utt);
    concat_units(utt);

    lpcres = val_lpcres(utt_feat_val(utt,"target_lpcres"));

    if (cst_streq(resynth_type, "float"))
	    w = lpc_resynth(lpcres); 
    else if (cst_streq(resynth_type, "fixed"))
	    w = lpc_resynth_fixedpoint(lpcres); 
    else {
	    cst_errmsg("unknown resynthesis type %s\n", resynth_type);
	    cst_error(); /* Should not happen */
    }

    utt_set_wave(utt,w);
    
    return utt;
}

cst_utterance *join_units_modified_lpc(cst_utterance *utt)
{
    cst_wave *w;
    cst_lpcres *lpcres;
    const char *resynth_type;
    resynth_type = get_param_string(utt->features,"resynth_type", "float");

    f0_targets_to_pm(utt);
    concat_units(utt);

    lpcres = val_lpcres(utt_feat_val(utt,"target_lpcres"));

    if (cst_streq(resynth_type, "float"))
	    w = lpc_resynth(lpcres); 
    else if (cst_streq(resynth_type, "fixed"))
	    w = lpc_resynth_fixedpoint(lpcres); 
    else {
	    cst_errmsg("unknown resynthesis type %s\n", resynth_type);
	    cst_error(); /* Should not happen */
    }

    utt_set_wave(utt,w);
    
    return utt;
}

cst_utterance *asis_to_pm(cst_utterance *utt)
{
    /* Copy the PM structure from the units unchanged */
    cst_item *u;
    cst_lpcres *target_lpcres;
    int unit_entry, unit_start, unit_end;
    int utt_pms, utt_size, i;
    cst_sts_list *sts_list;

    sts_list = val_sts_list(utt_feat_val(utt,"sts_list"));
    target_lpcres = new_lpcres();
    
    utt_pms = utt_size = 0;
    for (u=relation_head(utt_relation(utt,"Unit"));
	 u; 
	 u=item_next(u))
    {
	unit_entry = item_feat_int(u,"unit_entry");
	unit_start = item_feat_int(u,"unit_start");
	unit_end = item_feat_int(u,"unit_end");
	utt_size += get_unit_size(sts_list,unit_start,unit_end);
	utt_pms += unit_end - unit_start;
	item_set_int(u,"target_end",utt_size);
    }
    lpcres_resize_frames(target_lpcres,utt_pms);

    utt_pms = utt_size = 0;
    for (u=relation_head(utt_relation(utt,"Unit"));
	 u; 
	 u=item_next(u))
    {
	unit_entry = item_feat_int(u,"unit_entry");
	unit_start = item_feat_int(u,"unit_start");
	unit_end = item_feat_int(u,"unit_end");
	for (i=unit_start; i<unit_end; i++,utt_pms++)
	{
	    utt_size += get_frame_size(sts_list, i);
	    target_lpcres->times[utt_pms] = utt_size;
	}
    }
    utt_set_feat(utt,"target_lpcres",lpcres_val(target_lpcres));
    return utt;
}

cst_utterance *f0_targets_to_pm(cst_utterance *utt)
{
    cst_item *t;
    float pos,lpos,f0,lf0,m;
    double time;
    int pm;
    cst_sts_list *sts_list;
    cst_lpcres *target_lpcres;

    sts_list = val_sts_list(utt_feat_val(utt,"sts_list"));
    lpos = 0;
    lf0 = 120; /* hmm */
    pm = 0;
    time = 0;
    /* First pass to count how many pms will be required */
    for (t=relation_head(utt_relation(utt,"Target"));
	 t;
	 t=item_next(t))
    {
	pos = item_feat_float(t,"pos");
	f0 = item_feat_float(t,"f0");
	if (time == pos) continue;
	m = (f0-lf0)/(pos-lpos);
	for ( ; time < pos; pm++)
	{
	    time += 1/(lf0 + ((time-lpos)*m));
	}
    }
    target_lpcres = new_lpcres();
    lpcres_resize_frames(target_lpcres,pm);

    lpos = 0;
    lf0 = 120;
    pm = 0;
    time = 0;
    /* Second pass puts the values in */
    for (t=relation_head(utt_relation(utt,"Target"));
	 t;
	 t=item_next(t))
    {
	pos = item_feat_float(t,"pos");
	f0 = item_feat_float(t,"f0");
	if (time == pos) continue;
	m = (f0-lf0)/(pos-lpos);
	for ( ; time < pos; pm++)
	{
	    time += 1/(lf0 + ((time-lpos)*m));
	    target_lpcres->times[pm] = sts_list->sample_rate * time;
	}
    }
    utt_set_feat(utt,"target_lpcres",lpcres_val(target_lpcres));
    return utt;
}

cst_utterance *concat_units(cst_utterance *utt)
{
    cst_lpcres *target_lpcres;
    cst_item *u;
    int pm_i, unit_entry, unit_size, unit_start, unit_end;
    int rpos, pm_start, nearest_u_pm;
    int sample_rate;
    int target_end, target_start;
    float m, u_index;
    cst_sts_list *sts_list;
    const char *residual_type;

    residual_type = get_param_string(utt->features,"residual_type", "plain");
    sts_list = val_sts_list(utt_feat_val(utt,"sts_list"));
    target_lpcres = val_lpcres(utt_feat_val(utt,"target_lpcres"));
    sample_rate = sts_list->sample_rate;
    
    target_lpcres->lpc_min = sts_list->coeff_min;
    target_lpcres->lpc_range = sts_list->coeff_range;
    target_lpcres->num_channels = sts_list->num_channels;
    target_lpcres->sample_rate = sts_list->sample_rate;
    target_lpcres->post_emphasis = sts_list->post_emphasis;
    target_lpcres->residual_fold = sts_list->residual_fold;
    lpcres_resize_samples(target_lpcres,
			  target_lpcres->times[target_lpcres->num_frames-1]);
    
    sample_rate = sts_list->sample_rate;

    target_start = 0.0; rpos = 0; pm_i = 0; u_index = 0;
    for (u=relation_head(utt_relation(utt,"Unit")); u; u=item_next(u))
    {
	unit_entry = item_feat_int(u,"unit_entry");
	unit_start = item_feat_int(u,"unit_start");
	unit_end = item_feat_int(u,"unit_end");
	unit_size = get_unit_size(sts_list,unit_start,unit_end);
	target_end = item_feat_int(u,"target_end");
	
	u_index = 0;
	m = (float)unit_size/(float)(target_end-target_start);
/*	printf("unit_size %d start %d end %d tstart %d tend %d m %f\n",  
	unit_size, unit_start, unit_end, target_start, target_end, m); */
	for (pm_start=pm_i ; 
	     (pm_i < target_lpcres->num_frames) &&
		 (target_lpcres->times[pm_i] <= target_end);
	     pm_i++)
	{
	    nearest_u_pm = nearest_pm(sts_list,unit_start,unit_end,u_index);
	    /* Get LPC coefs (pointer) */
	    target_lpcres->frames[pm_i] = get_sts_frame(sts_list, nearest_u_pm);
	    /* Get residual (copy) */
	    target_lpcres->sizes[pm_i] =
		target_lpcres->times[pm_i] -
		(pm_i > 0 ? target_lpcres->times[pm_i-1] : 0);
	    if (cst_streq(residual_type,"pulse"))
		add_residual_pulse(target_lpcres->sizes[pm_i],
				   &target_lpcres->residual[rpos],
				   get_frame_size(sts_list, nearest_u_pm),
				   get_sts_residual(sts_list, nearest_u_pm));
	    /* But this requires particault layout of residuals which
	       probably isn't true */
	    /*
	    if (cst_streq(residual_type,"windowed"))
		add_residual_windowed(target_lpcres->sizes[pm_i],
				     &target_lpcres->residual[rpos],
				     get_frame_size(sts_list, nearest_u_pm),
				     get_sts_residual(sts_list, nearest_u_pm));
	    */
	    else
		add_residual(target_lpcres->sizes[pm_i],
			     &target_lpcres->residual[rpos],
			     get_frame_size(sts_list, nearest_u_pm),
			     get_sts_residual(sts_list, nearest_u_pm));
	    rpos+=target_lpcres->sizes[pm_i];
	    u_index += (float)target_lpcres->sizes[pm_i]*m;
	}
	target_start = target_end;
    }
    target_lpcres->num_frames = pm_i;
    return utt;
}

static int nearest_pm(cst_sts_list *sts_list, int start,int end,float u_index)
{
    /* First the pm in unit_entry that is closest to u_index */
    int i, i_size, n_size;
    i_size = 0;

    for (i=start; i < end; i++)
    {
	n_size = i_size + get_frame_size(sts_list, i);
	if (fabs((double)(u_index-(float)i_size)) <
	    fabs((double)(u_index-(float)n_size)))
	    return i;
	i_size = n_size;
    }

    return end-1;
}

void add_residual_windowed(int targ_size, 
			   unsigned char *targ_residual,
			   int unit_size, 
			   const unsigned char *unit_residual)
{
    /* Note this doesn't work unless the unit_residuals and consecutive */
#define DI_PI 3.14159265358979323846
    float *window, *unit, *residual;
    int i,j,k, offset, win_size;

    win_size = (targ_size*2)+1;
    window = cst_alloc(float,win_size);
    window[targ_size+1] = 1.0;
    k = DI_PI / (win_size - 1);
    for (i=0,j=win_size-1; i < targ_size+1; i++,j--)
	window[j] = window[i] = 0.54 - (0.46 * cos(k * i));

    residual = cst_alloc(float,win_size);
    for (i=0; i<win_size; i++)
	residual[i] = cst_ulaw_to_short(targ_residual[i]);

    unit = cst_alloc(float,(unit_size*2)+1);
    for (i=0; i<(unit_size*2)+1; i++)
	unit[i] = cst_ulaw_to_short(unit_residual[i]);

    if (targ_size < unit_size)
	for (i=0; i < win_size; i++)
	    residual[i] += window[i] * unit[i+(unit_size-targ_size)/2];
    else
    {
	offset = (targ_size-unit_size)/2;
	for (i=offset; i < win_size-offset; i++)
	    residual[i] += window[i] * unit[i-offset];
    }

    for (i=0; i < win_size; i++)
	targ_residual[i] = cst_short_to_ulaw((short)residual[i]);

    cst_free(window);
    cst_free(residual);
    cst_free(unit);

}

void add_residual(int targ_size, unsigned char *targ_residual,
		  int unit_size, const unsigned char *unit_residual)
{
/*    float pow_factor;
      int i; */

    if (unit_size < targ_size)
	memmove(&targ_residual[((targ_size-unit_size)/2)],
		&unit_residual[0],
		unit_size*sizeof(unsigned char));
    else
    {
	memmove(&targ_residual[0],
		&unit_residual[((unit_size-targ_size)/2)],
		targ_size*sizeof(unsigned char));
#if 0
    if (unit_size < targ_size)
	memmove(&targ_residual[0],
		&unit_residual[0],
		unit_size*sizeof(unsigned char));
    else
    {
	memmove(&targ_residual[0],
		&unit_residual[0],
		targ_size*sizeof(unsigned char));
#endif
	/* I can't hear any improvement with power fixes so I don't do them */
/*	pow_factor = (float)targ_size/(float)unit_size;
	for (i=0; i<targ_size; i++)
	    targ_residual[i] = 
	    cst_short_to_ulaw((short)(cst_ulaw_to_short(targ_residual[i])*pow_factor)); */
    }
}

void add_residual_pulse(int targ_size, unsigned char *targ_residual,
			int unit_size, const unsigned char *unit_residual)
{
    /* Unit residual isn't a pointed its a number, the power for the 
       the sts, yes this is hackily vasting the address to a number */

    if (unit_size < targ_size)
	targ_residual[((targ_size-unit_size)/2)] 
	    = cst_short_to_ulaw((short)(int)unit_residual);
    else
	targ_residual[((unit_size-targ_size)/2)] 
	    = cst_short_to_ulaw((short)(int)unit_residual);
}

