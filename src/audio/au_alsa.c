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
/*             Author:  Geoff Harrison (mandrake@cepstral.com)           */
/*               Date:  Sepetember 2001                                  */
/*************************************************************************/
/*                                                                       */
/*  Access to ALSA audio devices                                          */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "cst_string.h"
#include "cst_wave.h"
#include "cst_audio.h"

#include <sys/asoundlib.h>

#include <sys/stat.h>
#include <fcntl.h>

static int alsa_card = 0, alsa_device = 0;

cst_audiodev *audio_open_alsa(int sps, int channels, cst_audiofmt fmt)
{
    snd_pcm_channel_info_t pinfo;
    snd_pcm_channel_params_t params;
    snd_pcm_channel_setup_t setup;
    snd_pcm_t *pcm;
    cst_audiodev *ad;
    int err;

#ifdef __QNXNTO__
    if (snd_pcm_open_preferred(&pcm,&alsa_card,&alsa_device,SND_PCM_OPEN_PLAYBACK) < 0)
    {
	cst_errmsg("alsa_audio: failed to open audio device\n");
	cst_error();
    }
    if (snd_pcm_plugin_set_disable(pcm,PLUGIN_DISABLE_MMAP) < 0)
    {
	cst_errmsg("alsa_audio: failed to disable mmap\n");
	snd_pcm_close(pcm);
	cst_error();
    }
#else
    if (snd_pcm_open(&pcm,alsa_card,alsa_device,SND_PCM_OPEN_PLAYBACK) < 0)
    {
	cst_errmsg("alsa_audio: failed to open audio device\n");
	cst_error();
    }
#endif


    memset(&pinfo, 0, sizeof(pinfo));
    memset(&params, 0, sizeof(params));
    memset(&setup, 0, sizeof(setup));

    pinfo.channel = SND_PCM_CHANNEL_PLAYBACK;
    snd_pcm_plugin_info(pcm,&pinfo);

    params.mode = SND_PCM_MODE_BLOCK;
    params.channel = SND_PCM_CHANNEL_PLAYBACK;
    params.start_mode = SND_PCM_START_DATA;
    params.stop_mode = SND_PCM_STOP_STOP;

    params.buf.block.frag_size = pinfo.max_fragment_size;
    params.buf.block.frags_max = 1;
    params.buf.block.frags_min = 1;
    
    params.format.interleave = 1;
    params.format.rate = sps;
    params.format.voices = channels;

    switch (fmt)
    {
    case CST_AUDIO_LINEAR16:
	if (CST_LITTLE_ENDIAN)
	    params.format.format = SND_PCM_SFMT_S16_LE;
	else
	    params.format.format = SND_PCM_SFMT_S16_BE;
	break;
    case CST_AUDIO_LINEAR8:
	params.format.format = SND_PCM_SFMT_U8;
	break;
    case CST_AUDIO_MULAW:
	params.format.format = SND_PCM_SFMT_MU_LAW;
	break;
    }

    if((err = snd_pcm_plugin_params(pcm,&params)) < 0)
    {
	cst_errmsg("alsa_audio params setting failed: %s\n",snd_strerror(err));
	snd_pcm_close(pcm);	
	cst_error();
    }
    if((err = snd_pcm_plugin_setup(pcm,SND_PCM_CHANNEL_PLAYBACK)) > 0) {
	cst_errmsg("alsa_audio sound prepare setting failed: %s\n",snd_strerror(err));
	snd_pcm_close(pcm);
	cst_error();
    }
    if((err = snd_pcm_plugin_prepare(pcm,SND_PCM_CHANNEL_PLAYBACK)) > 0) {
	cst_errmsg("alsa_audio sound prepare setting failed: %s\n",snd_strerror(err));
	snd_pcm_close(pcm);
	cst_error();
    }

    pinfo.channel = SND_PCM_CHANNEL_PLAYBACK;
    snd_pcm_plugin_info(pcm,&pinfo);

    ad = cst_alloc(cst_audiodev, 1);
    ad->platform_data = pcm;
    ad->sps = ad->real_sps = sps;
    ad->channels = ad->real_channels = channels;
    ad->fmt = ad->real_fmt = fmt;

    return ad;
}

int audio_close_alsa(cst_audiodev *ad)
{
    snd_pcm_t *pcm;

    if (ad == NULL)
	    return 0;

    pcm = ad->platform_data;
    snd_pcm_plugin_flush(pcm,0);
    snd_pcm_close(pcm);
    cst_free(ad);

    return 0;
}

int audio_write_alsa(cst_audiodev *ad, void *samples, int num_bytes)
{
    snd_pcm_t *pcm = ad->platform_data;

    return snd_pcm_plugin_write(pcm,samples,num_bytes);
}

int audio_flush_alsa(cst_audiodev *ad)
{
    snd_pcm_t *pcm = ad->platform_data;

    return snd_pcm_plugin_flush(pcm,0);
}

int audio_drain_alsa(cst_audiodev *ad)
{
    snd_pcm_t *pcm = ad->platform_data;

    return snd_pcm_plugin_playback_drain(pcm);
}

