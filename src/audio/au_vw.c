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
/*  Access to "voxware" audio devices (Linux/FreeBSD etc),               */
/*  (sorry my naming convention is showing my age here, basic any        */
/*  generic audio on Linux/BSD such as OSS and ALSA)                     */
/*                                                                       */
/*************************************************************************/
#if defined(CST_AUDIO_LINUX) || defined(CST_AUDIO_FREEBSD)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "cst_string.h"
#include "cst_wave.h"
#include "cst_audio.h"

#ifdef CST_AUDIO_LINUX
/* Linux/voxware audio specific */
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#endif
#ifdef CST_AUDIO_FREEBSD
/* probably Net and Open too */
#include <machine/soundcard.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

static const char * const vw_audio_device = "/dev/dsp";

int audio_set_sample_rate_vw(int afd,int sample_rate)
{
    int fmt;
    int sfmts;
    int stereo=0;
    int channels=1;

    ioctl(afd,SNDCTL_DSP_RESET,0);
    ioctl(afd,SNDCTL_DSP_SPEED,&sample_rate);
    ioctl(afd,SNDCTL_DSP_STEREO,&stereo);
    ioctl(afd,SNDCTL_DSP_CHANNELS,&channels);
    ioctl(afd,SNDCTL_DSP_GETFMTS,&sfmts);

    if (sfmts == AFMT_U8)
	fmt = AFMT_U8;         // its really an 8 bit only device
    else if (CST_LITTLE_ENDIAN)
	fmt = AFMT_S16_LE;  
    else
	fmt = AFMT_S16_BE;  
    
    ioctl(afd,SNDCTL_DSP_SETFMT,&fmt);

    if (fmt == AFMT_U8)
	return -1;
    else
	return 0;
}

int audio_open_vw()
{
    int r;
    r = open(vw_audio_device,O_WRONLY);
    if (r == -1)
	cst_errmsg("vw_audio: failed to open audio device %s\n",
		   vw_audio_device);
    return r;
}

int audio_close_vw(int fd)
{
    return close(fd);
}

int audio_write_vw(int afd, void *samples, int num_bytes)
{
    return write(afd,samples,num_bytes);
}

int audio_flush_vw(int afd)
{
    return ioctl(afd, SNDCTL_DSP_SYNC);
}
#endif
