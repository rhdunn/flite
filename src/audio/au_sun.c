/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
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
/*  Sun/Solaris audio support                                            */
/*                                                                       */
/*************************************************************************/
#ifdef CST_AUDIO_SUNOS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/filio.h>
#include <sys/audioio.h>
#include "cst_string.h"
#include "cst_wave.h"
#include "cst_audio.h"

static const char *sun_audio_device = "/dev/audio";

int audio_set_sample_rate_sun(int afd,int sample_rate)
{
    audio_info_t ainfo;

    ioctl(afd,AUDIO_GETINFO,&ainfo);
    
    ainfo.play.encoding = AUDIO_ENCODING_LINEAR;
    ainfo.play.precision = 16;
    ainfo.play.channels = 1;
    ainfo.play.sample_rate = sample_rate;

    if (ioctl(afd,AUDIO_SETINFO,&ainfo) == -1)
	return FALSE;
    else
	return TRUE;;
}

int audio_open_sun()
{
    int r;
    r = open(sun_audio_device,O_WRONLY);
    if (r == -1)
	cst_errmsg("sun_audio: failed to open audio device %s\n",
		   sun_audio_device);
    return r;
}

int audio_close_sun(int fd)
{
    return close(fd);
}

int audio_write_sun(int afd, void *samples, int num_bytes)
{
    return write(afd,samples,num_bytes);
}

int audio_flush_sun(int afd)
{
    return ioctl(afd, AUDIO_DRAIN, 0);
}

#endif
