/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2000                            */
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
/*  Waveforms                                                            */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cst_string.h"
#include "cst_wave.h"
#include "cst_file.h"

int cst_wave_save(cst_wave *w,const char *filename,const char *type)
{
    if (cst_streq(type,"riff"))
	return cst_wave_save_riff(w,filename);
/*    else if (cst_streq(type,"aiff"))
	return cst_wave_save_aiff(w,filename);
    else if (cst_streq(type,"snd"))
    return cst_wave_save_snd(w,filename); */
    else
    {
	cst_errmsg("cst_wave_save: unsupported wavetype \"%s\"\n",
		   type);
	return -1;
    }
}

int cst_wave_save_raw(cst_wave *w, const char *filename)
{
    cst_file fd;
    int rv;

    if ((fd = cst_fopen(filename,CST_OPEN_WRITE|CST_OPEN_BINARY)) == NULL)
    {
	cst_errmsg("cst_wave_save: can't open file \"%s\"\n",
		   filename);
	return -1;
    }

    rv = cst_wave_save_riff_fd(w, fd);
    cst_fclose(fd);

    return rv;
}

int cst_wave_save_raw_fd(cst_wave *w, cst_file fd)
{
    if (cst_fwrite(fd, cst_wave_samples(w),
		   sizeof(short), cst_wave_num_samples(w)) == cst_wave_num_samples(w))
	return 0;
    else
	return -1;
}


int cst_wave_save_riff(cst_wave *w,const char *filename)
{
    cst_file fd;
    int rv;

    if ((fd = cst_fopen(filename,CST_OPEN_WRITE|CST_OPEN_BINARY)) == NULL)
    {
	cst_errmsg("cst_wave_save: can't open file \"%s\"\n",
		   filename);
	return -1;
    }

    rv = cst_wave_save_riff_fd(w, fd);
    cst_fclose(fd);

    return rv;
}

int cst_wave_save_riff_fd(cst_wave *w, cst_file fd)
{
    char *info;
    short d_short;
    int d_int, n;
    int num_bytes;

    info = "RIFF";
    cst_fwrite(fd,info,4,1);
    num_bytes = (cst_wave_num_samples(w)
		 * cst_wave_num_channels(w)
		 * sizeof(short)) + 8 + 16 + 12;
    if (CST_BIG_ENDIAN) num_bytes = SWAPINT(num_bytes);
    cst_fwrite(fd,&num_bytes,4,1); /* num bytes in whole file */
    info = "WAVE";
    cst_fwrite(fd,info,1,4);
    info = "fmt ";
    cst_fwrite(fd,info,1,4);
    num_bytes = 16;                   /* size of header */
    if (CST_BIG_ENDIAN) num_bytes = SWAPINT(num_bytes);
    cst_fwrite(fd,&num_bytes,4,1);        
    d_short = RIFF_FORMAT_PCM;        /* sample type */
    if (CST_BIG_ENDIAN) d_short = SWAPSHORT(d_short);
    cst_fwrite(fd,&d_short,2,1);          
    d_short = cst_wave_num_channels(w); /* number of channels */
    if (CST_BIG_ENDIAN) d_short = SWAPSHORT(d_short);
    cst_fwrite(fd,&d_short,2,1);          
    d_int = cst_wave_sample_rate(w);  /* sample rate */
    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
    cst_fwrite(fd,&d_int,4,1);  
    d_int = (cst_wave_sample_rate(w)
	     * cst_wave_num_channels(w)
	     * sizeof(short));        /* average bytes per second */
    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
    cst_fwrite(fd,&d_int,4,1);
    d_short = (cst_wave_num_channels(w)
	       * sizeof(short));      /* block align */
    if (CST_BIG_ENDIAN) d_short = SWAPSHORT(d_short);
    cst_fwrite(fd,&d_short,2,1);          
    d_short = 2 * 8;                  /* bits per sample */
    if (CST_BIG_ENDIAN) d_short = SWAPSHORT(d_short);
    cst_fwrite(fd,&d_short,2,1);          
    info = "data";
    cst_fwrite(fd,info,1,4);
    d_int = (cst_wave_num_channels(w)
	     * cst_wave_num_samples(w)
	     * sizeof(short));	      /* bytes in data */
    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
    cst_fwrite(fd,&d_int,4,1);  

    if (CST_BIG_ENDIAN)
    {
        short *xdata = cst_alloc(short,cst_wave_num_channels(w)*
				 cst_wave_num_samples(w));
	memmove(xdata,cst_wave_samples(w),
		sizeof(short)*cst_wave_num_channels(w)*
		cst_wave_num_samples(w));
	swap_bytes_short(xdata,
			 cst_wave_num_channels(w)*
			 cst_wave_num_samples(w));
	n = cst_fwrite(fd,xdata,sizeof(short),
		       cst_wave_num_channels(w)*cst_wave_num_samples(w));
	cst_free(xdata);
    }
    else
    {
	n = cst_fwrite(fd,cst_wave_samples(w),sizeof(short),
		       cst_wave_num_channels(w)*cst_wave_num_samples(w));
    }

    if (n != cst_wave_num_channels(w)*cst_wave_num_samples(w))
	return -1;
    else
	return 0;
	
}

int cst_wave_load_raw(cst_wave *w,const char *filename,
		      const char *bo, int sample_rate)
{
    cst_file fd;
    int r;

    if ((fd = cst_fopen(filename,CST_OPEN_READ|CST_OPEN_BINARY)) == NULL)
    {
	cst_errmsg("cst_wave_load: can't open file \"%s\"\n",
		   filename);
	return -1;
    }

    r = cst_wave_load_raw_fd(w, fd, bo, sample_rate);
    
    cst_fclose(fd);
    
    return r;
}

int cst_wave_load_raw_fd(cst_wave *w, cst_file fd,
			 const char *bo, int sample_rate)
{
    long size;

    /* Won't work on pipes, tough luck... */
    size = cst_filesize(fd) / sizeof(short);
    cst_wave_resize(w, size, 1);
    if (cst_fread(fd, w->samples, sizeof(short), size) != size)
	return -1;

    w->sample_rate = sample_rate;
    if (bo) /* if it's NULL we don't care */
	if ((CST_LITTLE_ENDIAN && cst_streq(bo, BYTE_ORDER_BIG))
	    || (CST_BIG_ENDIAN && cst_streq(bo, BYTE_ORDER_LITTLE)))
	    swap_bytes_short(w->samples,w->num_samples);

    return 0;
}

int cst_wave_load_riff(cst_wave *w,const char *filename)
{
    cst_file fd;
    int r;

    if ((fd = cst_fopen(filename,CST_OPEN_READ|CST_OPEN_BINARY)) == NULL)
    {
	cst_errmsg("cst_wave_load: can't open file \"%s\"\n",
		   filename);
	return -1;
    }

    r = cst_wave_load_riff_fd(w,fd);
    
    cst_fclose(fd);
    
    return r;
}

int cst_wave_load_riff_fd(cst_wave *w,cst_file fd)
{
    char info[4];
    short d_short;
    int d_int, d;
    int hsize;
    int num_channels, samples, data_length;

    if (cst_fread(fd,info,1,4) != 4)
	return CST_WRONG_FORMAT;
    else if (strncmp(info,"RIFF",4) != 0)
	return CST_WRONG_FORMAT;
	
    cst_fread(fd,&d_int,4,1);
    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
    
    if ((cst_fread(fd,info,1,4) != 4) ||
	(strncmp(info,"WAVE",4) != 0))
	return CST_ERROR_FORMAT;

    if ((cst_fread(fd,info,1,4) != 4) ||
	(strncmp(info,"fmt ",4) != 0))
	return CST_ERROR_FORMAT;

    cst_fread(fd,&d_int,4,1);
    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
    hsize = d_int;
    cst_fread(fd,&d_short,2,1);
    if (CST_BIG_ENDIAN) d_short = SWAPSHORT(d_short);

    if (d_short != RIFF_FORMAT_PCM)
    {
	cst_errmsg("cst_load_wave_riff: unsupported sample format\n");
	return CST_ERROR_FORMAT;
    }
    cst_fread(fd,&d_short,2,1);
    if (CST_BIG_ENDIAN) d_short = SWAPSHORT(d_short);
    if (d_short != 1)
    {
	cst_errmsg("cst_load_wave_riff: can onlly support mono\n");
	return CST_ERROR_FORMAT;
    }
    num_channels = d_short;

    cst_fread(fd,&d_int,4,1);
    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
    cst_wave_set_sample_rate(w,d_int);     /* sample rate */
    cst_fread(fd,&d_int,4,1);              /* avg bytes per second */
    cst_fread(fd,&d_short,2,1);            /* block align */
    cst_fread(fd,&d_short,2,1);            /* bits per sample */
    
    cst_fseek(fd,cst_ftell(fd)+(hsize-16),CST_SEEK_ABSOLUTE); /* skip rest of header */

    /* Note there's a bunch of potential random headers */
    while (1)
    {
	if (cst_fread(fd,info,1,4) != 4)
	    return CST_ERROR_FORMAT;
	if (strncmp(info,"data",4) == 0)
	{
	    cst_fread(fd,&d_int,4,1);
	    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
	    samples = d_int/sizeof(short);
	    break;
	}
	else if (strncmp(info,"fact",4) == 0)
	{   
	    cst_fread(fd,&d_int,4,1);
	    if (CST_BIG_ENDIAN) d_int = SWAPINT(d_int);
	    cst_fseek(fd,cst_ftell(fd)+d_int,CST_SEEK_ABSOLUTE);
	}
	else 
	{
	    cst_errmsg("cst_wave_load_riff: unsupported chunk type \"%*s\"\n",
		       4,info);
	    return CST_ERROR_FORMAT;
	}
    }

    /* Now read the data itself */
    data_length = samples * num_channels;
    cst_wave_resize(w,samples,num_channels);

    if ((d = cst_fread(fd,w->samples,sizeof(short),data_length)) != data_length)
    {
	cst_errmsg("cst_wave_load_riff: %d missing samples, resized accordingly\n",
		   data_length-d);
	w->num_samples = d;
    }

    if (CST_BIG_ENDIAN)
	swap_bytes_short(w->samples,w->num_samples);

    return CST_OK_FORMAT;
}
