#ifndef __MAPREADER_h
#define __MAPREADER_h

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cst_cg.h"
#include "cst_cart.h"
#include "cst_val.h"
#include "cst_synth.h"

static void* voice_mapped_addr;
static void* current_sought_addr;
static struct stat voice_file_size;

int mapreader_init(int);
void mapreader_finish();

cst_cg_db* mapreader_load_db();
void mapreader_free_db(cst_cg_db*);

void* mapreader_read_paddded(int*); 

char** mapreader_read_types();

cst_cart_node* mapreader_read_tree_nodes();
char** mapreader_read_tree_feats();
cst_cart* mapreader_read_tree();
cst_cart** mapreader_read_tree_array();

void* mapreader_read_array();
void** mapreader_read_2d_array();

dur_stat** mapreader_read_dur_stats();

char*** mapreader_read_phone_states();

void mapreader_read_voice_feature(char** fname, char** fval);

int mapreader_read_int();
#endif
