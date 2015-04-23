/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                      Copyright (c) 1999-2007                          */
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
/*               Date:  November 2007                                    */
/*************************************************************************/
/*                                                                       */
/*  A clustergen voice defintion			                 */
/*                                                                       */
/*************************************************************************/

#include <string.h>
#include "flite.h"
#include "cst_cg.h"

#include "usenglish.h"
#include "cmu_lex.h"

//#include "cmu_pashto_lang.h"
//#include "cmu_pashto_lex.h"


#include "mapreader.h"
#include <fcntl.h>
#include <unistd.h>

static cst_cg_db* cmu_us_generic_cg_db;

cst_voice *cmu_us_generic_cg = NULL;

cst_voice *register_cmu_us_generic_cg(const char *voxdir)
{
    cst_voice *vox;
    cst_lexicon *lex;
    int fd;
    int voice_feature_count;
    int i;
    char* language;

    if (cmu_us_generic_cg)
        return cmu_us_generic_cg;  /* Already registered */

    
    fd = open(voxdir,O_RDONLY);
    if(fd<0) 
      {
	printf("file: %s\n",voxdir);
	perror("Opening voice data");
	return NULL;
      }
    if(mapreader_init(fd) < 0)
      {
	perror("mmap failed");
	return NULL;
      }

    /* Load up cmu_us_generic_cg_db from external file */
    cmu_us_generic_cg_db = mapreader_load_db();

    if(cmu_us_generic_cg_db == NULL)
      return NULL;

    vox = new_voice();

    /* Read voice features from the external file */
    voice_feature_count = mapreader_read_int();
    for(i=0;i<voice_feature_count;i++)
      {
	char* fname;
	char* fval;
	mapreader_read_voice_feature(&fname, &fval);
	printf("Setting feature %s: %s\n",fname, fval);
	flite_feat_set_string(vox->features,fname, fval);
      }

    /* Use the language feature to initialize the correct voice */
    language = flite_get_param_string(vox->features, "language", "");
    printf("Found language: %s\n",language);

    if(strcmp(language, "eng")==0)
      {
	usenglish_init(vox);
	lex = cmu_lex_init();
      }
    /*
      else if(strcmp(language, "pus")==0)
      {
	cmu_pashto_lang_init(vox);
	lex = cmu_pashto_lex_init();
	}
    */
    else
      {
	/* Language is not supported */
	/* Delete allocated memory in cmu_us_generic_cg_db */
	mapreader_free_db(cmu_us_generic_cg_db);
	mapreader_finish();
	return NULL;	
      }
    
    /* Things that weren't filled in already. */
    flite_feat_set_string(vox->features,"name",cmu_us_generic_cg_db->name);

    
    flite_feat_set(vox->features,"lexicon",lexicon_val(lex));
    flite_feat_set(vox->features,"postlex_func",uttfunc_val(lex->postlex));

    /* No standard segment durations are needed as its done at the */
    /* HMM state level */
    flite_feat_set_string(vox->features,"no_segment_duration_model","1");
    flite_feat_set_string(vox->features,"no_f0_target_model","1");
    

    /* Waveform synthesis */
    flite_feat_set(vox->features,"wave_synth_func",uttfunc_val(&cg_synth));
    flite_feat_set(vox->features,"cg_db",cg_db_val(cmu_us_generic_cg_db));
    flite_feat_set_int(vox->features,"sample_rate",cmu_us_generic_cg_db->sample_rate);

    cmu_us_generic_cg = vox;

    return cmu_us_generic_cg;
}

void unregister_cmu_us_generic_cg(cst_voice *vox)
{
    if (vox != cmu_us_generic_cg)
	return;

    /* Delete allocated memory in cmu_us_generic_cg_db */
    mapreader_free_db(cmu_us_generic_cg_db);
    delete_voice(vox);
    mapreader_finish();
    
    cmu_us_generic_cg = NULL;
}

