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
/*  Simple top level program                                             */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "flite.h"
#include "flite_version.h"

cst_voice *register_cmu_us_kal();

static void flite_version()
{
    printf("  version: %s-%s-%s %s\n",
	   FLITE_PROJECT_PREFIX,
	   FLITE_PROJECT_VERSION,
	   FLITE_PROJECT_STATE,
	   FLITE_PROJECT_DATE);
    printf("  CMU Copyright 1999-2001, all rights reserved\n");
}

static void flite_usage()
{
    printf("flite: a small simple speech synthesizer\n");
    flite_version();
    printf("usage: flite TEXT/FILE [WAVEFILE]\n"
           "  Converts text in TEXTFILE to a waveform in WAVEFILE\n"
           "  If text contains a space the it is treated as a literal\n"
           "  textstring and spoken, and not as a file name\n"
           "  if WAVEFILE is unspecified or \"play\" the result is\n"
           "  played on the current systems audio device.  If WAVEFILE\n"
           "  is \"none\" the waveform is discarded (good for benchmarking)\n"
           "  Other options must appear before these options\n"
           "  --version   Output flite version number\n"
           "  --help      Output usage string\n"
           "  -o WAVEFILE Explicitly set output filename\n"
           "  -f TEXTFILE Explicitly set input filename\n"
           "  -t TEXT     Explicitly set input textstring\n"
           "  --set F=V   Set feature (guesses type)\n"
           "  -s F=V      Set feature (guesses type)\n"
           "  --seti F=V  Set int feature\n"
           "  --setf F=V  Set float feature\n"
           "  --sets F=V  Set string feature\n"
           "  -v          Verbose mode\n");
    exit(0);
}

static void ef_set(cst_features *f,const char *fv,const char *type)
{
    /* set feature from fv (F=V), guesses type if not explicit type given */
    const char *val;
    char *feat;

    if ((val = strchr(fv,'=')) == 0)
    {
	fprintf(stderr,
		"flite: can't find '=' in featval \"%s\", ignoring it\n",
		fv);
    }
    else
    {
	feat = cst_strdup(fv);
	feat[strlen(fv)-strlen(val)] = '\0';
	val = val+1;
	if ((type && cst_streq("int",type)) ||
	    ((type == 0) && (cst_regex_match(cst_rx_int,val))))
	    feat_set_int(f,feat,atoi(val));
	else if ((type && cst_streq("float",type)) ||
		 ((type == 0) && (cst_regex_match(cst_rx_double,val))))
	    feat_set_float(f,feat,atof(val));
	else
	    feat_set_string(f,feat,val);
	/* I don't free feat, because feats think they are const */
	/* which is true except in this particular case          */
    }
}

int main(int argc, char **argv)
{
    struct timeval tv;
    struct timezone tz; 
    cst_voice *v;
    const char *filename;
    const char *outtype;
    int i;
    float durs;
    double time_start, time_end;
    int flite_verbose, explicit_filename, explicit_text;
    cst_features *extra_feats;

    filename = 0;
    outtype = "play";   /* default is to play */
    flite_verbose = FALSE;
    explicit_text = explicit_filename = FALSE;
    extra_feats = new_features();

    flite_init();

    /* if (argc < 2) flite_usage(); */

    for (i=1; i<argc; i++)
    {
	if (cst_streq(argv[i],"--version"))
	{
	    flite_version();
	    return 1;
	}
	else if (cst_streq(argv[i],"-h") ||
		 cst_streq(argv[i],"--help") ||
		 cst_streq(argv[i],"-?"))
	    flite_usage();
	else if (cst_streq(argv[i],"-v"))
	    flite_verbose = TRUE;
	else if ((cst_streq(argv[i],"-o")) && (i+1 < argc))
	{
	    outtype = argv[i+1];
	    i++;
	}
	else if (cst_streq(argv[i],"-f") && (i+1 < argc))
	{
	    filename = argv[i+1];
	    explicit_filename = TRUE;
	    i++;
	}
	else if ((cst_streq(argv[i],"-set") || cst_streq(argv[i],"-s"))
		 && (i+1 < argc))
	{
	    ef_set(extra_feats,argv[i+1],0);
	    i++;
	}
	else if (cst_streq(argv[i],"--seti") && (i+1 < argc))
	{
	    ef_set(extra_feats,argv[i+1],"int");
	    i++;
	}
	else if (cst_streq(argv[i],"--setf") && (i+1 < argc))
	{
	    ef_set(extra_feats,argv[i+1],"float");
	    i++;
	}
	else if (cst_streq(argv[i],"--sets") && (i+1 < argc))
	{
	    ef_set(extra_feats,argv[i+1],"string");
	    i++;
	}
	else if (cst_streq(argv[i],"-t") && (i+1 < argc))
	{
	    filename = argv[i+1];
	    explicit_text = TRUE;
	    i++;
	}
	else if (filename)
	    outtype = argv[i];
	else
	    filename = argv[i];
    }

    if (filename == NULL) filename = "-";  /* stdin */
    v = register_cmu_us_kal();
    feat_copy_into(extra_feats,v->features);
    durs = 0.0;

    gettimeofday(&tv,&tz);
    time_start = (double)(tv.tv_sec)+(((double)tv.tv_usec)/1000000.0);

    if ((strchr(filename,' ') && !explicit_filename) || explicit_text)
	durs = flite_text_to_speech(filename,v,outtype);
    else
	durs = flite_file_to_speech(filename,v,outtype);

    gettimeofday(&tv,&tz);
    time_end = ((double)(tv.tv_sec))+((double)tv.tv_usec/1000000.0);

    if (flite_verbose)
	printf("%f seconds of speech synthesized in %f (%f)\n",
	       durs,
	       (float)(time_end-time_start),
	       durs/(float)(time_end-time_start));

    delete_features(extra_feats);

    return 0;
}
