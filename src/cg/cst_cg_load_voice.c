/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2010                            */
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
/*             Author:  Alok Parlikar (aup@cs.cmu.edu)                   */
/*               Date:  April 2010                                       */
/*************************************************************************/
/*                                                                       */
/*  Load a clustergen voice from a file                                  */
/*                                                                       */
/*************************************************************************/

#include "flite.h"
#include "cst_cg.h"
#include "cst_cg_map.h"

cst_voice *cst_cg_load_voice(const char *voxdir,
                             cst_val *voice_list,
                             cst_lang **lang_table)
{
    cst_voice *vox;
    cst_lexicon *lex = NULL;
    int fd;
    int voice_feature_count;
    int i;
    const char* language;
    cst_cg_db *cg_db;
    char* fname;
    char* fval;
    cst_filemap *vd;

    vd = cst_mmap_file(voxdir);
    if (vd == NULL)
    {
	printf("file: %s\n",voxdir);
	perror("Opening voice data");
	return NULL;
    }
    if (cst_cg_map_init(vd) == NULL)
    {
	perror("mmap failed");
        cst_munmap_file(vd);
	return NULL;
    }

    /* Load up cg_db from external file */
    cg_db = mapreader_load_db();

    if (cg_db == NULL)
    {
        cst_munmap_file(vd);
        return NULL;
    }

    vox = new_voice();

    /* Read voice features from the external file */
    voice_feature_count = mapreader_read_int();
    for(i=0;i<voice_feature_count;i++)
    {
	mapreader_read_voice_feature(&fname, &fval);
	printf("Setting feature %s: %s\n",fname, fval);
	flite_feat_set_string(vox->features,fname, fval);
    }

    /* Use the language feature to initialize the correct voice */
    language = flite_get_param_string(vox->features, "language", "");
    printf("Found language: %s\n",language);

    /* Search Lang table for lang_init() and lex_init(); */
    for (i=0; lang_table[i]; i++)
    {
        if (cst_streq(language,lang_table[i]->lang))
        {
            (lang_table[i]->lang_init)(vox);
            lex = (lang_table[i]->lex_init)();
            break;
        }
    }
    if (lex == NULL)
    {   /* Language is not supported */
	/* Delete allocated memory in cg_db */
	mapreader_free_db(cg_db);
	mapreader_finish();
	return NULL;	
    }
    
    /* Things that weren't filled in already. */
    flite_feat_set_string(vox->features,"name",cg_db->name);
    flite_feat_set_string(vox->features,"voxdir",path);
    flite_feat_set(voice->features,"voxdata",userdata_val(vd));
    
    flite_feat_set(vox->features,"lexicon",lexicon_val(lex));
    flite_feat_set(vox->features,"postlex_func",uttfunc_val(lex->postlex));

    /* No standard segment durations are needed as its done at the */
    /* HMM state level */
    flite_feat_set_string(vox->features,"no_segment_duration_model","1");
    flite_feat_set_string(vox->features,"no_f0_target_model","1");

    /* Waveform synthesis */
    flite_feat_set(vox->features,"wave_synth_func",uttfunc_val(&cg_synth));
    flite_feat_set(vox->features,"cg_db",cg_db_val(cg_db));
    flite_feat_set_int(vox->features,"sample_rate",cg_db->sample_rate);

    /* Add it to voice_list */

    return vox;
}

void cst_cg_unload_voice(cst_voice *vox,cst_val *voice_list)
{
    delete_voice(vox);
}

