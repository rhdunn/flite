/*************************************************************************/
/*                                                                       */
/*                           Cepstral, LLC                               */
/*                        Copyright (c) 2001                             */
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
/*  CEPSTRAL, LLC AND THE CONTRIBUTORS TO THIS WORK DISCLAIM ALL         */
/*  WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED       */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL         */
/*  CEPSTRAL, LLC NOR THE CONTRIBUTORS BE LIABLE FOR ANY SPECIAL,        */
/*  INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER          */
/*  RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION    */
/*  OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR  */
/*  IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.          */
/*                                                                       */
/*************************************************************************/
/*             Author:  David Huggins-Daines (dhd@cepstral.com)          */
/*               Date:  October 2001                                     */
/*************************************************************************/
/*                                                                       */
/* cst_sts.c: audio database (short-term-signals) management code.       */
/*                                                                       */
/*************************************************************************/

#include "cst_string.h"
#include "cst_val.h"
#include "cst_sts.h"
#include "cst_file.h"

CST_VAL_REGISTER_TYPE_NODEL(sts_list,cst_sts_list)

cst_sts_list *new_sts_list()
{
    cst_sts_list *l = cst_alloc(struct cst_sts_list_struct,1);
    return l;
}

void delete_sts_list(cst_sts_list *l)
{
    if (l)
    {
	cst_free(l);
    }
    return;
}

const unsigned short * get_sts_frame(const cst_sts_list *sts_list, int frame)
{
    
    if (sts_list->sts == NULL)
    {
	if (sts_list->frames->mem == NULL)
	{
	    unsigned short *data = cst_alloc(unsigned short, sts_list->num_channels);
	    cst_fseek(sts_list->frames->fh,
		      (frame * sts_list->num_channels * sizeof(unsigned short)),
		      CST_SEEK_ABSOLUTE);
	    cst_fread(sts_list->frames->fh, data, sizeof(unsigned short), sts_list->num_channels);
	    return data;
	}
	else
	    return (const unsigned short *)sts_list->frames->mem
		+ (frame * sts_list->num_channels);
    }
    else
	return sts_list->sts[frame].frame;
}

const unsigned char * get_sts_residual_fixed(const cst_sts_list *sts_list, int frame)
{
    if (sts_list->sts == NULL)
	if (sts_list->residuals->mem == NULL)
	{
	    unsigned char *data = cst_alloc(unsigned char, sts_list->num_channels);
	    cst_fseek(sts_list->residuals->fh, (frame * sts_list->num_channels),
		      CST_SEEK_ABSOLUTE);
	    cst_fread(sts_list->residuals->fh, data, 1, sts_list->num_channels);
	    return data;
	}
	else
	    return (const unsigned char *)sts_list->residuals->mem
		+ (frame * sts_list->num_channels);
    else
	return sts_list->sts[frame].residual;
}

static unsigned long mapped_frame_offset(const cst_sts_list *sts_list, int frame)
{
    /* This assumes that the voice compiler has generated an extra
           offset at the end of the array. */
    if (sts_list->resoffs->mem == NULL)
    {
	unsigned long off;

	cst_fseek(sts_list->resoffs->fh, frame*sizeof(unsigned long), CST_SEEK_ABSOLUTE);
	cst_fread(sts_list->resoffs->fh, &off, sizeof(off), 1);
	return off;
    }
    else
	return ((const unsigned long *)sts_list->resoffs->mem)[frame];
}

const unsigned char * get_sts_residual(const cst_sts_list *sts_list, int frame)
{
    if (sts_list->sts == NULL)
    {
	if (sts_list->frames->mem == NULL)
	{
	    unsigned char *data;
	    unsigned long a = mapped_frame_offset(sts_list, frame);
	    unsigned long b = mapped_frame_offset(sts_list, frame+1);

	    data = cst_alloc(unsigned char, b - a);
	    cst_fseek(sts_list->residuals->fh, a, CST_SEEK_ABSOLUTE);
	    cst_fread(sts_list->residuals->fh, data, 1, b - a);
	    return data;
	}
	else
	    return (const unsigned char *)sts_list->residuals->mem
		+ ((const unsigned long *)sts_list->resoffs->mem)[frame];
    }
    else
	return sts_list->sts[frame].residual;
}

int get_frame_size(const cst_sts_list *sts_list, int frame)
{
    if (sts_list->sts == NULL) {
	/* This assumes that the voice compiler has generated an extra
           offset at the end of the array. */
	if (sts_list->resoffs->mem == NULL)
	{
	    unsigned long offs[2];

	    cst_fseek(sts_list->resoffs->fh, frame*sizeof(unsigned long), CST_SEEK_ABSOLUTE);
	    cst_fread(sts_list->resoffs->fh, offs, sizeof(offs), 1);
	    return offs[1] - offs[0];
	}
	else
	    return ((const unsigned long *)sts_list->resoffs->mem)[frame+1]
		- ((const unsigned long *)sts_list->resoffs->mem)[frame];
    } else {
	return sts_list->sts[frame].size;
    }
}

int get_unit_size(const cst_sts_list *s,int start, int end)
{
    /* returns size (in samples) of unit */
    int i,size;

    for (i=start,size=0; i<end; i++)
	size += get_frame_size(s, i);

    return size;
}

void release_sts_frame(const cst_sts_list *sts_list, int frame,
		       const unsigned short *data)
{
    if (sts_list->sts == NULL && sts_list->frames->mem == NULL)
	cst_free((void *)data);
}

void release_sts_residual(const cst_sts_list *sts_list, int frame,
			  const unsigned char *data)
{
    if (sts_list->sts == NULL && sts_list->residuals->mem == NULL)
	cst_free((void *)data);
}
