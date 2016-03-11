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
/*             Author:  Alok Parlikar (aup@cs.cmu.edu)                   */
/*               Date:  March 2010                                       */
/*************************************************************************/
/*                                                                       */
/*  Utility for dumping a clustergen voice as an mmap'able file          */
/*                                                                       */
/*************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "cst_cg.h"
#include "cst_cart.h"

extern const cst_cg_db VOICE_DB_NAME ;

/* Write magic string */
void write_init(int fd)
{
  if(write(fd, "CMU_FLITE_CG_VOXDATA-v1.001\0",28) < 0)
    {
      perror("Could not write to file");
      exit(-1);
    }
}

/* To make it easier to read the file, the size of the data written is
   written as an integer and then the data itself is written.
   We make sure everything we write is in multiple of 4 bytes
   otherwise misalignments of pointers can couse SIGBUS upon
   reading the mmap file */
void write_padded(int fd, const void* data, int numbytes)
{
  int paddingbytes, totalbytes;
  paddingbytes=4-(numbytes%4);
  if(paddingbytes == 4) paddingbytes = 0;
  totalbytes = paddingbytes+numbytes;
  write(fd, &totalbytes, sizeof(int));
  write(fd, data, numbytes);
  write(fd, "padding",paddingbytes);
}

/* Write the data structure */

void write_db_info(int fd, const cst_cg_db* db)
{
  write_padded(fd, (void*)db, sizeof(cst_cg_db));
  write_padded(fd, (void*)db->name, strlen(db->name)+1);
}

/* Write the CG types */

void write_db_types(int fd, const cst_cg_db* db)
{
  int i;
  
  i=0;

  while(db->types[i])
    i++;

  write(fd, &i,sizeof(int));

  // Create the space for pointers. 
  // Upon loading the mmap file, these locations
  // will be updated to point to the right strings.
  // We need i+1 pointers
  write(fd, db->types, (i+1)*sizeof(char*)); 

  i=0;
  while(db->types[i])
    {
      write_padded(fd, (void*)(db->types[i]), strlen(db->types[i])+1);
      i++;
    }
}

/* Write the nodes of a cart tree */

void write_tree_nodes(int fd, const cst_cart_node* nodes)
{
  int i;
  i=0;
  cst_cart_node* node;

  while(nodes[i].val != 0)
    i++;
  
  // there are i+1 nodes that we must dump.
  write_padded(fd, nodes, (i+1)*sizeof(cst_cart_node));

  // now write node data. 
  i=0;
  while(nodes[i].val!=0)
    {
      node = &(nodes[i]);
      write_padded(fd,  node->val, sizeof(cst_val));
      if(node->val->c.a.type == CST_VAL_TYPE_STRING)
        write_padded(fd, node->val->c.a.v.vval, strlen(node->val->c.a.v.vval)+1);
      i++;
    }
}

/* Write the feats table of a cart tree */

void write_tree_feats(int fd, const char** feats)
{
  int i;

  i=0;

  while(feats[i])
    i++;

  write(fd, &i,sizeof(int));
  // Create the space for pointers
  // We need i+1 pointers
  write(fd, feats, (i+1)*sizeof(char*));

  i=0;
  while(feats[i])
    {
      write_padded(fd, (void*)(feats[i]), strlen(feats[i])+1);
      i++;
    }

}

/* Write a cart tree */

void write_tree(int fd, const cst_cart* tree)
{
  write_padded(fd, tree, sizeof(cst_cart));
  write_tree_nodes(fd, tree->rule_table);
  write_tree_feats(fd, tree->feat_table);
}

/* Write an array of cart trees */

void write_tree_array(int fd, const cst_cart** trees)
{
  int i;
  i=0;

  while(trees[i])
    i++;

  write(fd, &i,sizeof(int));
  // Create the space for pointers
  // We need i+1 pointers
  write(fd, trees, (i+1)*sizeof(cst_cart*));

  i=0;
  while(trees[i])
    {
      write_tree(fd, trees[i]);
      i++;
    }
}

/* Write a single dimensional array whose total size is "bytesize" */

void write_array(int fd, const void* data, int bytesize)
{
  write_padded(fd, data, bytesize);
}

/* Write a two dimensional array, with every unit item's size given */

void write_2d_array(int fd, const void** data, int rows, int cols, int unitsize)
{
  int i;
  int columnsize = cols*unitsize;

  write(fd, &rows, sizeof(int));

  // Write space for pointers
  write(fd, data, rows*sizeof(void*));

  for(i=0;i<rows;i++)
    write_array(fd, data[i], columnsize);
}

/* Write duration stats */

void write_dur_stats(int fd, const dur_stat** ds)
{
  int i;
  int temp = 0;
  int numstats;
  char* s;
  numstats = 0;
  while(ds[numstats])
    numstats++;

  write(fd, &numstats, sizeof(int));

  // There are i items (+ null). reserve pointers.
  
  for(i=0;i<=numstats;i++)
    write(fd, ds, sizeof(dur_stat*));

  // Write the structures themselves
  for(i=0;i<numstats;i++)
    write_padded(fd, ds[i], sizeof(dur_stat));

  // Write the string resources

  for(i=0;i<numstats;i++)
    {
      s = ds[i]->phone;
      write_padded(fd, s, strlen(s)+1);
    }
}

/* Write phone-states mapping */

void write_phone_states(int fd, const char*** ps)
{
  int i,j;
  int count;
  int count2;

  count=0;
  while(ps[count])
    count++;

  write(fd, &count, sizeof(int));

  write_padded(fd, ps, (1+count)*sizeof(char***));

  for(i=0;i<count;i++)
    {
      count2=0;
      while(ps[i][count2])
	count2++;

      write(fd, &count2, sizeof(int));

      write_padded(fd, ps[i], (1+count2)*sizeof(char**));

      for(j=0;j<count2;j++)
	write_padded(fd, ps[i][j],strlen(ps[i][j])+1);
    }  
}

/* Write a feature that will get loaded into the voice */
void write_voice_feature(int fd, const char* fname, const char* fval)
{
  // Since value will be retrieved using fgets, remove the trailing \n character in value.
  char*s = strdup(fval);
  if(s[strlen(s)-1] == '\n')
    s[strlen(s)-1] = '\0';
  write_padded(fd, fname, strlen(fname)+1);
  write_padded(fd, fval, strlen(fval)+1);
  free(s);
}

void my_fgets(char *buffer, size_t size, FILE *fp)
{
  if (NULL != fgets(buffer, size, fp))
    {
      // remove newline
      char *nlptr = strchr(buffer, '\n');
      if(nlptr) *nlptr = '\0';
    }
}

int main(int argc, char* argv[])
{
  int fd, voice_feature_count;
  cst_cg_db* db;

  char str[256];

  db = &VOICE_DB_NAME ;

  if(argc != 2) 
    {
      printf("Usage: %s outputfilename\n",argv[0]);
      exit(-1);
    }

  fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd < 0)
    {
      perror("Could not open output file");
      exit(EXIT_FAILURE);
    }

  write_init(fd);
  write_db_info(fd, db);
  write_db_types(fd, db);
  write_tree_array(fd,db->f0_trees);
  write_tree_array(fd,db->param_trees0);
  
  write_2d_array(fd, db->model_vectors0, db->num_frames0, db->num_channels0, sizeof(unsigned short));
  
  write_array(fd, db->model_min, sizeof(float)*db->num_channels0);
  write_array(fd, db->model_range, sizeof(float)*db->num_channels0);

  write_dur_stats(fd, db->dur_stats);
  write_tree(fd, db->dur_cart);

  write_phone_states(fd, db->phone_states);

  write_array(fd, db->dynwin, db->dynwinsize*sizeof(float));
  
  write_2d_array(fd, db->me_h, db->ME_num, db->ME_order, sizeof(double)); 

  voice_feature_count = 6;
  write(fd, &voice_feature_count, sizeof(int));
  /* 6 mandatory features that every voice MUST have. */
  /* If the value of some feature is not known, 
     set it to "unknown" 
  */

  printf("Enter the following information for this voice.\n");

  printf("ISO3 code for voice's language [eng] : ");
  my_fgets(str,255,stdin);
  if(!strcmp(str,"")) strcpy(str, "eng");
  write_voice_feature(fd, "language", str);

  printf("ALL-CAPS ISO3 code for speaker's country [USA] : ");
  my_fgets(str,255,stdin);
  if(!strcmp(str,"")) strcpy(str, "USA");
  write_voice_feature(fd, "country", str);

  printf("Age of the speaker at the time of recording [unknown] : ");
  my_fgets(str,255,stdin);
  if(!strcmp(str,"")) strcpy(str, "unknown");
  write_voice_feature(fd, "age", str);

  printf("Gender of the speaker (male/female) [unknown] : ");
  my_fgets(str,255,stdin);
  if(!strcmp(str,"")) strcpy(str, "unknown");
  if((strcmp(str,"male")!=0) & (strcmp(str,"female")!=0))
    strcpy(str, "unknown");
  write_voice_feature(fd, "gender", str);

  printf("Date you have built the festvox voice (YYYY-MM-DD) [unknown] : ");
  my_fgets(str,255,stdin);
  if(!strcmp(str,"")) strcpy(str, "unknown");
  write_voice_feature(fd, "build_date", str);

  printf("One line description of this voice. [unavailable] : ");
  my_fgets(str,255,stdin);
  if(!strcmp(str,"")) strcpy(str, "unavailable");
  write_voice_feature(fd, "desc", str);

  printf("\n\nVoice data Saved\n");

  close(fd);
  return 0;
}
